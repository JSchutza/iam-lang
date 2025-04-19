#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cstdlib>
#include <algorithm>

// IAM Language Interpreter in C++
// Enhanced version with arrays and loops

// Token types
enum TokenType {
    NUMBER,
    IDENTIFIER,
    KEYWORD,
    OPERATOR,
    STRING,
    ARRAY_DECL,     // New: Array declaration
    ARRAY_ACCESS,   // New: Array element access
    LOOP_KEYWORD,   // New: Loop-related keywords (for, while, end)
    SPECIAL_CHAR,   // New: Special characters like '[', ']', etc.
    EOL
};

// Token structure
struct Token {
    TokenType type;
    std::string value;
};

// Value structure to store different types of values
struct Value {
    enum Type { INTEGER, STRING, ARRAY } type;
    int intValue;
    std::string stringValue;
    std::vector<Value> arrayElements;
    
    // Constructors for different types
    Value() : type(INTEGER), intValue(0) {}
    
    Value(int val) : type(INTEGER), intValue(val) {}
    
    Value(const std::string& val) : type(STRING), stringValue(val) {}
    
    Value(const std::vector<Value>& elements) : type(ARRAY), arrayElements(elements) {}
    
    // String representation
    std::string toString() const {
        if (type == INTEGER) {
            return std::to_string(intValue);
        } else if (type == STRING) {
            return stringValue;
        } else if (type == ARRAY) {
            std::string result = "[";
            for (size_t i = 0; i < arrayElements.size(); i++) {
                if (i > 0) result += ", ";
                result += arrayElements[i].toString();
            }
            result += "]";
            return result;
        }
        return "undefined";
    }
};

// Parser class for IAM language
class IAMInterpreter {
private:
    std::vector<Token> tokens;
    std::map<std::string, Value> variables;
    size_t position;
    bool inLoop; // Track if we're inside a loop
    std::vector<size_t> loopStartPositions; // Stack of loop start positions

    // Lexer - Convert input to tokens
    void tokenize(const std::string& input) {
        std::istringstream stream(input);
        std::string line;
        
        tokens.clear();
        
        while (std::getline(stream, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') {
                // Add EOL token even for comments
                Token eolToken;
                eolToken.type = EOL;
                eolToken.value = "EOL";
                tokens.push_back(eolToken);
                continue;
            }
            
            std::istringstream lineStream(line);
            std::string word;
            bool inString = false;
            std::string currentString;
            
            // Process each character to handle strings and special chars better
            for (size_t i = 0; i < line.length(); i++) {
                char c = line[i];
                
                // Handle string literals
                if (c == '"') {
                    if (!inString) {
                        // Start of string
                        inString = true;
                        currentString = "";
                    } else {
                        // End of string
                        inString = false;
                        
                        // Add the string token
                        Token token;
                        token.type = STRING;
                        token.value = currentString;
                        tokens.push_back(token);
                    }
                    continue;
                }
                
                if (inString) {
                    currentString += c;
                    continue;
                }
                
                // Handle array access notation [index]
                if (c == '[' || c == ']') {
                    // Add any pending word
                    if (!word.empty()) {
                        addWordToken(word);
                        word = "";
                    }
                    
                    Token token;
                    token.type = SPECIAL_CHAR;
                    token.value = std::string(1, c);
                    tokens.push_back(token);
                    continue;
                }
                
                // Handle whitespace
                if (isspace(c)) {
                    // Add any pending word
                    if (!word.empty()) {
                        addWordToken(word);
                        word = "";
                    }
                    continue;
                }
                
                // Collect word characters
                word += c;
            }
            
            // Add any remaining word
            if (!word.empty()) {
                addWordToken(word);
            }
            
            // Add end of line token
            Token eolToken;
            eolToken.type = EOL;
            eolToken.value = "EOL";
            tokens.push_back(eolToken);
        }
    }
    
    // Helper to add a word token
    void addWordToken(const std::string& word) {
        Token token;
        
        // Check for numbers
        if (isdigit(word[0]) || (word[0] == '-' && word.length() > 1 && isdigit(word[1]))) {
            token.type = NUMBER;
            token.value = word;
        }
        // Check for keywords
        else if (word == "print" || word == "if" || word == "set" || word == "input" || 
                 word == "array") {
            if (word == "array") {
                token.type = ARRAY_DECL;
            } else {
                token.type = KEYWORD;
            }
            token.value = word;
        }
        // Check for loop keywords
        else if (word == "for" || word == "while" || word == "end" || word == "in") {
            token.type = LOOP_KEYWORD;
            token.value = word;
        }
        // Check for operators
        else if (word == "+" || word == "-" || word == "*" || word == "/" ||
                 word == "==" || word == "!=" || word == "<" || word == ">" ||
                 word == "<=" || word == ">=" || word == "=") {
            token.type = OPERATOR;
            token.value = word;
        }
        // Check for legacy keywords
        else if (word == "LET" || word == "PRINT" || word == "EXIT" || word == "IF") {
            token.type = KEYWORD;
            token.value = word;
        }
        // Default to identifier
        else {
            token.type = IDENTIFIER;
            token.value = word;
        }
        
        tokens.push_back(token);
    }
    
    // Get the current token
    Token& current() {
        if (position >= tokens.size()) {
            static Token empty;
            empty.type = EOL;
            empty.value = "EOF";
            return empty;
        }
        return tokens[position];
    }
    
    // Look ahead to the next token
    Token& peek() {
        if (position + 1 >= tokens.size()) {
            static Token empty;
            empty.type = EOL;
            empty.value = "EOF";
            return empty;
        }
        return tokens[position + 1];
    }
    
    // Move to the next token
    void advance() {
        position++;
    }
    
    // Check if tokens match array access pattern
    bool isArrayAccess() {
        // Need at least 4 tokens: identifier, [, index, ]
        if (position + 3 >= tokens.size()) {
            return false;
        }
        
        return current().type == IDENTIFIER &&
               peek().type == SPECIAL_CHAR && peek().value == "[";
    }
    
    // Parse array access expression: name[index]
    std::pair<std::string, int> parseArrayAccess() {
        std::string arrayName = current().value;
        advance(); // Skip array identifier
        advance(); // Skip '['
        
        int index = 0;
        // Index can be a number or variable
        if (current().type == NUMBER) {
            index = std::stoi(current().value);
        } else if (current().type == IDENTIFIER) {
            std::string varName = current().value;
            if (variables.find(varName) != variables.end() && 
                variables[varName].type == Value::INTEGER) {
                index = variables[varName].intValue;
            } else {
                std::cerr << "Error: Invalid index variable " << varName << std::endl;
            }
        } else {
            std::cerr << "Error: Expected number or variable as array index" << std::endl;
        }
        
        advance(); // Skip index
        advance(); // Skip ']'
        
        return {arrayName, index};
    }
    
    // Evaluate an expression (number, variable, or array access)
    Value evaluateExpression() {
        if (current().type == NUMBER) {
            Value value(std::stoi(current().value));
            advance();
            return value;
        } else if (current().type == STRING) {
            Value value(current().value);
            advance();
            return value;
        } else if (isArrayAccess()) {
            auto [arrayName, index] = parseArrayAccess();
            
            if (variables.find(arrayName) != variables.end() && 
                variables[arrayName].type == Value::ARRAY) {
                
                auto& array = variables[arrayName].arrayElements;
                if (index >= 0 && index < static_cast<int>(array.size())) {
                    return array[index];
                } else {
                    std::cerr << "Error: Array index out of bounds: " << index << std::endl;
                    return Value(0);
                }
            } else {
                std::cerr << "Error: Array not found: " << arrayName << std::endl;
                return Value(0);
            }
        } else if (current().type == IDENTIFIER) {
            std::string varName = current().value;
            advance();
            
            if (variables.find(varName) != variables.end()) {
                return variables[varName];
            } else {
                std::cerr << "Warning: Variable not defined: " << varName << std::endl;
                return Value(0);
            }
        }
        
        // Default case
        return Value(0);
    }
    
    // Execute print statement
    void executePrint() {
        advance(); // Skip 'print'
        
        std::cout << evaluateExpression().toString() << std::endl;
    }
    
    // Execute set statement (variable assignment)
    void executeSet() {
        advance(); // Skip 'set'
        
        if (isArrayAccess()) {
            auto [arrayName, index] = parseArrayAccess();
            
            if (variables.find(arrayName) != variables.end() && 
                variables[arrayName].type == Value::ARRAY) {
                
                auto& array = variables[arrayName].arrayElements;
                if (index >= 0 && index < static_cast<int>(array.size())) {
                    // Parse the value to assign
                    Value value = evaluateExpression();
                    array[index] = value;
                } else {
                    std::cerr << "Error: Array index out of bounds for assignment: " << index << std::endl;
                }
            } else {
                std::cerr << "Error: Array not found for assignment: " << arrayName << std::endl;
            }
        } else if (current().type == IDENTIFIER) {
            std::string varName = current().value;
            advance();
            
            // Simple assignment
            variables[varName] = evaluateExpression();
        }
    }
    
    // Execute input statement
    void executeInput() {
        advance(); // Skip 'input'
        
        if (current().type == IDENTIFIER) {
            std::string varName = current().value;
            std::string input;
            std::getline(std::cin, input);
            
            try {
                // Try to parse as integer first
                variables[varName] = Value(std::stoi(input));
            } catch (...) {
                // Otherwise store as string
                variables[varName] = Value(input);
            }
            
            advance();
        }
    }
    
    // Execute array declaration
    void executeArrayDeclaration() {
        advance(); // Skip 'array'
        
        if (current().type == IDENTIFIER) {
            std::string arrayName = current().value;
            advance();
            
            // Check the next token to determine initialization type
            if (current().type == NUMBER) {
                // Array with size: array myArray 5
                int size = std::stoi(current().value);
                std::vector<Value> elements(size, Value(0));
                variables[arrayName] = Value(elements);
                advance();
            } else {
                // For now, default to empty array
                variables[arrayName] = Value(std::vector<Value>());
            }
        }
    }
    
    // Execute for loop
    void executeForLoop() {
        advance(); // Skip 'for'
        
        // Store the loop start position
        size_t loopStart = position;
        loopStartPositions.push_back(loopStart);
        
        // Parse loop header: for i 1 10
        if (current().type == IDENTIFIER) {
            std::string counterVar = current().value;
            advance();
            
            // Parse start value
            Value startValue = evaluateExpression();
            
            // Parse end value (exclusive)
            Value endValue = evaluateExpression();
            
            if (startValue.type == Value::INTEGER && endValue.type == Value::INTEGER) {
                // Initialize counter
                variables[counterVar] = startValue;
                
                // Execute the loop
                inLoop = true;
                while (variables[counterVar].intValue < endValue.intValue) {
                    // Save current position to return here for next iteration
                    size_t loopBodyStart = position;
                    
                    // Execute the loop body until we hit 'end'
                    while (!(current().type == LOOP_KEYWORD && current().value == "end")) {
                        executeStatement();
                    }
                    
                    // Increment counter
                    variables[counterVar].intValue++;
                    
                    // Reset position to the start of the loop body
                    position = loopBodyStart;
                }
                
                // Skip past the loop body to the 'end' keyword
                while (!(current().type == LOOP_KEYWORD && current().value == "end")) {
                    advance();
                }
                advance(); // Skip 'end'
                
                inLoop = false;
                loopStartPositions.pop_back();
            }
        }
    }
    
    // Execute a single statement
    void executeStatement() {
        if (current().type == KEYWORD) {
            if (current().value == "print" || current().value == "PRINT") {
                executePrint();
            } else if (current().value == "set" || current().value == "LET") {
                executeSet();
            } else if (current().value == "input") {
                executeInput();
            } else if (current().value == "EXIT") {
                // Exit the interpreter
                exit(0);
            }
        } else if (current().type == ARRAY_DECL) {
            executeArrayDeclaration();
        } else if (current().type == LOOP_KEYWORD) {
            if (current().value == "for") {
                executeForLoop();
            } else if (current().value == "end") {
                // End of loop, should never reach here directly
                advance();
            }
        } else {
            // Skip unknown statement
            advance();
        }
        
        // Skip to next line if not at EOL
        while (current().type != EOL && position < tokens.size()) {
            advance();
        }
        
        // Skip the EOL token
        if (current().type == EOL) {
            advance();
        }
    }

public:
    IAMInterpreter() : position(0), inLoop(false) {}
    
    // Execute a program from string
    void execute(const std::string& program) {
        tokenize(program);
        position = 0;
        
        while (position < tokens.size()) {
            executeStatement();
        }
    }
    
    // Execute a program from a file
    bool executeFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        
        execute(buffer.str());
        return true;
    }
    
    // Execute from standard input
    void executeFromStdin() {
        std::stringstream buffer;
        std::string line;
        
        while (std::getline(std::cin, line)) {
            buffer << line << std::endl;
        }
        
        execute(buffer.str());
    }
};

int main(int argc, char* argv[]) {
    IAMInterpreter interpreter;
    
    std::cout << "IAM Language Interpreter (C++ Version with Arrays & Loops)" << std::endl;
    
    if (argc > 1) {
        // Execute file provided as argument
        interpreter.executeFile(argv[1]);
    } else {
        // Execute from stdin if no file is provided
        interpreter.executeFromStdin();
    }
    
    return 0;
} 