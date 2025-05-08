#include <iostream>
#include <fstream> //for output files
#include <sstream>
#include "RPD.h"
#include "Lexical_Analyzer.h"
using namespace std;

int main(){

    //get's file name and reads file
    string FILE_NAME;
    cout << "Please enter the file name (RPD_input_1.txt, RPD_input_2.txt, RPD_input_3.txt): ";
    cin >> FILE_NAME;

    FILE* filePointer = fopen(FILE_NAME.c_str(), "r");

    if (!filePointer) {
        std::cout << "Error: Cannot open file " << FILE_NAME << std::endl;
        return 1;
    }

    //initializes class and vector
    LexicalAnalyzer la;
    std::vector<Token> tokens;

    //calls lexer and pushes tokens inside the vector
    while (true) {
        Token token = la.lexer(filePointer);
        if (token.value.empty()) {
            break;
        }
        tokens.push_back(token);
    }

    //closes file
    fclose(filePointer);

    //get's file name and writes to file
    string outputFileName = "Lexical_Analysis_Output.txt";

    ofstream outFile(outputFileName);
    if (!outFile) {
        std::cerr << "Error opening output file for writing" << std::endl;
        return 1;
    }

    outFile << "\nOutput:\n";
    outFile << "                    token                             lexeme\n";
    outFile << "------------------------------------------------------------\n";

    //outputs using the vector
    for (size_t i = 0; i < tokens.size(); ++i) {
        string type;
        switch (tokens[i].type) {
        case TokenType::KEYWORD: type = "keyword";
            break;
        case TokenType::IDENTIFIER: type = "identifier";
            break;
        case TokenType::INTEGER: type = "integer";
            break;
        case TokenType::REAL: type = "real";
            break;
        case TokenType::OPERATOR: type = "operator";
            break;
        case TokenType::SEPARATOR: type = "separator";
            break;
        default: type = "unknown";
            break;
        }

        outFile << "                   " << left << setw(35) << type << tokens[i].value << endl;
    }

    try {
        // Get RPD filename from user
        string RPD_File;
        cout << "Please enter the file name (RPD_output_1.txt, RPD_output_2.txt, RPD_output_3.txt): ";
        cin >> RPD_File;
        
        // Open output files
        ofstream symbol_assembly_file(RPD_File);
        if (!symbol_assembly_file.is_open()) {
            throw runtime_error("Failed to open RPD output file");
        }

        string syntax_output_file = "Syntax_Output.txt";
        ofstream outSyn_A_File(syntax_output_file);
        if (!outSyn_A_File.is_open()) {
            throw runtime_error("Failed to open syntax output file");
        }

        // Initialize analyzer with both streams
        SyntaxAnalyzer analyzer(outSyn_A_File, symbol_assembly_file);
        
        // Process files
        analyzer.readFile("Lexical_Analysis_Output.txt", 4); 
        analyzer.Rat25S();
        analyzer.display_RPD();

        // Files will auto-close when going out of scope
    } 
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
