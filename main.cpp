#include <iostream>
#include <fstream> //for output files
#include <sstream>
#include "RPD.h"
#include "Lexical_Analyzer.h"
using namespace std;

int main(){

    //get's file name and reads file
    string FILE_NAME;
    cout << "Please enter the file name (RPD_input_1.txt): ";
    getline(cin, FILE_NAME);

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


    SyntaxAnalyzer analyzer;
    OutputRedirector redirect("Syntax_Output.txt"); // Redirect output to the specified file

    analyzer.readFile(outputFileName, 4); // Read the input file

    analyzer.Rat25S(); // Start the parsing process


    string RPD_File = "RPD_File.txt";
    analyzer.display_RPD(RPD_File);
    return 0;
}
