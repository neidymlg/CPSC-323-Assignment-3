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

struct Value {
    enum Type { INTEGER, BOOLEAN, IDENTIFIER, REAL, UNDEFINED };
    Type type;
    optional<int> intVal;
    optional<bool> boolVal;
    optional<double> realVal;
    string identifier;

    // Default constructor
    Value() : type(UNDEFINED) {}

    // Constructors for typed but valueless
    Value(Type t) : type(t) {
        switch(t) {
            case INTEGER: intVal = nullopt; break;
            case BOOLEAN: boolVal = nullopt; break;
            case REAL: realVal = nullopt; break;
            default: break;
        }
    }

    // Regular constructors
    Value(int v) : type(INTEGER), intVal(v) {}
    Value(bool v) : type(BOOLEAN), boolVal(v) {}
    Value(double v) : type(REAL), realVal(v) {}
    Value(string id) : type(IDENTIFIER), identifier(id) {}

    bool hasValue() const {
        switch(type) {
            case INTEGER: return intVal.has_value();
            case BOOLEAN: return boolVal.has_value();
            case REAL: return realVal.has_value();
            case IDENTIFIER: return !identifier.empty();
            default: return false;
        }
    }
};


// Redirects the output to a file
// changes cout to a file stream
class OutputRedirector {
private:
    ofstream file;
    streambuf* original_buf;
public:
    explicit OutputRedirector(const string& filename) {
        original_buf = cout.rdbuf();
        file.open(filename);
        if (file.is_open()) {
            cout.rdbuf(file.rdbuf());
        }
    }

    ~OutputRedirector() {
        if (file.is_open()) {
            file.close();
            cout.rdbuf(original_buf);
        }
    }
};

class Symbol_and_Assembly{
private:
    int memoryAddr = 10000;
    int instructionAddr= 1;

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
        Value value;
    };


    unordered_map<string, SymbolInfo> SymbolTable;
    
    stack<Value> Stack;
    stack<int> JumpStack;

    int getValue(const Value& val) {
        switch (val.type) {
            case Value::INTEGER:
                if (!val.intVal.has_value()) {
                    throw runtime_error("Integer value not initialized");
                }
                return val.intVal.value();
            case Value::BOOLEAN:
                if (!val.boolVal.has_value()) {
                    throw runtime_error("Boolean value not initialized");
                }
                return val.boolVal.value();
            case Value::IDENTIFIER: {
                auto it = SymbolTable.find(val.identifier);
                if (it == SymbolTable.end()) {
                    throw runtime_error("Undefined identifier: " + val.identifier);
                }
                return getValue(it->second.value);
            }
            default:
                throw runtime_error("Invalid value type for arithmetic operation");
        }
    }
    
public:
    Symbol_and_Assembly(){
        //reserve 1000 spaces in vector for Instruction table
        InstructTable.reserve(100);
    }

     void display_instructions(ostream& out) {
        out << "\n=== INSTRUCTION TABLE ===\n";
        out << "ADDR\tOPERATOR\tOPERAND\n";
        out << "------------------------\n";
        for (const auto& instr : InstructTable) {
            out << instr.ADDR << "\t" << instr.Operator << "\t\t";
            if (instr.Operand.has_value()) {
                out << instr.Operand.value();
            } else {
                out << "-";
            }
            out << "\n";
        }
    }

    void display_stack(ostream& out) {
        out << "\n=== STACK CONTENTS ===\n";
        out << "TOP\n";
        out << "----\n";
        
        // Create a temporary stack to preserve original
        auto tempStack = Stack;
        while (!tempStack.empty()) {
            out << tempStack.top().type << "\n";
            tempStack.pop();
        }
        out << "BOTTOM\n";
    }

    void display_symbol_table(ostream& out) {
        out << "\n=== SYMBOL TABLE ===\n";
        out << "NAME\t\tADDRESS\t\tType\t\tValue\n";
        out << "--------------------------------\n";
        for (const auto& entry : SymbolTable) {
            out << entry.first << "\t\t" << entry.second.memoryADDR;
    
            switch (entry.second.value.type) {
                case Value::INTEGER:
                    out << "\t\t" << "Integer" << "\t\t";
                    if (entry.second.value.intVal.has_value()) {
                        out << entry.second.value.intVal.value() << endl;
                    } else {
                        out << "-" << endl;
                    }
                    break;
    
                case Value::BOOLEAN:
                    out << "\t\t" << "Boolean" << "\t\t";
                    if (entry.second.value.boolVal.has_value()) {
                        out << (entry.second.value.boolVal.value() ? "true" : "false") << endl;
                    } else {
                        out << "-" << endl;
                    }
                    break;
    
                case Value::IDENTIFIER:
                    out << "\t\t" << "Identifier" << "\t\t";
                    if (!entry.second.value.identifier.empty()) {
                        out << entry.second.value.identifier << endl;
                    } else {
                        out << "-" << endl;
                    }
                    break;
    
                case Value::REAL:
                    out << "\t\t" << "Real" << "\t\t";
                    if (entry.second.value.realVal.has_value()) {
                        out << entry.second.value.realVal.value() << endl;
                    } else {
                        out << "-" << endl;
                    }
                    break;
    
                default:
                    out << "\t\t" << "Undefined" << "\t\t-\n";
                    break;
            }
        }
    }
    
    //Add into Instruction Table
    void generate_instruction(string op, optional<int> oprnd = nullopt){
        InstructTable.emplace_back(instructionAddr++, op, oprnd);
    }

    //Add a variable into the Symbol table
    void generate_symbol(string var, Value value){
        SymbolTable[var].memoryADDR = memoryAddr;
        SymbolTable[var].value =  value; 
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
    void PUSHI(int intValue){
        //Pushes the {Integer Value} onto the Top of the Stack (TOS)
        Stack.push(Value(intValue));
    }

    void PUSHB(bool boolValue) {
        Stack.push(Value(boolValue));
    }

    //PUSHM
    void PUSHM(string var){
        //Pushes the value stored at {ML} onto TOS
        auto it = SymbolTable.find(var);
        if (it != SymbolTable.end()) {
            Stack.push(it->second.value);
        } else {
            throw runtime_error("Undefined variable: " + var);
        }
    }

    void POPM(int memoryLoc, string var){
        //Pops the value from the top of the stack and stores it at {ML}
        if(Stack.empty()){
            throw runtime_error("Stack underflow");
        }
            Value stackValue = Stack.top();
            Stack.pop();

            SymbolTable[var].value = stackValue;

            generate_instruction("POPM", memoryLoc);
    }

    void SOUT(){
        // Pops the value from the top of the stack and prints it and adds instruction
        //generate_instruction("SOUT");
        if (Stack.empty()) {
            throw runtime_error("Stack underflow");
        }
        Value value = Stack.top();
        Stack.pop();
        switch(value.type) {
            case Value::INTEGER:
                cout << "Output: " << value.intVal.value() << endl;
                break;
            case Value::BOOLEAN:
                cout << "Output: " << (value.boolVal.value() ? "1" : "0") << endl;
                break;
            case Value::REAL: // most likely delete (i forgot if he said no real anymore)
                cout << "Output: " << value.realVal.value() << endl;
                break;
            case Value::IDENTIFIER:
                cout << "Output: " << value.identifier << endl;
                break;
            default:
                cout << "Output: Undefined" << endl;
        }
        generate_instruction("SOUT");
    }

    void SIN(string var){
        //generate_instruction("SIN");
        auto it = SymbolTable.find(var);

        if (it == SymbolTable.end()) {
            throw runtime_error("Undefined variable: " + var);
        }
        Value& expectedValue = it->second.value;
        Value inputValue;
        cout << "Enter value for " << var << ": ";

        if (expectedValue.type == Value::BOOLEAN) {
            string userInput;
            cin >> userInput;
            if (userInput == "1" || userInput == "true") { // maybe change to "0" and "1"
                inputValue = Value(true);
            } else if (userInput == "0" || userInput == "false") {
                inputValue = Value(false);
            } else {
                throw runtime_error("Invalid boolean input (must be 'true' or 'false')");
            }
        } else if (expectedValue.type == Value::INTEGER) {
            int num;
            cin >> num;
            inputValue = Value(num);
        } else if (expectedValue.type == Value::REAL) {
            double num;
            cin >> num;
            inputValue = Value(num);
        } else {
            throw runtime_error("Unsupported type for SIN");
        }
        SymbolTable[var].value = inputValue;
        generate_instruction("SIN", SymbolTable[var].memoryADDR);
    }

    void A() {
        if (Stack.size() < 2) {
            return;
        }
    
        Value first = Stack.top(); 
        Stack.pop();
        Value second = Stack.top(); 
        Stack.pop();
   
        try {
            int firstVal = getValue(first);
            int secondVal = getValue(second);
            
            if(first.type == Value::BOOLEAN && second.type == Value::BOOLEAN){
                Stack.push(Value(firstVal || secondVal));
            }
            else{
                Stack.push(Value(secondVal + firstVal));
            }
        } catch (const std::exception& e) {
            throw runtime_error("Addition error");
        }
    }

    void S(){
        // Pop the first two items from stack and push the difference onto the TOS
        if (Stack.size() < 2) {
            return;
        }
    
        Value first = Stack.top(); 
        Stack.pop();
        Value second = Stack.top(); 
        Stack.pop();
    
        try {
            int firstVal = getValue(first);
            int secondVal = getValue(second);
            if(first.type == Value::BOOLEAN && second.type == Value::BOOLEAN ){
                Stack.push(Value(secondVal != firstVal));
            }           
            else{
                Stack.push(Value(secondVal - firstVal));
            }
        } catch (const std::exception& e) {
            throw runtime_error("Subtraction error");
        }
    }

    void M(){
        // Pop the first two items from stack and push the product onto the TOS
        if (Stack.size() < 2) {
            return;
        }
    
        Value first = Stack.top(); 
        Stack.pop();
        Value second = Stack.top(); 
        Stack.pop();
    
        try {
            int firstVal = getValue(first);
            int secondVal = getValue(second);
            if(first.type == Value::BOOLEAN && second.type == Value::BOOLEAN ){
                Stack.push(Value(firstVal && secondVal));
            }           
            else{
                Stack.push(Value(secondVal * firstVal));
            }
        } catch (const std::exception& e) {
            throw runtime_error("Multiplication error");
        }
    }

    void D(){
        // Pop the first two items from stack and push the result onto the TOS
        if (Stack.size() < 2) {
            return;
        }
    
        Value first = Stack.top(); 
        Stack.pop();
        Value second = Stack.top(); 
        Stack.pop();
    
        try {
            int firstVal = getValue(first);
            int secondVal = getValue(second);
            if(first.type == Value::BOOLEAN && second.type == Value::BOOLEAN){
                Stack.push(Value(secondVal == firstVal));
            }
            else if(firstVal == 0){
                Stack.push(Value());
            }           
            else{
                Stack.push(Value(secondVal / firstVal));
            }
        } catch (const std::exception& e) {
            throw runtime_error("Division error");
        }
    }

    void GRT(){
        //Pops two items from the stack and pushes 1 onto TOS if second item is larger otherwise push 0
        if (Stack.size() < 2) {
            return;
        }
    
        Value first = Stack.top(); 
        Stack.pop();
        Value second = Stack.top(); 
        Stack.pop();
    
        try {
            int firstVal = getValue(first);
            int secondVal = getValue(second);
            Stack.push(Value(secondVal > firstVal));
        } catch (const std::exception& e) {
            throw runtime_error("GRT error");
        }
    }

    void LES() {
        //Pops two items from the stack and pushes 1 onto TOS if the second item is smaller than first item otherwise push 0
        if (Stack.size() < 2) {
            return;
        }
    
        Value first = Stack.top(); 
        Stack.pop();
        Value second = Stack.top(); 
        Stack.pop();
    
        try {
            int firstVal = getValue(first);
            int secondVal = getValue(second);
            Stack.push(Value(secondVal < firstVal));
        } catch (const std::exception& e) {
            throw runtime_error("LES error");
        }
    }

    void EQU() {
        //Pops two items from the stack and pushes 1 onto TOS if they are equal otherwise push 0
        if (Stack.size() < 2) {
            return;
        }
    
        Value first = Stack.top(); 
        Stack.pop();
        Value second = Stack.top(); 
        Stack.pop();
    
        try {
            int firstVal = getValue(first);
            int secondVal = getValue(second);
            Stack.push(Value(secondVal == firstVal));
        } catch (const std::exception& e) {
            throw runtime_error("EQU error");
        }
    }

    void NEQ() {
        //Pops two items from the stack and pushes 1 onto TOS if they are not equal otherwise push 0
        if (Stack.size() < 2) {
            return;
        }
    
        Value first = Stack.top(); 
        Stack.pop();
        Value second = Stack.top(); 
        Stack.pop();
    
        try {
            int firstVal = getValue(first);
            int secondVal = getValue(second);
            Stack.push(Value(secondVal != firstVal));
        } catch (const std::exception& e) {
            throw runtime_error("NEQ error");
        }

    }

    void GEQ() {
        //Pops two items from the stack and pushes 1 onto TOS if second item is larger or equal otherwise push 0
        if (Stack.size() < 2) {
            return;
        }
    
        Value first = Stack.top(); 
        Stack.pop();
        Value second = Stack.top(); 
        Stack.pop();
    
        try {
            int firstVal = getValue(first);
            int secondVal = getValue(second);
            Stack.push(Value(secondVal >= firstVal));
        } catch (const std::exception& e) {
            throw runtime_error("GEQ error");
        }

    }

    void LEQ() {
        //Pops two items from the stack and pushes 1 onto TOS if second item is Less or equal otherwise push 0
        if (Stack.size() < 2) {
            return;
        }
    
        Value first = Stack.top(); 
        Stack.pop();
        Value second = Stack.top(); 
        Stack.pop();
    
        try {
            int firstVal = getValue(first);
            int secondVal = getValue(second);
            Stack.push(Value(secondVal <= firstVal));
        } catch (const std::exception& e) {
            throw runtime_error("LEQ error");
        }
    }

    void JMP0(){
    //Pop the stack and if the value is 0 then jmp to {IL}
        if (Stack.empty()){
            return;
        }
        Value value = Stack.top();
        Stack.pop();
        if(getValue(value) == 0){
            generate_instruction("JMP0");
        }
    }

    void JMP(int instructionLoc) {
        //Unconditionally jmp to {IL}
        generate_instruction("JMP", instructionLoc);
    }

    void push_JMPstack(int instructionLoc){
        JumpStack.push(instructionLoc);
    }

};

class SyntaxAnalyzer { 
private:

    Symbol_and_Assembly symbolAndAssembly;
    vector<Token> tokens;
    size_t currentIndex = 0;

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
                cout << "================================================================================" << endl;
                cout << "\t\t\tToken:" << tokenTypeToString(token.type) << "\tLexeme:" << token.value << endl;
                cout << "================================================================================" << endl;
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

    void display_RPD(const string& filename) {

        ofstream outFile(filename);
        
        if (outFile.is_open()) {
            // Display all components
            symbolAndAssembly.display_instructions(outFile);
            //symbolAndAssembly.display_stack(outFile);
            symbolAndAssembly.display_symbol_table(outFile);
            
            outFile.close();
        } else {
            cerr << "Error opening debug output file\n";
        }
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
            cout << "<Rat25S> -> $$ <Opt Function Definitions> $$ <Opt Declaration List> $$ <Statement List>$$" << endl;
            cout << "<Rat25S> -> $$ <Opt Function Definitions>" << endl;
            Opt_Function_Definitions();
            token = lexer(true);
            if (check$$(token)) {
                cout << "$$ <Opt Declaration List>" << endl;
                Opt_Declaration_List();
                token = lexer(true);
                if (check$$(token)) {
                    cout << "$$ <Statement List>" << endl;
                    Statement_List();
                    token = lexer(true);
                    if (check$$(token)) {
                        cout << "$$" << endl;
                        cout << "Parse complete: Correct syntax" << endl;
                    } else {
                        cout << "Error: Expected '$$' at the end of Statement_List";
                    }
                } else {
                    cout << "Error: Expected '$$' at the end of Opt_Declaration_List";
                }
            } else {
                cout << "Error: Expected '$$' at the end of Opt_Function_Definitions";
            }
        } else {
            cout << "Error: Expected '$$' at the start of Opt_Function_Definitions";
        }
    }

    void Opt_Function_Definitions(){
        // <Function Definitions> | <Empty>
        if(!Empty()){
            cout << "<Opt Function Definitions> -> <Function Definitions>" << endl;
             Function_Definitions();
        }
        else{
            cout << "<Opt Function Definitions> -> <Empty>" << endl;
        }
    }

    void Function_Definitions(){
        //<Function> <fd>
        cout << "<Function Definitions> -> <Function> <FD>" << endl;
        Function();
        FD();
    }

    void FD(){
        //(ε | <Function Definitions>)
        if (!Empty()) {
            cout << "<FD> -> <Function Definitions>" << endl;
            Function_Definitions();
        }
        else{
            cout << "<FD> -> ε" << endl;
        }
    }

    void Function(){
        // function <Identifier> ( <Opt Parameter List> ) <Opt Declaration List> <Body>
        Token token = lexer(true);
        if (token.type == TokenType::KEYWORD && token.value == "function") {
            cout << "<Function> -> function <Identifier> ( <Opt Parameter List> ) <Opt Declaration List> <Body>" << endl;
            cout << "<Function> -> function <Identifier>" << endl;   
            Identifier(Value::UNDEFINED);
            token = lexer(true);
            if(token.type == TokenType::SEPARATOR && token.value == "("){
                cout << "( <Opt Parameter List>" << endl;   
                Opt_Parameter_List();
                token = lexer(true);
                if(token.type == TokenType::SEPARATOR && token.value == ")"){
                    cout << ") <Opt Declaration List>" << endl;
                    Opt_Declaration_List();
                    cout << "<Body>" << endl;
                    Body();
                    cout << "End of Function" << endl;
                } else {
                    cout << "Error: Expected ')' at the end of Function";
                }
            } else {
                cout << "Error: Expected '(' at the start of Function";
            }

        } else {
            cout << "Error: Expected 'function' at the start of Function";
        }
    }

    void Opt_Parameter_List(){
        // Parameter_List() | Empty()
        Token token = lexer();
        currentIndex--;
        if(token.type != TokenType::SEPARATOR && (token.value != ")" || token.value != "$$")){
            cout << "<Opt Parameter List> -> <Parameter List>" << endl;
            Parameter_List();
        }
        else{
            cout << "<Opt Parameter List> -> <Empty>" << endl;
        }
    }

    void Parameter_List(){
        //<Parameter> <P>
        cout << "<Parameter List> -> <Parameter> <P>" << endl;
        Parameter();
        P();
    }

    void P(){
        // (ε |  , <Parameter List>)
        Token token = lexer();
        currentIndex--;
        if(token.type == TokenType::SEPARATOR && token.value == ","){
            Token token = lexer(true);
            cout << "<P> -> , <Parameter List>" << endl;
            Parameter_List();
        }
        else{
            cout << "<P> -> ε" << endl;
        }
    }

    void Parameter(){
        //<IDs> <Qualifier>
        cout << "<Parameter> -> <IDs> <Qualifier>" << endl;
        IDS();
        Qualifier();
    }

    Value Qualifier(){
        // integer | boolean | real
        Token token = lexer(true);
        if(token.type == TokenType::KEYWORD && token.value == "integer"){
            cout << "<Qualifier> -> integer | boolean | real" << endl;
            return Value(Value::INTEGER);
        }
        else if(token.type == TokenType::KEYWORD && token.value == "boolean"){
            cout << "<Qualifier> -> integer | boolean | real" << endl;
            return Value(Value::BOOLEAN);
        }
        else if(token.type == TokenType::KEYWORD && token.value == "real"){
            cout << "<Qualifier> -> integer | boolean | real" << endl;
            return Value(Value::REAL);
        }
        else {
            cout << "Error: Invalid Qualifier. Expected token type of integer, boolean, or real";
            return "";
        }
    }

    void Body(){
        // { < Statement List> }
        Token token = lexer(true);
        if (token.type == TokenType::SEPARATOR && token.value == "{") {
            cout << "<Body> -> { <Statement List> }" << endl;
            cout << "<Body> -> { <Statement List>" << endl;
            Statement_List();
            Token token = lexer(true);
            if (token.type == TokenType::SEPARATOR && token.value == "}") {
                cout << "}" << endl;
                cout << "End of Body" << endl;
            } else {
                cout << "Error: Expected '}' at the end of Body";
            }
        } else {
            cout << "Error: Expected '{' at the start of Body";
        }
    }

    void Opt_Declaration_List(){
        // Declaration_List() | Empty()
        Token token = lexer();
        currentIndex--;
        if(token.type == TokenType::KEYWORD && (token.value == "integer" || token.value == "real" || token.value == "boolean")){
            cout << "<Opt Declaration List> -> <Declaration List>" << endl;
            Declaration_List();
        }
        else{
            cout << "<Opt Declaration List> -> <Empty>" << endl;
        }
    }

    void Declaration_List(){
        // <Declaration> ; <D>
        cout << "<Declaration List> -> <Declaration> ; <D>" << endl;
        Declaration();
        Token token = lexer(true);
        if(token.type == TokenType::SEPARATOR && token.value == ";"){
            cout << "; <D>" << endl;
            D();
        } else {
            cout << "Error: Expected ';' at the end of Declaration_List";
        }
    }

    void D(){
        // (ε | <Declaration List>)
        Token token = lexer();
        currentIndex--;
        if(token.type != TokenType::SEPARATOR && (token.value != "{" || token.value != "$$")){
            cout << "<D> -> <Declaration List>" << endl;
            Declaration_List();
        }
        else{
            cout << "<D> -> ε" << endl;
        }
    }

    void Declaration(){
        // <Qualifier > <IDs>
        cout << "<Declaration> -> <Qualifier> <IDs>" << endl;
        Value value = Qualifier();
        IDS(value);
    }

    // Duplicate IDS, Identifier, id ===============================================
    void IDS(){
        Identifier();
        id();
    }

    void IDS(Value value){
        cout << "<IDs> -> <Identifier> <id>" << endl;
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
            cout << "<id> -> , <IDs>" << endl;
            IDS();
        }
        else{
            cout << "<id> -> ε" << endl;
        }
    }

    void id(Value value){
        //  (ε | , <IDs>)
        Token token = lexer();
        currentIndex--;
        if(token.type == TokenType::SEPARATOR && token.value == ","){
            Token token = lexer(true);
            cout << "<id> -> , <IDs>" << endl;
            IDS(value);
        }
        else{
            cout << "<id> -> ε" << endl;
        }
    }

    void Identifier(){
        // <Identifier> ::= <IDENTIFIER>
        Token token = lexer(true);
        if(token.type == TokenType::IDENTIFIER) {
            cout << "<Identifier> -> Identifier" << endl;
        } else {
            cout << "Error: Invalid Identifier. Expected token type of IDENTIFIER";
        }
    }

    void Identifier(Value valueType){
        // <Identifier> ::= <IDENTIFIER>
        Token token = lexer(true);
        if(token.type == TokenType::IDENTIFIER) {
            cout << "<Identifier> -> Identifier" << endl;
            symbolAndAssembly.generate_symbol(token.value, valueType);
        } else {
            cout << "Error: Invalid Identifier. Expected token type of IDENTIFIER";
        }
    }

    // =============================================================================

    void Statement_List(){
        //<Statement><S>
        cout << "<Statement List> -> <Statement> <S>" << endl;
        Statement();
        S();
    }

    void S(){
        //  (ε | <Statement List>)
        Token token = lexer();
        currentIndex--;
        if(token.type != TokenType::SEPARATOR && (token.value != "}" || token.value != "$$")){
            cout << "<S> -> <Statement List>" << endl;
            Statement_List();
        }
        else{
            cout << "<S> -> ε" << endl;
        }
    }

    void Statement(){
        // <Compound> | <Assign> | <If> | <Return> | <Print> | <Scan> | <While>
        Token token = lexer(true);
        cout << "<Statement> -> ";

        // <Compound> ::= { <Statement List> }
        // <Assign> ::= <Identifier> = <Expression> ;
        // <If> ::= if ( <Condition> )  <Statement> <if> 
        // <Return> ::= return <r>
        // <Print> ::= print ( <Expression> ) ;
        // <Scan> ::= scan ( <IDs> ) ;
        // <While> ::= while ( <Condition> ) <Statement> endwhile
        if(token.type == TokenType::SEPARATOR && token.value == "{"){
            cout << "<Compound>" << endl;
            cout << "<Compound> -> { <Statement List> }" << endl;
            cout << "<Compound> -> { <Statement List>" << endl;
            Statement_List();
            token = lexer(true);
            if(token.type == TokenType::SEPARATOR && token.value == "}"){
                cout << "}" << endl;
                cout << "End of Compound" << endl;
            }
            else{
                cout << "Error in beggining '}' for <Compound>'";
            }

        }
        else if(token.type == TokenType::KEYWORD && token.value == "if"){
            cout << "<If>" << endl;
            cout << "<If> -> if ( <Condition> ) <Statement> <if>" << endl;
            cout << "<If> -> if" << endl;
            token = lexer(true);
            if(token.type == TokenType::SEPARATOR && token.value == "("){
                cout << "( <Condition>" << endl;
                Condition();
                token = lexer(true);
                if(token.type == TokenType::SEPARATOR && token.value == ")"){
                    cout << ") <Statement> <if>" << endl;
                    Statement();
                    _if();
                }
                else{
                    cout << "Error in beginning ')' for <If>";
                }
            }
            else{
                cout << "Error in beginning '(' for <If>";
            }
        }
        else if(token.type == TokenType::KEYWORD && token.value == "return"){
            cout << "<Return>" << endl;
            cout << "<Return> -> return <r>" << endl;
            cout << "<Return> -> return" << endl;
            r();
        }
        else if(token.type == TokenType::KEYWORD && token.value == "print"){
            cout << "<Print>" << endl;
            cout << "<Print> -> print ( <Expression> )" << endl;
            cout << "<Print> -> print" << endl;
            token = lexer(true);
            if(token.type == TokenType::SEPARATOR && token.value == "("){
                cout << "( <Expression>" << endl;
                Expression();
                symbolAndAssembly.SOUT();
                token = lexer(true);
                if(token.type == TokenType::SEPARATOR && token.value == ")"){
                    cout << ")" << endl;
                    token = lexer(true);
                    if(token.type == TokenType::SEPARATOR && token.value == ";"){
                        cout << ";" << endl;
                        cout << "End of Print" << endl;
                    }
                    else{
                        cout << "Error in ';' for <Scan>";
                    }
                }
                else{
                    cout << "Error in beginning ')' for <Print>";
                }
            }
            else{
                cout << "Error in beginning '(' for <Print>";
            }
        }
        else if(token.type == TokenType::KEYWORD && token.value == "scan"){
            cout << "<Scan>" << endl;
            cout << "<Scan> -> scan ( <IDs> );" << endl;
            cout << "<Scan> -> scan" << endl;
            token = lexer(true);

            if(token.type == TokenType::SEPARATOR && token.value == "("){
                cout << "( <IDs>" << endl;
                IDS();
                token = lexer(true);

                if(token.type == TokenType::IDENTIFIER){
                    string var = token.value; // Save the variable
                    if(!symbolAndAssembly.getAddress(var)){
                        cout << "Error: Variable " << var << " not found in symbol table." << endl;
                    }
                    else{
                        symbolAndAssembly.SIN(var);
                    }
                    token = lexer(true);

                }
                else{
                    cout << "Error in <IDs> for <Scan>";
                }

                if(token.type == TokenType::SEPARATOR && token.value == ")"){
                    cout << ")" << endl;
                    token = lexer(true);
                    if(token.type == TokenType::SEPARATOR && token.value == ";"){
                        cout << ";" << endl;
                        cout << "End of Scan" << endl;
                    }
                    else{
                        cout << "Error in ';' for <Scan>";
                    }
                }
                else{
                    cout << "Error in beginning ')' for <Scan>";
                }
            }
            else{
                cout << "Error in beginning '(' for <Scan>";
            }
        }
    
        else if(token.type == TokenType::KEYWORD && token.value == "while"){
            cout << "<While>" << endl;
            cout << "<While> -> while ( <Condition> ) <Statement> endwhile" << endl;
            cout << "<While> -> while" << endl;
            int instruction_Addr = symbolAndAssembly.getInstructionAddr();
            symbolAndAssembly.generate_instruction("LABEL");
            token = lexer(true);
            if(token.type == TokenType::SEPARATOR && token.value == "("){
                cout << "( <Condition>" << endl;
                Condition();
                token = lexer(true);
                if(token.type == TokenType::SEPARATOR && token.value == ")"){
                    cout << ") <Statement>" << endl;
                    Statement();
                    token = lexer(true);

                    symbolAndAssembly.JMP(instruction_Addr);
                    symbolAndAssembly.back_patch(symbolAndAssembly.getInstructionAddr());
                    symbolAndAssembly.generate_instruction("LABEL");
                    
                    if(token.type == TokenType::KEYWORD && token.value == "endwhile"){
                    cout << "endwhile" << endl;
                    }
                    else{
                    cout << "Error in 'endwhile' for <While>";
                    }
                }
                else{
                    cout << "Error in beginning ')' for <While>";
                }
            }
            else{
                cout << "Error in beginning '(' for <While>";
            }
        }
        else if(token.type == TokenType::IDENTIFIER){
            cout << "<Assign>" << endl;
            cout << "<Assign> -> <Identifier> = <Expression> ;" << endl;
            cout << "<Assign> -> <Identifier>" << endl;
            string var = token.value; // Save the variable
            Token token = lexer(true);
                if(token.type == TokenType::OPERATOR && token.value == "="){
                    cout << "= <Expression> ;" << endl;
                    Expression();
                    token = lexer(true);
                    int memoryLoc = symbolAndAssembly.getAddress(var);
                    symbolAndAssembly.POPM(memoryLoc, var);
                    if(token.type == TokenType::SEPARATOR && token.value == ";"){
                        cout << ";" << endl;
                        cout << "End of Assign" << endl;
                    }
                    else{
                        cout << "Error in ';' for <Assign>";
                    }
                }
                else{
                    cout << "Error in '=' for <Assign>";
                }
        }
        else{
            cout << "Error: Invalid Statement. Expected statement type of <Compound>, <Assign>, <If>, <Return>, <Print>, <Scan>, or <While>";
        }

    }

    void _if(){
        //endif | else <Statement> endif
        Token token = lexer(true);
        if(token.type == TokenType::KEYWORD && token.value == "endif"){
            symbolAndAssembly.back_patch(symbolAndAssembly.getInstructionAddr());
            symbolAndAssembly.generate_instruction("LABEL");
            cout << "<if> -> endif" << endl;
        }
        else if(token.type == TokenType::KEYWORD && token.value == "else"){
            cout << "<if> -> else <Statement> endif" << endl;
            Statement();
            token = lexer(true);
            if(token.type == TokenType::KEYWORD && token.value == "endif"){
                symbolAndAssembly.back_patch(symbolAndAssembly.getInstructionAddr());
                symbolAndAssembly.generate_instruction("LABEL");
                cout << "endif" << endl;
                cout << "End of <If>" << endl;
            }
            else{
                cout << "Error in 'endif' for <if>";
            }
        }
        else{
            cout << "Error, expected 'endif' or 'else' for <if>";
        }

    }

    void r(){
        //  (; | <Expression> ;)
        Token token = lexer();
        currentIndex--;
        if(token.type == TokenType::SEPARATOR && token.value == ";"){
            token = lexer(true);
            cout << "<r> -> ;" << endl;
        }
        else{
            cout << "<r> -> <Expression> ;" << endl;
            Expression();
            token = lexer(true);
            if(token.type == TokenType::SEPARATOR && token.value == ";"){
                cout << ";" << endl;
                cout << "End of <Return>" << endl;
            }
            else{
                cout << "Error in ';' for <Return>";
            }
            
        }

    }

    void Condition(){
        //<Expression> <Relop> <Expression>
        cout << "<Condition> -> <Expression> <Relop> <Expression>" << endl;
        Expression();
        Relop();
    }

    void Relop(){
        // == | != | > | < | <= | =>
        Token token = lexer(true);
        if(token.type == TokenType::OPERATOR && 
            (token.value == "==" || token.value == "!=" || token.value == ">" || token.value == "<" || token.value == "<=" || token.value == "=>")){
            cout << "<Relop> -> == | != | > | < | <= | =>" << endl;
            
            Expression();
            if(token.value == ">"){
                symbolAndAssembly.GRT();
                symbolAndAssembly.generate_instruction("GRT");
                symbolAndAssembly.push_JMPstack(symbolAndAssembly.getInstructionAddr() - 1);
                symbolAndAssembly.JMP0();
            }
            else if(token.value == "<"){
                symbolAndAssembly.LES();
                symbolAndAssembly.generate_instruction("LES");
                symbolAndAssembly.push_JMPstack(symbolAndAssembly.getInstructionAddr() - 1);
                symbolAndAssembly.JMP0();
            }
            else if(token.value == "=="){
                symbolAndAssembly.EQU();
                symbolAndAssembly.generate_instruction("EQU");
                symbolAndAssembly.push_JMPstack(symbolAndAssembly.getInstructionAddr() - 1);
                symbolAndAssembly.JMP0();
            }
            else if(token.value == "!="){
                symbolAndAssembly.NEQ();
                symbolAndAssembly.generate_instruction("NEQ");
                symbolAndAssembly.push_JMPstack(symbolAndAssembly.getInstructionAddr() - 1);
                symbolAndAssembly.JMP0();
            }
            else if(token.value == "=>"){
                symbolAndAssembly.GEQ();
                symbolAndAssembly.generate_instruction("GEQ");
                symbolAndAssembly.push_JMPstack(symbolAndAssembly.getInstructionAddr() - 1);
                symbolAndAssembly.JMP0();
            }
            else if(token.value == "<="){
                symbolAndAssembly.LEQ();
                symbolAndAssembly.generate_instruction("LEQ");
                symbolAndAssembly.push_JMPstack(symbolAndAssembly.getInstructionAddr() - 1);
                symbolAndAssembly.JMP0();
            }
        }
        else{
            cout << "Error in Relop. Expected token type of OPERATOR with value ==, !=, >, <, <=, or =>";
        }
    }

    void Expression(){
        //<Term> <E>
        cout << "<Expression> -> <Term> <E>" << endl;
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
            cout << "<E> -> + <Term> <E> | - <Term><E>" << endl;
            Term();
            if(operator_addition_subtraction == "+"){
                symbolAndAssembly.A();
                symbolAndAssembly.generate_instruction("A");
            }
            else if(operator_addition_subtraction == "-"){
                symbolAndAssembly.S();
                symbolAndAssembly.generate_instruction("S");
            }
            E();
        }
        else{
            cout << "<E> -> ε" << endl;
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
            cout << "<T> -> * <Factor> <T> | / <Factor> <T>" << endl;
            Factor();

            if(var == "*"){
                symbolAndAssembly.M();
                symbolAndAssembly.generate_instruction("M");
            }
            else if(var == "/"){
                symbolAndAssembly.D();
                symbolAndAssembly.generate_instruction("D");
            }

            T();
        } 
        else{
            cout << "<T> -> ε" << endl;
        }
    }

    void Term(){
        // <Factor> <T>
        cout << "<Term> -> <Factor> <T>" << endl;
        Factor();
        T();
    }
    
    void Factor() {
        // - <Primary> | <Primary>
        Token token = lexer();
        currentIndex--;
        if(token.type == TokenType::OPERATOR && token.value == "-"){
            token = lexer(true);
            cout << "<Factor> -> - <Primary>" << endl;
            Primary();
        } else {
            cout << "<Factor> -> <Primary>" << endl;
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
                 cout << "<Identifier> ( <IDs> ) ->"; 
                 cout << " <Identifier> (" << endl;
                 IDS();
 
                 token = lexer(true);
                 if(token.type == TokenType::SEPARATOR && token.value == ")"){
                     cout << "<Identifier> ( <IDs> )" << endl;
                 }
                 else{
                     cout << "Error in Primary. Expected token type of ) for <Identifier> ( <IDs> )" << endl;
                 }
             }
            else{
                symbolAndAssembly.PUSHM(oldToken.value);
                symbolAndAssembly.generate_instruction("PUSHM", symbolAndAssembly.getAddress(oldToken.value));
                cout << "<Primary> -> <Identifier> | <Integer> | <Identifier> | true, false" << endl;
            }
         }
         else if (token.type == TokenType::INTEGER || token.type == TokenType::REAL) {
            //ADD IN PUSHM for the identifier, PUSHI for the random Integers, CHANGE
            symbolAndAssembly.PUSHI(stoi(token.value));
            symbolAndAssembly.generate_instruction("PUSHI");
            cout << "<Primary> -> <Identifier> | <Integer> | <Identifier> | true, false" << endl;
        } 
        else if(token.type == TokenType::KEYWORD && (token.value == "true" || token.value == "false")){
            symbolAndAssembly.PUSHB( token.value == "true" ? true : false );
            symbolAndAssembly.generate_instruction("PUSHB");
            cout << "<Primary> -> <Identifier> | <Integer> | <Identifier> | true, false" << endl;
        }
        else if (token.type == TokenType::SEPARATOR && token.value == "("){
             cout << "( <Expression> )" << endl;
             cout << "( <Expression> ) -> (" << endl;
             Expression();
             token = lexer(true);
             if(token.type == TokenType::SEPARATOR && token.value == ")"){
                 cout << "( <Expression> )" << endl;
             }
             else{
                 cout << "Error in Primary. Expected token type of ) for <Identifier> ( <IDs> )";
             }
         }
         else{
             cout << "Error in Primary. <Identifier> | <Integer> | <Identifier> ( <IDs> ) | ( <Expression> ) | <Real> | true | false";
         }
     }

};

#endif
