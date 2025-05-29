#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/symboltable.h"
#include "../include/typechecker.h"
#include "../include/codegen.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        return 1;
    }

    try {
        // Read source file
        std::string source = readFile(argv[1]);

        // Initialize compiler components
        Lexer lexer(source);
        Parser parser(lexer);
        TypeChecker typeChecker;
        CodeGenerator codeGen;

        // Parse the source code
        std::cout << "Parsing source code..." << std::endl;
        auto ast = parser.parse();

        // Perform semantic analysis
        std::cout << "Performing semantic analysis..." << std::endl;
        bool hasErrors = false;
        try {
            typeChecker.check(ast);
            std::cout << "No semantic errors found." << std::endl;
        } catch (const TypeError& e) {
            std::cerr << "Type error: " << e.what() << std::endl;
            hasErrors = true;
        }

        // If there are no errors, generate code
        if (!hasErrors) {
            // Generate code
            std::cout << "Generating code..." << std::endl;
            codeGen.generate(ast);

            // Optimize the generated code
            std::cout << "Optimizing..." << std::endl;
            codeGen.optimize();

            // Output the generated code
            std::cout << "\nGenerated Code:" << std::endl;
            std::cout << "----------------" << std::endl;
            codeGen.dumpCode();
        } else {
            std::cerr << "Compilation stopped due to semantic errors." << std::endl;
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 