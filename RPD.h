#ifndef RPD_H
#define RPD_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stack>
#include <unordered_map>
#include <optional>
#include "TokenType.h"
using namespace std;

enum Type { INTEGER, BOOLEAN, UNDEFINED };


class Symbol_and_Assembly{
private:
    int memoryAddr = 10000;
    int instructionAddr= 1;
    ostream& symbol_assembly_file;


    struct Instruction {
        int ADDR;
        string Operator;
        optional<int> Operand;
        Instruction(int a, const string& op, optional<int> o) : 
        ADDR(a), Operator(op), Operand(o) {}
    };

    vector<Instruction> InstructTable;

    struct SymbolInfo {
        int memoryADDR;
        Type type;
    };


    unordered_map<string, SymbolInfo> SymbolTable;
    
    stack<Type> Stack;
    stack<int> JumpStack;
    
public:
    Symbol_and_Assembly(ostream& out) : symbol_assembly_file(out) {
        //reserve 1000 spaces in vector for Instruction table
        InstructTable.reserve(1000);
        if (!out.good()) {
            throw runtime_error("Output stream is not in good state");
        }
    }

     void display_instructions() {
        symbol_assembly_file << "\n=== INSTRUCTION TABLE ===\n";
        symbol_assembly_file << "ADDR\tOPERATOR\tOPERAND\n";
        symbol_assembly_file << "------------------------\n";
        for (const auto& instr : InstructTable) {
            symbol_assembly_file << instr.ADDR << "\t" << instr.Operator << "\t\t";
            if (instr.Operand.has_value()) {
                symbol_assembly_file << instr.Operand.value();
            } else {
                symbol_assembly_file << "-";
            }
            symbol_assembly_file << "\n";
        }
    }

    void display_symbol_table() {
        symbol_assembly_file << "\n=== SYMBOL TABLE ===\n";
        symbol_assembly_file << "NAME\t\tADDRESS\t\tType\n";
        symbol_assembly_file << "--------------------------------\n";
        for (const auto& entry : SymbolTable) {
            symbol_assembly_file << entry.first << "\t\t" << entry.second.memoryADDR;
    
            switch (entry.second.type) {
                case Type::INTEGER:
                    symbol_assembly_file << "\t\t" << "Integer" << endl;
                    break;
    
                case Type::BOOLEAN:
                    symbol_assembly_file << "\t\t" << "Boolean" << endl;
                    break;
    
                default:
                    symbol_assembly_file << "\t\t" << "Undefined" << endl;
                    break;
            }
        }
    }
    
    //Add into Instruction Table
    void generate_instruction(string op, optional<int> oprnd = nullopt){
        InstructTable.emplace_back(instructionAddr++, op, oprnd);
    }

    //Add a variable into the Symbol table
    void generate_symbol(string var, Type type){
        SymbolTable[var].memoryADDR = memoryAddr;
        SymbolTable[var].type =  type; 
        memoryAddr++;
    }

    // Returns Variable Memory
    int getAddress(string var){
        return SymbolTable[var].memoryADDR;
    }

    int getInstructionAddr(){
        return instructionAddr;
    }

    void back_patch(int JMP_address){
        if(JumpStack.size() >= 1){
            int addr = JumpStack.top();
            JumpStack.pop();
            if(InstructTable[addr].Operator == "JMP0"){
                InstructTable[addr].Operand = JMP_address;
            }
        }
    }

    //PUSHI
    void PUSHI(Type type){
        //Pushes the {Integer Value} onto the Top of the Stack (TOS)
        Stack.push(Type(type));
        generate_instruction("PUSHI");
    }

    void PUSHB(Type type) {
        //Push Boolean values onto TOS
        Stack.push(Type(type));
        generate_instruction("PUSHB");

    }

    //PUSHM
    void PUSHM(string var, int memoryLoc){
        //Pushes the value stored at {ML} onto TOS
        auto it = SymbolTable.find(var);
        if (it != SymbolTable.end()) {
            Stack.push(it->second.type);
            generate_instruction("PUSHM", memoryLoc);
        } else {
            throw runtime_error("Undefined variable: " + var);
        }
    }

    void POPM(int memoryLoc, string var){
        //Pops the value from the top of the stack and stores it at {ML}
        if(Stack.empty()){
            throw runtime_error("Stack underflow");
        }

        Type stackType = Stack.top();
        Stack.pop();

        SymbolTable[var].type = stackType;

        generate_instruction("POPM", memoryLoc);
    }

    void SOUT(){
        // Pops the value from the top of the stack and prints it and adds instruction
        if (Stack.empty()) {
            throw runtime_error("Stack underflow");
        }
        // Type type = Stack.top();
        Stack.pop();

        generate_instruction("SOUT");
    }

    void SIN(string var){
        auto it = SymbolTable.find(var);

        if (it == SymbolTable.end()) {
            throw runtime_error("Undefined variable: " + var);
        }

        Type expectedType = it->second.type;

        if (expectedType == Type::BOOLEAN) {
            PUSHB(Type(Type::BOOLEAN));
        } else if (expectedType == Type::INTEGER) {
            PUSHI(Type(Type::INTEGER));
        }
        else {
            Stack.push(Type(Type::UNDEFINED));
            generate_instruction("PUSHU");
        }
        
        generate_instruction("SIN");
        POPM(SymbolTable[var].memoryADDR, var);
    }

    void A() {
        if (Stack.size() < 2) {
            throw runtime_error("Stack underflow");
        }
    
        Type first = Stack.top(); 
        Stack.pop();
        Type second = Stack.top(); 
        Stack.pop();
   
        if(first == Type::INTEGER && second == Type::INTEGER){
            Stack.push(Type(Type::INTEGER));
        }
        else {
            Stack.push(Type(Type::UNDEFINED));
        }
        
        generate_instruction("A");

    }

    void S(){
        // Pop the first two items from stack and push the difference onto the TOS
                if (Stack.size() < 2) {
            throw runtime_error("Stack underflow");
        }
    
        Type first = Stack.top(); 
        Stack.pop();
        Type second = Stack.top(); 
        Stack.pop();
   
        if(first == Type::INTEGER && second == Type::INTEGER){
            Stack.push(Type(Type::INTEGER));
        }
        else {
            Stack.push(Type(Type::UNDEFINED));
        }
        
        generate_instruction("S");
    }

    void M(){
        // Pop the first two items from stack and push the product onto the TOS
                if (Stack.size() < 2) {
            throw runtime_error("Stack underflow");
        }
    
        Type first = Stack.top(); 
        Stack.pop();
        Type second = Stack.top(); 
        Stack.pop();
   
        if(first == Type::INTEGER && second == Type::INTEGER){
            Stack.push(Type(Type::INTEGER));
        }
        else {
            Stack.push(Type(Type::UNDEFINED));
        }
        
        generate_instruction("M");
    }

    void D(){
        // Pop the first two items from stack and push the result onto the TOS
                if (Stack.size() < 2) {
            throw runtime_error("Stack underflow");
        }
    
        Type first = Stack.top(); 
        Stack.pop();
        Type second = Stack.top(); 
        Stack.pop();
   
        if(first == Type::INTEGER && second == Type::INTEGER){
            Stack.push(Type(Type::INTEGER));
        }
        else {
            Stack.push(Type(Type::UNDEFINED));
        }
        
        generate_instruction("D");
    }

    void GRT(){
        //Pops two items from the stack and pushes 1 onto TOS if second item is larger otherwise push 0
        if (Stack.size() < 2) {
            throw runtime_error("Stack underflow");
        }
    
        Type first = Stack.top(); 
        Stack.pop();
        Type second = Stack.top(); 
        Stack.pop();
   
        if(first == Type::INTEGER && second == Type::INTEGER){
            Stack.push(Type(Type::INTEGER));
        }
        else if(first == Type::BOOLEAN && second == Type::BOOLEAN){
            Stack.push(Type(Type::BOOLEAN));
        }
        else {
            Stack.push(Type(Type::UNDEFINED));
        }
        
        generate_instruction("GRT");
    }

    void LES() {
        //Pops two items from the stack and pushes 1 onto TOS if the second item is smaller than first item otherwise push 0
        if (Stack.size() < 2) {
            throw runtime_error("Stack underflow");
        }
    
        Type first = Stack.top(); 
        Stack.pop();
        Type second = Stack.top(); 
        Stack.pop();
   
        if(first == Type::INTEGER && second == Type::INTEGER){
            Stack.push(Type(Type::INTEGER));
        }
        else if(first == Type::BOOLEAN && second == Type::BOOLEAN){
            Stack.push(Type(Type::BOOLEAN));
        }
        else {
            Stack.push(Type(Type::UNDEFINED));
        }
        
        generate_instruction("LES");
    }

    void EQU() {
        //Pops two items from the stack and pushes 1 onto TOS if they are equal otherwise push 0
       if (Stack.size() < 2) {
            throw runtime_error("Stack underflow");
        }
    
        Type first = Stack.top(); 
        Stack.pop();
        Type second = Stack.top(); 
        Stack.pop();
   
        if(first == Type::INTEGER && second == Type::INTEGER){
            Stack.push(Type(Type::INTEGER));
        }
        else if(first == Type::BOOLEAN && second == Type::BOOLEAN){
            Stack.push(Type(Type::BOOLEAN));
        }
        else {
            Stack.push(Type(Type::UNDEFINED));
        }
        
        generate_instruction("EQU");
    }

    void NEQ() {
        //Pops two items from the stack and pushes 1 onto TOS if they are not equal otherwise push 0
        if (Stack.size() < 2) {
            throw runtime_error("Stack underflow");
        }
    
        Type first = Stack.top(); 
        Stack.pop();
        Type second = Stack.top(); 
        Stack.pop();
   
        if(first == Type::INTEGER && second == Type::INTEGER){
            Stack.push(Type(Type::INTEGER));
        }
        else if(first == Type::BOOLEAN && second == Type::BOOLEAN){
            Stack.push(Type(Type::BOOLEAN));
        }
        else {
            Stack.push(Type(Type::UNDEFINED));
        }
        
        generate_instruction("NEQ");
    }

    void GEQ() {
        //Pops two items from the stack and pushes 1 onto TOS if second item is larger or equal otherwise push 0
       if (Stack.size() < 2) {
            throw runtime_error("Stack underflow");
        }
    
        Type first = Stack.top(); 
        Stack.pop();
        Type second = Stack.top(); 
        Stack.pop();
   
        if(first == Type::INTEGER && second == Type::INTEGER){
            Stack.push(Type(Type::INTEGER));
        }
        else if(first == Type::BOOLEAN && second == Type::BOOLEAN){
            Stack.push(Type(Type::BOOLEAN));
        }
        else {
            Stack.push(Type(Type::UNDEFINED));
        }
        
        generate_instruction("GEQ");

    }

    void LEQ() {
        //Pops two items from the stack and pushes 1 onto TOS if second item is Less or equal otherwise push 0
        if (Stack.size() < 2) {
            throw runtime_error("Stack underflow");
        }
    
        Type first = Stack.top(); 
        Stack.pop();
        Type second = Stack.top(); 
        Stack.pop();
   
        if(first == Type::INTEGER && second == Type::INTEGER){
            Stack.push(Type(Type::INTEGER));
        }
        else if(first == Type::BOOLEAN && second == Type::BOOLEAN){
            Stack.push(Type(Type::BOOLEAN));
        }
        else {
            Stack.push(Type(Type::UNDEFINED));
        }
        
        generate_instruction("LEQ");
    }

    void JMP0(){
    //Pop the stack and if the value is 0 then jmp to {IL}
        if (Stack.empty()){
            throw runtime_error("Stack underflow");
        }
        // Type value = Stack.top();
        Stack.pop();
        generate_instruction("JMP0");
    }

    void JMP(int instructionLoc) {
        //Unconditionally jmp to {IL}
        generate_instruction("JMP", instructionLoc);
    }

    void push_JMPstack(int instructionLoc){
        JumpStack.push(instructionLoc);
    }

    void LABEL() {
        generate_instruction("LABEL");
    }
};

class SyntaxAnalyzer { 
private:

    Symbol_and_Assembly symbolAndAssembly;
    vector<Token> tokens;
    size_t currentIndex = 0;
    ostream& outSyntaxAnalyzer;

    // Converts string to TokenType
    TokenType stringToTokenType(const string& tokenType) {
        if (tokenType == "keyword") return TokenType::KEYWORD;
        if (tokenType == "identifier") return TokenType::IDENTIFIER;
        if (tokenType == "integer") return TokenType::INTEGER;
        if (tokenType == "real") return TokenType::REAL;
        if (tokenType == "operator") return TokenType::OPERATOR;
        if (tokenType == "separator") return TokenType::SEPARATOR;
        return TokenType::UNKNOWN;
    }
    
    // Converts TokenType to string for printing
    string tokenTypeToString(TokenType type) {
        switch (type) {
            case TokenType::KEYWORD: return "KEYWORD";
            case TokenType::IDENTIFIER: return "IDENTIFIER";
            case TokenType::INTEGER: return "INTEGER";
            case TokenType::REAL: return "REAL";
            case TokenType::OPERATOR: return "OPERATOR";
            case TokenType::SEPARATOR: return "SEPARATOR";
            case TokenType::UNKNOWN: return "UNKNOWN";
            case TokenType::EMPTY: return "EMPTY";
            default: return "UNKNOWN";
        }
    }

    Token lexer(bool print = false) {
        // Check if there are more tokens to read
        if (currentIndex < tokens.size()) {
            Token token = tokens[currentIndex++];

            //will print the token type and value if the print is true
            if(print){
                outSyntaxAnalyzer << "================================================================================" << endl;
                outSyntaxAnalyzer << "\t\t\tToken:" << tokenTypeToString(token.type) << "\tLexeme:" << token.value << endl;
                outSyntaxAnalyzer << "================================================================================" << endl;
            }

            return token;
        } else {
            // Handle the case where there are no more tokens
            return Token(TokenType::EMPTY, "");
        }
    }

    //returns true if the token is $$
    bool check$$(Token token){
        if (token.type == TokenType::SEPARATOR && token.value == "$$") {
            return true;
        }
        else{
            return false;
        }
    }

    bool Empty(){
        Token token = lexer();
        if (check$$(token)) {
            currentIndex--;
            return true;
        }
        else if (token.type == TokenType::EMPTY){
            return true;
        }
        else{
            currentIndex--;
            return false;
        }
    }

public: 
    SyntaxAnalyzer(ostream& syntaxOut, ostream& symbolOut) 
    : symbolAndAssembly(symbolOut),
    outSyntaxAnalyzer(syntaxOut) {  
        if (!syntaxOut.good() || !symbolOut.good()) {
            throw runtime_error("Output stream(s) not in good state");
        }
    }

    void display_RPD() {
        symbolAndAssembly.display_instructions();
        symbolAndAssembly.display_symbol_table();
    }

    //reads all values from the file
    void readFile(const string& input, const int& headerNumber) {
        ifstream file(input);
        string line;
        string tokenValue, tokenType;

        //skips the header information
        for(int i = 0; i < headerNumber; i++){
            getline(file, line);
        }

        //gets token type and token value from each line
        while (getline(file, line)) {
            istringstream lineStream(line);
            lineStream >> tokenType >> tokenValue;
            tokens.push_back(Token(stringToTokenType(tokenType), tokenValue));
        }

        file.close();
    }

    void Rat25S() {
        // $$ <Opt Function Definitions> $$ <Opt Declaration List> $$ <Statement List>$$
        Token token = lexer(true);
        if (check$$(token)) {
            outSyntaxAnalyzer << "<Rat25S> -> $$ <Opt Function Definitions> $$ <Opt Declaration List> $$ <Statement List>$$" << endl;
            outSyntaxAnalyzer << "<Rat25S> -> $$ <Opt Function Definitions>" << endl;
            Opt_Function_Definitions();
            token = lexer(true);
            if (check$$(token)) {
                outSyntaxAnalyzer << "$$ <Opt Declaration List>" << endl;
                Opt_Declaration_List();
                token = lexer(true);
                if (check$$(token)) {
                    outSyntaxAnalyzer << "$$ <Statement List>" << endl;
                    Statement_List();
                    token = lexer(true);
                    if (check$$(token)) {
                        outSyntaxAnalyzer << "$$" << endl;
                        outSyntaxAnalyzer << "Parse complete: Correct syntax" << endl;
                    } else {
                        outSyntaxAnalyzer << "Error: Expected '$$' at the end of Statement_List";
                    }
                } else {
                    outSyntaxAnalyzer << "Error: Expected '$$' at the end of Opt_Declaration_List";
                }
            } else {
                outSyntaxAnalyzer << "Error: Expected '$$' at the end of Opt_Function_Definitions";
            }
        } else {
            outSyntaxAnalyzer << "Error: Expected '$$' at the start of Opt_Function_Definitions";
        }
    }

    void Opt_Function_Definitions(){
        // <Function Definitions> | <Empty>
        if(!Empty()){
            outSyntaxAnalyzer << "<Opt Function Definitions> -> <Function Definitions>" << endl;
             Function_Definitions();
        }
        else{
            outSyntaxAnalyzer << "<Opt Function Definitions> -> <Empty>" << endl;
        }
    }

    void Function_Definitions(){
        //<Function> <fd>
        outSyntaxAnalyzer << "<Function Definitions> -> <Function> <FD>" << endl;
        Function();
        FD();
    }

    void FD(){
        //(ε | <Function Definitions>)
        if (!Empty()) {
            outSyntaxAnalyzer << "<FD> -> <Function Definitions>" << endl;
            Function_Definitions();
        }
        else{
            outSyntaxAnalyzer << "<FD> -> ε" << endl;
        }
    }

    void Function(){
        // function <Identifier> ( <Opt Parameter List> ) <Opt Declaration List> <Body>
        Token token = lexer(true);
        if (token.type == TokenType::KEYWORD && token.value == "function") {
            outSyntaxAnalyzer << "<Function> -> function <Identifier> ( <Opt Parameter List> ) <Opt Declaration List> <Body>" << endl;
            outSyntaxAnalyzer << "<Function> -> function <Identifier>" << endl;   
            Identifier(Type(Type::UNDEFINED));
            token = lexer(true);
            if(token.type == TokenType::SEPARATOR && token.value == "("){
                outSyntaxAnalyzer << "( <Opt Parameter List>" << endl;   
                Opt_Parameter_List();
                token = lexer(true);
                if(token.type == TokenType::SEPARATOR && token.value == ")"){
                    outSyntaxAnalyzer << ") <Opt Declaration List>" << endl;
                    Opt_Declaration_List();
                    outSyntaxAnalyzer << "<Body>" << endl;
                    Body();
                    outSyntaxAnalyzer << "End of Function" << endl;
                } else {
                    outSyntaxAnalyzer << "Error: Expected ')' at the end of Function";
                }
            } else {
                outSyntaxAnalyzer << "Error: Expected '(' at the start of Function";
            }

        } else {
            outSyntaxAnalyzer << "Error: Expected 'function' at the start of Function";
        }
    }

    void Opt_Parameter_List(){
        // Parameter_List() | Empty()
        Token token = lexer();
        currentIndex--;
        if(token.type != TokenType::SEPARATOR && (token.value != ")" || token.value != "$$")){
            outSyntaxAnalyzer << "<Opt Parameter List> -> <Parameter List>" << endl;
            Parameter_List();
        }
        else{
            outSyntaxAnalyzer << "<Opt Parameter List> -> <Empty>" << endl;
        }
    }

    void Parameter_List(){
        //<Parameter> <P>
        outSyntaxAnalyzer << "<Parameter List> -> <Parameter> <P>" << endl;
        Parameter();
        P();
    }

    void P(){
        // (ε |  , <Parameter List>)
        Token token = lexer();
        currentIndex--;
        if(token.type == TokenType::SEPARATOR && token.value == ","){
            Token token = lexer(true);
            outSyntaxAnalyzer << "<P> -> , <Parameter List>" << endl;
            Parameter_List();
        }
        else{
            outSyntaxAnalyzer << "<P> -> ε" << endl;
        }
    }

    void Parameter(){
        //<IDs> <Qualifier>
        outSyntaxAnalyzer << "<Parameter> -> <IDs> <Qualifier>" << endl;
        IDS();
        Qualifier();
    }

    Type Qualifier(){
        // integer | boolean | real
        Token token = lexer(true);
        if(token.type == TokenType::KEYWORD && token.value == "integer"){
            outSyntaxAnalyzer << "<Qualifier> -> integer | boolean | real" << endl;
            return Type(Type::INTEGER);
        }
        else if(token.type == TokenType::KEYWORD && token.value == "boolean"){
            outSyntaxAnalyzer << "<Qualifier> -> integer | boolean | real" << endl;
            return Type(Type::BOOLEAN);
        }
        else if(token.type == TokenType::KEYWORD && token.value == "real"){
            outSyntaxAnalyzer << "<Qualifier> -> integer | boolean | real" << endl;
            return Type(Type::UNDEFINED);
        }
        else {
            outSyntaxAnalyzer << "Error: Invalid Qualifier. Expected token type of integer, boolean, or real";
            return Type(Type::UNDEFINED);
        }
    }

    void Body(){
        // { < Statement List> }
        Token token = lexer(true);
        if (token.type == TokenType::SEPARATOR && token.value == "{") {
            outSyntaxAnalyzer << "<Body> -> { <Statement List> }" << endl;
            outSyntaxAnalyzer << "<Body> -> { <Statement List>" << endl;
            Statement_List();
            Token token = lexer(true);
            if (token.type == TokenType::SEPARATOR && token.value == "}") {
                outSyntaxAnalyzer << "}" << endl;
                outSyntaxAnalyzer << "End of Body" << endl;
            } else {
                outSyntaxAnalyzer << "Error: Expected '}' at the end of Body";
            }
        } else {
            outSyntaxAnalyzer << "Error: Expected '{' at the start of Body";
        }
    }

    void Opt_Declaration_List(){
        // Declaration_List() | Empty()
        Token token = lexer();
        currentIndex--;
        if(token.type == TokenType::KEYWORD && (token.value == "integer" || token.value == "real" || token.value == "boolean")){
            outSyntaxAnalyzer << "<Opt Declaration List> -> <Declaration List>" << endl;
            Declaration_List();
        }
        else{
            outSyntaxAnalyzer << "<Opt Declaration List> -> <Empty>" << endl;
        }
    }

    void Declaration_List(){
        // <Declaration> ; <D>
        outSyntaxAnalyzer << "<Declaration List> -> <Declaration> ; <D>" << endl;
        Declaration();
        Token token = lexer(true);
        if(token.type == TokenType::SEPARATOR && token.value == ";"){
            outSyntaxAnalyzer << "; <D>" << endl;
            D();
        } else {
            outSyntaxAnalyzer << "Error: Expected ';' at the end of Declaration_List";
        }
    }

    void D(){
        // (ε | <Declaration List>)
        Token token = lexer();
        currentIndex--;
        if(token.type != TokenType::SEPARATOR && (token.value != "{" || token.value != "$$")){
            outSyntaxAnalyzer << "<D> -> <Declaration List>" << endl;
            Declaration_List();
        }
        else{
            outSyntaxAnalyzer << "<D> -> ε" << endl;
        }
    }

    void Declaration(){
        // <Qualifier > <IDs>
        outSyntaxAnalyzer << "<Declaration> -> <Qualifier> <IDs>" << endl;
        Type value = Qualifier();
        IDS(value);
    }

    // Duplicate IDS, Identifier, id ===============================================
    void IDS(){
        Identifier();
        id();
    }

    void IDS(Type value){
        outSyntaxAnalyzer << "<IDs> -> <Identifier> <id>" << endl;
        // <Identifier> <id>
        Identifier(value);
        id(value);
    }

    void id(){
        //  (ε | , <IDs>)
        Token token = lexer();
        currentIndex--;
        if(token.type == TokenType::SEPARATOR && token.value == ","){
            Token token = lexer(true);
            outSyntaxAnalyzer << "<id> -> , <IDs>" << endl;
            IDS();
        }
        else{
            outSyntaxAnalyzer << "<id> -> ε" << endl;
        }
    }

    void id(Type value){
        //  (ε | , <IDs>)
        Token token = lexer();
        currentIndex--;
        if(token.type == TokenType::SEPARATOR && token.value == ","){
            Token token = lexer(true);
            outSyntaxAnalyzer << "<id> -> , <IDs>" << endl;
            IDS(value);
        }
        else{
            outSyntaxAnalyzer << "<id> -> ε" << endl;
        }
    }

    void Identifier(){
        // <Identifier> ::= <IDENTIFIER>
        Token token = lexer(true);
        if(token.type == TokenType::IDENTIFIER) {
            outSyntaxAnalyzer << "<Identifier> -> Identifier" << endl;
                if(!symbolAndAssembly.getAddress(token.value)){
                    outSyntaxAnalyzer << "Error: Variable " << token.value << " not found in symbol table." << endl;
                }
                else{
                    symbolAndAssembly.SIN(token.value);
                }
        } else {
            outSyntaxAnalyzer << "Error: Invalid Identifier. Expected token type of IDENTIFIER";
        }
    }

    void Identifier(Type valueType){
        // <Identifier> ::= <IDENTIFIER>
        Token token = lexer(true);
        if(token.type == TokenType::IDENTIFIER) {
            outSyntaxAnalyzer << "<Identifier> -> Identifier" << endl;
            if(valueType != Type::UNDEFINED){
                symbolAndAssembly.generate_symbol(token.value, valueType);
            }
        } else {
            outSyntaxAnalyzer << "Error: Invalid Identifier. Expected token type of IDENTIFIER";
        }
    }

    // =============================================================================

    void Statement_List(){
        //<Statement><S>
        outSyntaxAnalyzer << "<Statement List> -> <Statement> <S>" << endl;
        Statement();
        S();
    }

    void S(){
        //  (ε | <Statement List>)
        Token token = lexer();
        currentIndex--;
        if(token.type != TokenType::SEPARATOR && (token.value != "}" || token.value != "$$")){
            outSyntaxAnalyzer << "<S> -> <Statement List>" << endl;
            Statement_List();
        }
        else{
            outSyntaxAnalyzer << "<S> -> ε" << endl;
        }
    }

    void Statement(){
        // <Compound> | <Assign> | <If> | <Return> | <Print> | <Scan> | <While>
        Token token = lexer(true);
        outSyntaxAnalyzer << "<Statement> -> ";

        // <Compound> ::= { <Statement List> }
        // <Assign> ::= <Identifier> = <Expression> ;
        // <If> ::= if ( <Condition> )  <Statement> <if> 
        // <Return> ::= return <r>
        // <Print> ::= print ( <Expression> ) ;
        // <Scan> ::= scan ( <IDs> ) ;
        // <While> ::= while ( <Condition> ) <Statement> endwhile
        if(token.type == TokenType::SEPARATOR && token.value == "{"){
            outSyntaxAnalyzer << "<Compound>" << endl;
            outSyntaxAnalyzer << "<Compound> -> { <Statement List> }" << endl;
            outSyntaxAnalyzer << "<Compound> -> { <Statement List>" << endl;
            Statement_List();
            token = lexer(true);
            if(token.type == TokenType::SEPARATOR && token.value == "}"){
                outSyntaxAnalyzer << "}" << endl;
                outSyntaxAnalyzer << "End of Compound" << endl;
            }
            else{
                outSyntaxAnalyzer << "Error in beggining '}' for <Compound>'";
            }

        }
        else if(token.type == TokenType::KEYWORD && token.value == "if"){
            outSyntaxAnalyzer << "<If>" << endl;
            outSyntaxAnalyzer << "<If> -> if ( <Condition> ) <Statement> <if>" << endl;
            outSyntaxAnalyzer << "<If> -> if" << endl;
            token = lexer(true);
            if(token.type == TokenType::SEPARATOR && token.value == "("){
                outSyntaxAnalyzer << "( <Condition>" << endl;
                Condition();
                token = lexer(true);
                if(token.type == TokenType::SEPARATOR && token.value == ")"){
                    outSyntaxAnalyzer << ") <Statement> <if>" << endl;
                    Statement();
                    _if();
                }
                else{
                    outSyntaxAnalyzer << "Error in beginning ')' for <If>";
                }
            }
            else{
                outSyntaxAnalyzer << "Error in beginning '(' for <If>";
            }
        }
        else if(token.type == TokenType::KEYWORD && token.value == "return"){
            outSyntaxAnalyzer << "<Return>" << endl;
            outSyntaxAnalyzer << "<Return> -> return <r>" << endl;
            outSyntaxAnalyzer << "<Return> -> return" << endl;
            r();
        }
        else if(token.type == TokenType::KEYWORD && token.value == "print"){
            outSyntaxAnalyzer << "<Print>" << endl;
            outSyntaxAnalyzer << "<Print> -> print ( <Expression> )" << endl;
            outSyntaxAnalyzer << "<Print> -> print" << endl;
            token = lexer(true);
            if(token.type == TokenType::SEPARATOR && token.value == "("){
                outSyntaxAnalyzer << "( <Expression>" << endl;
                Expression();
                symbolAndAssembly.SOUT();
                token = lexer(true);
                if(token.type == TokenType::SEPARATOR && token.value == ")"){
                    outSyntaxAnalyzer << ")" << endl;
                    token = lexer(true);
                    if(token.type == TokenType::SEPARATOR && token.value == ";"){
                        outSyntaxAnalyzer << ";" << endl;
                        outSyntaxAnalyzer << "End of Print" << endl;
                    }
                    else{
                        outSyntaxAnalyzer << "Error in ';' for <Print>";
                    }
                }
                else{
                    outSyntaxAnalyzer << "Error in beginning ')' for <Print>";
                }
            }
            else{
                outSyntaxAnalyzer << "Error in beginning '(' for <Print>";
            }
        }
        else if(token.type == TokenType::KEYWORD && token.value == "scan"){
            outSyntaxAnalyzer << "<Scan>" << endl;
            outSyntaxAnalyzer << "<Scan> -> scan ( <IDs> );" << endl;
            outSyntaxAnalyzer << "<Scan> -> scan" << endl;
            token = lexer(true);

            if(token.type == TokenType::SEPARATOR && token.value == "("){
                outSyntaxAnalyzer << "( <IDs>" << endl;
                IDS();
                token = lexer(true);

                if(token.type == TokenType::SEPARATOR && token.value == ")"){
                    
                    outSyntaxAnalyzer << ")" << endl;
                    token = lexer(true);
                    if(token.type == TokenType::SEPARATOR && token.value == ";"){
                        outSyntaxAnalyzer << ";" << endl;
                        outSyntaxAnalyzer << "End of Scan" << endl;
                    }
                    else{
                        outSyntaxAnalyzer << "Error in ';' for <Scan>";
                    }
                }
                else{
                    outSyntaxAnalyzer << "Error in beginning ')' for <Scan>";
                }
            }
            else{
                outSyntaxAnalyzer << "Error in beginning '(' for <Scan>";
            }
        }
    
        else if(token.type == TokenType::KEYWORD && token.value == "while"){
            outSyntaxAnalyzer << "<While>" << endl;
            outSyntaxAnalyzer << "<While> -> while ( <Condition> ) <Statement> endwhile" << endl;
            outSyntaxAnalyzer << "<While> -> while" << endl;
            int instruction_Addr = symbolAndAssembly.getInstructionAddr();
            symbolAndAssembly.LABEL();
            token = lexer(true);
            if(token.type == TokenType::SEPARATOR && token.value == "("){
                outSyntaxAnalyzer << "( <Condition>" << endl;
                Condition();
                token = lexer(true);
                if(token.type == TokenType::SEPARATOR && token.value == ")"){
                    outSyntaxAnalyzer << ") <Statement>" << endl;
                    Statement();
                    token = lexer(true);

                    symbolAndAssembly.JMP(instruction_Addr);
                    symbolAndAssembly.back_patch(symbolAndAssembly.getInstructionAddr());
                    symbolAndAssembly.LABEL();
                    
                    if(token.type == TokenType::KEYWORD && token.value == "endwhile"){
                    outSyntaxAnalyzer << "endwhile" << endl;
                    }
                    else{
                    outSyntaxAnalyzer << "Error in 'endwhile' for <While>";
                    }
                }
                else{
                    outSyntaxAnalyzer << "Error in beginning ')' for <While>";
                }
            }
            else{
                outSyntaxAnalyzer << "Error in beginning '(' for <While>";
            }
        }
        else if(token.type == TokenType::IDENTIFIER){
            outSyntaxAnalyzer << "<Assign>" << endl;
            outSyntaxAnalyzer << "<Assign> -> <Identifier> = <Expression> ;" << endl;
            outSyntaxAnalyzer << "<Assign> -> <Identifier>" << endl;
            string var = token.value; // Save the variable
            Token token = lexer(true);
                if(token.type == TokenType::OPERATOR && token.value == "="){
                    outSyntaxAnalyzer << "= <Expression> ;" << endl;
                    Expression();
                    token = lexer(true);
                    int memoryLoc = symbolAndAssembly.getAddress(var);
                    symbolAndAssembly.POPM(memoryLoc, var);
                    if(token.type == TokenType::SEPARATOR && token.value == ";"){
                        outSyntaxAnalyzer << ";" << endl;
                        outSyntaxAnalyzer << "End of Assign" << endl;
                    }
                    else{
                        outSyntaxAnalyzer << "Error in ';' for <Assign>";
                    }
                }
                else{
                    outSyntaxAnalyzer << "Error in '=' for <Assign>";
                }
        }
        else{
            outSyntaxAnalyzer << "Error: Invalid Statement. Expected statement type of <Compound>, <Assign>, <If>, <Return>, <Print>, <Scan>, or <While>";
        }

    }

    void _if(){
        //endif | else <Statement> endif
        Token token = lexer(true);
        if(token.type == TokenType::KEYWORD && token.value == "endif"){
            symbolAndAssembly.back_patch(symbolAndAssembly.getInstructionAddr());
            symbolAndAssembly.LABEL();
            outSyntaxAnalyzer << "<if> -> endif" << endl;
        }
        else if(token.type == TokenType::KEYWORD && token.value == "else"){
            outSyntaxAnalyzer << "<if> -> else <Statement> endif" << endl;
            Statement();
            token = lexer(true);
            if(token.type == TokenType::KEYWORD && token.value == "endif"){
                symbolAndAssembly.back_patch(symbolAndAssembly.getInstructionAddr());
                symbolAndAssembly.LABEL();
                outSyntaxAnalyzer << "endif" << endl;
                outSyntaxAnalyzer << "End of <If>" << endl;
            }
            else{
                outSyntaxAnalyzer << "Error in 'endif' for <if>";
            }
        }
        else{
            outSyntaxAnalyzer << "Error, expected 'endif' or 'else' for <if>";
        }

    }

    void r(){
        //  (; | <Expression> ;)
        Token token = lexer();
        currentIndex--;
        if(token.type == TokenType::SEPARATOR && token.value == ";"){
            token = lexer(true);
            outSyntaxAnalyzer << "<r> -> ;" << endl;
        }
        else{
            outSyntaxAnalyzer << "<r> -> <Expression> ;" << endl;
            Expression();
            token = lexer(true);
            if(token.type == TokenType::SEPARATOR && token.value == ";"){
                outSyntaxAnalyzer << ";" << endl;
                outSyntaxAnalyzer << "End of <Return>" << endl;
            }
            else{
                outSyntaxAnalyzer << "Error in ';' for <Return>";
            }
            
        }

    }

    void Condition(){
        //<Expression> <Relop> <Expression>
        outSyntaxAnalyzer << "<Condition> -> <Expression> <Relop> <Expression>" << endl;
        Expression();
        Relop();
    }

    void Relop(){
        // == | != | > | < | <= | =>
        Token token = lexer(true);
        if(token.type == TokenType::OPERATOR && 
            (token.value == "==" || token.value == "!=" || token.value == ">" || token.value == "<" || token.value == "<=" || token.value == "=>")){
            outSyntaxAnalyzer << "<Relop> -> == | != | > | < | <= | =>" << endl;
            
            Expression();
            if(token.value == ">"){
                symbolAndAssembly.GRT();
                symbolAndAssembly.push_JMPstack(symbolAndAssembly.getInstructionAddr() - 1);
                symbolAndAssembly.JMP0();
            }
            else if(token.value == "<"){
                symbolAndAssembly.LES();
                symbolAndAssembly.push_JMPstack(symbolAndAssembly.getInstructionAddr() - 1);
                symbolAndAssembly.JMP0();
            }
            else if(token.value == "=="){
                symbolAndAssembly.EQU();
                symbolAndAssembly.push_JMPstack(symbolAndAssembly.getInstructionAddr() - 1);
                symbolAndAssembly.JMP0();
            }
            else if(token.value == "!="){
                symbolAndAssembly.NEQ();
                symbolAndAssembly.push_JMPstack(symbolAndAssembly.getInstructionAddr() - 1);
                symbolAndAssembly.JMP0();
            }
            else if(token.value == "=>"){
                symbolAndAssembly.GEQ();
                symbolAndAssembly.push_JMPstack(symbolAndAssembly.getInstructionAddr() - 1);
                symbolAndAssembly.JMP0();
            }
            else if(token.value == "<="){
                symbolAndAssembly.LEQ();
                symbolAndAssembly.push_JMPstack(symbolAndAssembly.getInstructionAddr() - 1);
                symbolAndAssembly.JMP0();
            }
        }
        else{
            outSyntaxAnalyzer << "Error in Relop. Expected token type of OPERATOR with value ==, !=, >, <, <=, or =>";
        }
    }

    void Expression(){
        //<Term> <E>
        outSyntaxAnalyzer << "<Expression> -> <Term> <E>" << endl;
        Term();
        E();
    }

    void E() {
        //  + <Term> <E> | - <Term><E> | ɛ
        Token token = lexer();
        string operator_addition_subtraction = token.value;
        currentIndex--;
        if(token.type == TokenType::OPERATOR &&
            (token.value == "+" || token.value == "-")){
            token = lexer(true);
            outSyntaxAnalyzer << "<E> -> + <Term> <E> | - <Term><E>" << endl;
            Term();
            if(operator_addition_subtraction == "+"){
                symbolAndAssembly.A();
            }
            else if(operator_addition_subtraction == "-"){
                symbolAndAssembly.S();
            }
            E();
        }
        else{
            outSyntaxAnalyzer << "<E> -> ε" << endl;
        }

    }

    void T(){
        // * <Factor> <T> | / <Factor> <T> | ɛ
        Token token = lexer();
        string var = token.value; //get a copy of the "*" or "/"
        currentIndex--;
        if(token.type == TokenType::OPERATOR &&
            (token.value == "*" || token.value == "/")){
            token = lexer(true);
            outSyntaxAnalyzer << "<T> -> * <Factor> <T> | / <Factor> <T>" << endl;
            Factor();

            if(var == "*"){
                symbolAndAssembly.M();
            }
            else if(var == "/"){
                symbolAndAssembly.D();
            }

            T();
        } 
        else{
            outSyntaxAnalyzer << "<T> -> ε" << endl;
        }
    }

    void Term(){
        // <Factor> <T>
        outSyntaxAnalyzer << "<Term> -> <Factor> <T>" << endl;
        Factor();
        T();
    }
    
    void Factor() {
        // - <Primary> | <Primary>
        Token token = lexer();
        currentIndex--;
        if(token.type == TokenType::OPERATOR && token.value == "-"){
            token = lexer(true);
            outSyntaxAnalyzer << "<Factor> -> - <Primary>" << endl;
            Primary();
        } else {
            outSyntaxAnalyzer << "<Factor> -> <Primary>" << endl;
            Primary();
        }
    }

    
    void Primary() {
        // <Primary> ::= <Identifier> | <Integer> | <Identifier> ( <IDs> ) | ( <Expression> ) |
        //<Real> | true | false
         Token token = lexer(true);
         if(token.type == TokenType::IDENTIFIER){
            Token oldToken = token;
             token = lexer();
             currentIndex--;
             if (token.type == TokenType::SEPARATOR && token.value == "("){
                 token = lexer(true);
                 outSyntaxAnalyzer << "<Identifier> ( <IDs> ) ->"; 
                 outSyntaxAnalyzer << " <Identifier> (" << endl;
                 IDS();
 
                 token = lexer(true);
                 if(token.type == TokenType::SEPARATOR && token.value == ")"){
                     outSyntaxAnalyzer << "<Identifier> ( <IDs> )" << endl;
                 }
                 else{
                     outSyntaxAnalyzer << "Error in Primary. Expected token type of ) for <Identifier> ( <IDs> )" << endl;
                 }
             }
            else{
                symbolAndAssembly.PUSHM(oldToken.value, symbolAndAssembly.getAddress(oldToken.value));
                outSyntaxAnalyzer << "<Primary> -> <Identifier> | <Integer> | <Identifier> | true, false" << endl;
            }
         }
         else if (token.type == TokenType::INTEGER) {
            //ADD IN PUSHM for the identifier, PUSHI for the random Integers, CHANGE
            symbolAndAssembly.PUSHI(Type(Type::INTEGER));
            outSyntaxAnalyzer << "<Primary> -> <Identifier> | <Integer> | <Real> | true, false" << endl;
        } 
        else if(token.type == TokenType::REAL){
            outSyntaxAnalyzer << "<Primary> -> <Identifier> | <Integer> | <Real> | true, false" << endl;
        }
        else if(token.type == TokenType::KEYWORD && (token.value == "true" || token.value == "false")){
            symbolAndAssembly.PUSHB(Type(Type::INTEGER));
            outSyntaxAnalyzer << "<Primary> -> <Identifier> | <Integer> | <Real> | true, false" << endl;
        }
        else if (token.type == TokenType::SEPARATOR && token.value == "("){
             outSyntaxAnalyzer << "( <Expression> )" << endl;
             outSyntaxAnalyzer << "( <Expression> ) -> (" << endl;
             Expression();
             token = lexer(true);
             if(token.type == TokenType::SEPARATOR && token.value == ")"){
                 outSyntaxAnalyzer << "( <Expression> )" << endl;
             }
             else{
                 outSyntaxAnalyzer << "Error in Primary. Expected token type of ) for <Identifier> ( <IDs> )";
             }
         }
         else{
             outSyntaxAnalyzer << "Error in Primary. <Identifier> | <Integer> | <Identifier> ( <IDs> ) | ( <Expression> ) | <Real> | true | false";
         }
     }

};

#endif
