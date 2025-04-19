#!/bin/bash

# Make script more flexible by allowing command line arguments
SKIP_DEPS=false
CLEAN_BUILD=true
VERBOSE=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    --skip-deps)
      SKIP_DEPS=true
      shift
      ;;
    --no-clean)
      CLEAN_BUILD=false
      shift
      ;;
    --verbose)
      VERBOSE=true
      shift
      ;;
    --help)
      echo "Usage: ./install_and_build.sh [options]"
      echo "Options:"
      echo "  --skip-deps    Skip dependency installation"
      echo "  --no-clean     Don't remove existing build before rebuilding"
      echo "  --verbose      Show more detailed output"
      echo "  --help         Show this help message"
      exit 0
      ;;
    *)
      echo "Unknown option: $1"
      exit 1
      ;;
  esac
done

echo "IAM Language Complete Installer and Builder (C++ Version)"
echo "========================================================"

if [ "$SKIP_DEPS" = true ]; then
    echo "Skipping dependency installation (--skip-deps flag set)"
fi

if [ "$CLEAN_BUILD" = true ]; then
    echo "Will clean any existing builds before rebuilding"
else
    echo "Will not clean existing builds (--no-clean flag set)"
fi

# Create directories if they don't exist
mkdir -p ~/bin
mkdir -p ~/downloads

# Add ~/bin to PATH for this session
export PATH="$HOME/bin:$PATH"

# Add to .bashrc if it doesn't exist already
if ! grep -q "export PATH=\"\$HOME/bin:\$PATH\"" ~/.bashrc 2>/dev/null; then
    echo 'export PATH="$HOME/bin:$PATH"' >> ~/.bashrc
    echo "Added ~/bin to PATH in ~/.bashrc"
fi

# Function to install Scoop (Windows package manager)
install_scoop() {
    if [ "$SKIP_DEPS" = true ]; then
        if command -v scoop &> /dev/null; then
            echo "Using existing Scoop installation"
            return 0
        else
            echo "Warning: --skip-deps set but Scoop is not installed"
        fi
    fi
    
    echo "Checking for Scoop..."
    
    # Check if Scoop is installed
    if ! command -v scoop &> /dev/null; then
        echo "Scoop is not installed. Installing it now..."
        
        # Check PowerShell availability
        if command -v powershell.exe &> /dev/null; then
            echo "Installing Scoop using PowerShell..."
            powershell.exe -Command "Set-ExecutionPolicy RemoteSigned -Scope CurrentUser -Force; [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; iex (New-Object System.Net.WebClient).DownloadString('https://get.scoop.sh')" || true
            
            # Refresh path to pick up scoop
            export PATH="$HOME/scoop/shims:$PATH"
            
            if ! command -v scoop &> /dev/null; then
                echo "Failed to install Scoop. Will try alternative methods."
                return 1
            fi
            
            echo "Scoop installed successfully."
            return 0
        else
            echo "PowerShell not found. Cannot install Scoop."
            return 1
        fi
    else
        echo "Scoop is already installed."
        return 0
    fi
}

# Function to install Make
install_make() {
    if [ "$SKIP_DEPS" = true ]; then
        if command -v make &> /dev/null; then
            echo "Using existing Make installation"
            return 0
        else
            echo "Warning: --skip-deps set but Make is not installed"
        fi
    fi
    
    echo "Checking for Make..."
    
    # Check if Make is already installed
    if command -v make &> /dev/null; then
        echo "Make is already installed."
        return 0
    fi
    
    # Try to install Make using Scoop
    if command -v scoop &> /dev/null; then
        echo "Installing Make with Scoop..."
        scoop install make || true
        
        # Update PATH to include Scoop apps
        export PATH="$HOME/scoop/shims:$PATH"
        
        # Verify Make installation
        if command -v make &> /dev/null; then
            echo "Make installed successfully."
            return 0
        fi
    fi
    
    echo "Failed to install Make. Will use direct build method."
    return 1
}

# Function to install C++ compiler
install_cpp_compiler() {
    if [ "$SKIP_DEPS" = true ]; then
        if command -v g++ &> /dev/null || command -v cl &> /dev/null || 
           [ -f "$HOME/scoop/apps/gcc/current/bin/g++.exe" ] || [ -f ~/bin/g++.exe ]; then
            echo "Using existing C++ compiler"
            return 0
        else
            echo "Warning: --skip-deps set but no C++ compiler found"
        fi
    fi
    
    echo "Checking for C++ compiler..."
    
    # Check if g++ is already installed and in PATH
    if command -v g++ &> /dev/null; then
        echo "g++ compiler is already installed."
        return 0
    fi
    
    # Check if Visual C++ compiler is available
    if command -v cl &> /dev/null; then
        echo "Visual C++ compiler is already installed."
        return 0
    fi
    
    # Try to install gcc/g++ using Scoop
    if command -v scoop &> /dev/null; then
        echo "Installing gcc/g++ with Scoop..."
        scoop install gcc || true
        
        # Update PATH to include Scoop's gcc
        export PATH="$HOME/scoop/apps/gcc/current/bin:$PATH"
        export PATH="$HOME/scoop/shims:$PATH"
        
        echo "Updated PATH with gcc location."
        
        # Check if g++ is now available
        if command -v g++ &> /dev/null; then
            echo "g++ installed successfully."
            return 0
        fi
        
        # Check if g++ exists in the gcc installation dir
        if [ -f "$HOME/scoop/apps/gcc/current/bin/g++.exe" ]; then
            echo "Found g++ in Scoop's gcc directory."
            export PATH="$HOME/scoop/apps/gcc/current/bin:$PATH"
            
            if command -v g++ &> /dev/null; then
                echo "g++ now available in PATH."
                return 0
            else
                echo "Creating link to g++ in ~/bin"
                cp "$HOME/scoop/apps/gcc/current/bin/g++.exe" ~/bin/
                chmod +x ~/bin/g++.exe
                return 0
            fi
        fi
    fi
    
    # If nothing worked so far, try to find a g++ executable from common locations
    for gcc_path in "$HOME/scoop/apps/gcc/current/bin" "/mingw64/bin" "/c/MinGW/bin"; do
        if [ -f "$gcc_path/g++.exe" ]; then
            echo "Found g++ in $gcc_path"
            cp "$gcc_path/g++.exe" ~/bin/
            chmod +x ~/bin/g++.exe
            export PATH="$HOME/bin:$PATH"
            echo "g++ copied to ~/bin"
            return 0
        fi
    done
    
    echo "Failed to install a C++ compiler. Build may fail."
    return 1
}

# Function to verify source files
check_source_files() {
    if [ ! -f "iam_lang.cpp" ]; then
        echo "ERROR: iam_lang.cpp source file not found!"
        return 1
    fi
    
    if [ "$VERBOSE" = true ]; then
        echo "Found source file: iam_lang.cpp"
    fi
    
    return 0
}

# Function to clean up old build artifacts
clean_build() {
    if [ "$CLEAN_BUILD" = true ]; then
        echo "Cleaning up old build artifacts..."
        
        # Remove executable files
        if [ -f "iam_lang" ]; then
            echo "Removing old iam_lang executable"
            rm -f "iam_lang"
        fi
        
        if [ -f "iam_lang.exe" ]; then
            echo "Removing old iam_lang.exe executable"
            rm -f "iam_lang.exe"
        fi
        
        # Remove any object files
        rm -f *.o
        
        echo "Clean-up completed."
    else
        echo "Skipping clean-up (--no-clean flag set)"
    fi
}

# Function to ensure examples file exists
ensure_examples() {
    if [ ! -f "examples.iam" ]; then
        echo "Creating sample examples.iam file..."
        cat > examples.iam << 'EOF'
# IAM Language Examples

# Print a welcome message
print "Welcome to IAM Language!"

# Variable assignment
set counter 10

# Print a variable
print "The counter value is:"
print counter

# Ask for user input
print "Please enter a number:"
input userValue

# Show the entered value
print "You entered:"
print userValue
EOF
        echo "Created examples.iam file."
    elif [ "$VERBOSE" = true ]; then
        echo "Found existing examples.iam file"
    fi
}

# Function to ensure makefile exists
ensure_makefile() {
    if [ ! -f "Makefile" ] && [ ! -f "makefile" ]; then
        echo "Creating Makefile for C++ build..."
        cat > Makefile << 'EOF'
# Makefile for IAM Language C++ version

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

all: iam_lang

iam_lang: iam_lang.cpp
	$(CXX) $(CXXFLAGS) -o iam_lang iam_lang.cpp

clean:
	rm -f iam_lang iam_lang.exe *.o

run: iam_lang
	./iam_lang

test: iam_lang
	./iam_lang examples.iam
EOF
        echo "Created Makefile."
        return 0
    else
        if [ "$VERBOSE" = true ]; then
            echo "Found existing Makefile"
        else
            echo "Makefile already exists."
        fi
        return 0
    fi
}

# Function to build the IAM language using make
build_with_make() {
    echo "Building IAM language using make..."
    
    # Ensure makefile exists
    ensure_makefile
    
    # Run make with PATH explicitly set
    PATH="$HOME/scoop/apps/gcc/current/bin:$PATH" make
    
    if [ $? -eq 0 ]; then
        echo "Build completed successfully!"
        return 0
    else
        echo "Build with make failed. Will try direct compilation..."
        return 1
    fi
}

# Function to build the IAM language directly
build_directly() {
    echo "Building IAM language directly with C++ compiler..."
    
    # Try to find g++ in scoop's directory if not in PATH
    GPP_PATH="g++"
    if ! command -v g++ &> /dev/null; then
        if [ -f "$HOME/scoop/apps/gcc/current/bin/g++.exe" ]; then
            GPP_PATH="$HOME/scoop/apps/gcc/current/bin/g++.exe"
            echo "Using g++ from Scoop installation: $GPP_PATH"
        elif [ -f ~/bin/g++.exe ]; then
            GPP_PATH="$HOME/bin/g++.exe"
            echo "Using g++ from ~/bin: $GPP_PATH"
        else
            # Search for g++ in common locations as a last resort
            for dir in "/mingw64/bin" "/c/MinGW/bin"; do
                if [ -f "$dir/g++.exe" ]; then
                    GPP_PATH="$dir/g++.exe"
                    echo "Found g++ at: $GPP_PATH"
                    break
                fi
            done
        fi
    fi
    
    # Determine which compiler to use
    if [ -x "$GPP_PATH" ] || command -v g++ &> /dev/null; then
        echo "Using g++ compiler"
        "$GPP_PATH" -std=c++11 -Wall -o iam_lang iam_lang.cpp
    elif command -v cl &> /dev/null; then
        echo "Using Visual C++ compiler"
        cl /EHsc /Fe:iam_lang.exe iam_lang.cpp
    else
        echo "No C++ compiler found. Cannot build directly."
        return 1
    fi
    
    # Check if build succeeded
    if [ -f "iam_lang" ] || [ -f "iam_lang.exe" ]; then
        echo "Direct build completed successfully!"
        return 0
    else
        echo "Direct build failed."
        return 1
    fi
}

# Function to install all dependencies
install_dependencies() {
    if [ "$SKIP_DEPS" = true ]; then
        echo "Skipping dependency installation as requested"
        return 0
    fi
    
    local all_success=true
    
    # Install Scoop (for Windows package management)
    install_scoop
    
    # Install Make
    if ! install_make; then
        echo "Make installation incomplete."
        all_success=false
    fi
    
    # Install C++ compiler
    if ! install_cpp_compiler; then
        echo "C++ compiler installation incomplete."
        all_success=false
    fi
    
    if [ "$all_success" = true ]; then
        return 0
    else
        return 1
    fi
}

# Main script execution
echo "Step 1: Installing dependencies..."
install_dependencies
if [ $? -ne 0 ] && [ "$SKIP_DEPS" = false ]; then
    echo "WARNING: Some dependencies may not have installed correctly."
    echo "We'll attempt to build anyway, but it may fail."
fi

echo
echo "Step 2: Checking tools and versions..."
if command -v make &> /dev/null; then
    echo "Make: $(make --version | head -n1)"
else
    echo "Make: Not installed!"
fi

# Check for g++ in both PATH and Scoop directory
if command -v g++ &> /dev/null; then
    echo "g++: $(g++ --version | head -n1)"
elif [ -f "$HOME/scoop/apps/gcc/current/bin/g++.exe" ]; then
    echo "g++: $("$HOME/scoop/apps/gcc/current/bin/g++.exe" --version | head -n1) (not in PATH)"
elif [ -f ~/bin/g++.exe ]; then
    echo "g++: $(~/bin/g++.exe --version | head -n1) (in ~/bin)"
elif command -v cl &> /dev/null; then
    echo "cl: Visual C++ Compiler"
else
    echo "C++ Compiler: Not found!"
fi

echo
echo "Step 3: Checking source files..."
if ! check_source_files; then
    echo "ERROR: Required source files are missing."
    exit 1
fi

# Clean up old builds
clean_build

# Ensure examples file exists
ensure_examples

echo
echo "Step 4: Building IAM language (C++ version)..."

# First try to build with make
if command -v make &> /dev/null; then
    build_with_make || build_directly
else
    # If make is not available, build directly
    build_directly
fi

# Check if either build method succeeded
if [ -f "iam_lang" ] || [ -f "iam_lang.exe" ]; then
    echo
    echo "Installation and build completed successfully!"
    echo "You can now run the IAM language interpreter with: ./iam_lang"
    echo "Or run the example program with: ./iam_lang examples.iam"
    echo "Or pipe input to it with: cat examples.iam | ./iam_lang"
else
    echo
    echo "Both build methods failed. Please check the issues mentioned above."
    exit 1
fi 