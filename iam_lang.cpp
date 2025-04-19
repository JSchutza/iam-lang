#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cstdlib>

// IAM Language Interpreter in C++
// Simple implementation that interprets IAM language files

// Token types
enum TokenType {
    NUMBER,
    IDENTIFIER,
    KEYWORD,
    OPERATOR,
    STRING,
    EOL
};

// Token structure
struct Token {
    TokenType type;
    std::string value;
};

// Parser class for IAM language
class IAMInterpreter {
private:
    std::vector<Token> tokens;
    std::map<std::string, int> variables;
    size_t position;

    // Lexer - Convert input to tokens
    void tokenize(const std::string& input) {
        std::istringstream stream(input);
        std::string line;
        
        tokens.clear();
        
        while (std::getline(stream, line)) {
            std::istringstream lineStream(line);
            std::string word;
            
            while (lineStream >> word) {
                Token token;
                
                // Check for numbers
                if (isdigit(word[0])) {
                    token.type = NUMBER;
                    token.value = word;
                }
                // Check for string literals
                else if (word[0] == '"') {
                    token.type = STRING;
                    
                    // Handle multi-word strings
                    if (word[word.length()-1] != '"') {
                        std::string rest;
                        std::getline(lineStream, rest, '"');
                        word += rest + "\"";
                    }
                    
                    // Remove quotes
                    token.value = word.substr(1, word.length() - 2);
                }
                // Check for keywords
                else if (word == "print" || word == "if" || word == "while" || 
                         word == "set" || word == "input") {
                    token.type = KEYWORD;
                    token.value = word;
                }
                // Check for operators
                else if (word == "+" || word == "-" || word == "*" || word == "/" ||
                         word == "==" || word == "!=" || word == "<" || word == ">") {
                    token.type = OPERATOR;
                    token.value = word;
                }
                // Default to identifier
                else {
                    token.type = IDENTIFIER;
                    token.value = word;
                }
                
                tokens.push_back(token);
            }
            
            // Add end of line token
            Token eolToken;
            eolToken.type = EOL;
            eolToken.value = "EOL";
            tokens.push_back(eolToken);
        }
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
    
    // Move to the next token
    void advance() {
        position++;
    }
    
    // Execute print statement
    void executePrint() {
        advance(); // Skip 'print'
        
        if (current().type == STRING) {
            std::cout << current().value << std::endl;
            advance();
        } else if (current().type == IDENTIFIER) {
            std::string varName = current().value;
            if (variables.find(varName) != variables.end()) {
                std::cout << variables[varName] << std::endl;
            } else {
                std::cout << "0" << std::endl; // Default value for undefined variables
            }
            advance();
        } else if (current().type == NUMBER) {
            std::cout << current().value << std::endl;
            advance();
        }
    }
    
    // Execute set statement (variable assignment)
    void executeSet() {
        advance(); // Skip 'set'
        
        if (current().type == IDENTIFIER) {
            std::string varName = current().value;
            advance();
            
            if (current().type == NUMBER) {
                variables[varName] = std::stoi(current().value);
                advance();
            }
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
                variables[varName] = std::stoi(input);
            } catch (...) {
                variables[varName] = 0; // Default if not a number
            }
            
            advance();
        }
    }
    
    // Execute a single statement
    void executeStatement() {
        if (current().type == KEYWORD) {
            if (current().value == "print") {
                executePrint();
            } else if (current().value == "set") {
                executeSet();
            } else if (current().value == "input") {
                executeInput();
            }
        }
        
        // Skip to next line
        while (current().type != EOL && position < tokens.size()) {
            advance();
        }
        
        // Skip the EOL token
        if (current().type == EOL) {
            advance();
        }
    }

public:
    IAMInterpreter() : position(0) {}
    
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
    
    std::cout << "IAM Language Interpreter (C++ Version)" << std::endl;
    
    if (argc > 1) {
        // Execute file provided as argument
        interpreter.executeFile(argv[1]);
    } else {
        // Execute from stdin if no file is provided
        interpreter.executeFromStdin();
    }
    
    return 0;
} 