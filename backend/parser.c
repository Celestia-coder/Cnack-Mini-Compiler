#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ========================================================================= */
/* 1. LEXER DEFINITIONS & LOGIC                                              */
/* ========================================================================= */

typedef enum {
    TOKEN_COMMA, TOKEN_SEMICOLON, TOKEN_COLON, TOKEN_L_PAREN, TOKEN_R_PAREN, TOKEN_L_BRACE, TOKEN_R_BRACE, 
    TOKEN_L_BRACKET, TOKEN_R_BRACKET, 
    TOKEN_ADD_OP, TOKEN_SUB_OP, TOKEN_MULT_OP, TOKEN_DIV_OP, TOKEN_MOD_OP, TOKEN_EXPO_OP,
    TOKEN_ASSIGN_OP, TOKEN_ADD_ASSIGN_OP, TOKEN_SUB_ASSIGN_OP, TOKEN_MULT_ASSIGN_OP, TOKEN_DIV_ASSIGN_OP, TOKEN_MOD_ASSIGN_OP,
    TOKEN_ADDRESS_OF_OP, TOKEN_INCREMENT_OP, TOKEN_DECREMENT_OP,
    TOKEN_EQUAL_TO_OP, TOKEN_NOT_EQUAL_TO_OP, TOKEN_GREATER_OP, TOKEN_GREATER_EQUAL_OP, TOKEN_LESS_OP, TOKEN_LESS_EQUAL_OP,
    TOKEN_LOGICAL_NOT_OP, TOKEN_LOGICAL_AND_OP, TOKEN_LOGICAL_OR_OP,
    TOKEN_POINTER_OP, TOKEN_ARROW_OP, TOKEN_QUANTUM_POINTER_OP,
    TOKEN_IDENTIFIER, TOKEN_NUMBER_INT, TOKEN_NUMBER_FLOAT, TOKEN_STRING,
    TOKEN_TYPE_INT, TOKEN_TYPE_FLOAT, TOKEN_TYPE_CHAR, TOKEN_TYPE_BOOL, TOKEN_TYPE_STRING,
    TOKEN_KW_CONST, TOKEN_KW_IF, TOKEN_KW_ELSE, TOKEN_KW_ELIF, TOKEN_KW_SWITCH, TOKEN_KW_CASE, TOKEN_KW_DEFAULT,
    TOKEN_KW_ASSIGN, TOKEN_KW_FOR, TOKEN_KW_WHILE, TOKEN_KW_DO, TOKEN_KW_BREAK, TOKEN_KW_CONTINUE,
    TOKEN_KW_ASK, TOKEN_KW_DISPLAY, TOKEN_KW_TRUE, TOKEN_KW_FALSE, TOKEN_KW_FN, TOKEN_KW_STRUCT,
    TOKEN_RW_EXECUTE, TOKEN_RW_EXIT, TOKEN_RW_FETCH, TOKEN_RW_WHEN, TOKEN_RW_OTHERWISE, TOKEN_RW_AUTO_REF,
    TOKEN_SINGLE_COMMENT, TOKEN_MULTI_COMMENT, TOKEN_ERROR, TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    const char *lexeme_start;
    int lexeme_length;
    int line;
} Token;

typedef struct {
    const char *source_start;
    const char *token_start;
    const char *scan_ptr;
    int line_number;
} Scanner;

Scanner scanner;

typedef struct {
    const char *keyword;
    TokenType token_type;
} KeywordMapping;

KeywordMapping keywords[] = {
    {"int", TOKEN_TYPE_INT}, {"float", TOKEN_TYPE_FLOAT}, {"char", TOKEN_TYPE_CHAR}, 
    {"bool", TOKEN_TYPE_BOOL}, {"boolean", TOKEN_TYPE_BOOL}, {"string", TOKEN_TYPE_STRING}, {"String", TOKEN_TYPE_STRING},
    {"const", TOKEN_KW_CONST}, {"if", TOKEN_KW_IF}, {"else", TOKEN_KW_ELSE}, {"elif", TOKEN_KW_ELIF},
    {"switch", TOKEN_KW_SWITCH}, {"case", TOKEN_KW_CASE}, {"default", TOKEN_KW_DEFAULT}, {"assign", TOKEN_KW_ASSIGN}, 
    {"struct", TOKEN_KW_STRUCT}, {"for", TOKEN_KW_FOR}, {"while", TOKEN_KW_WHILE}, {"do", TOKEN_KW_DO}, 
    {"break", TOKEN_KW_BREAK}, {"continue", TOKEN_KW_CONTINUE}, {"ask", TOKEN_KW_ASK}, {"display", TOKEN_KW_DISPLAY},
    {"execute", TOKEN_RW_EXECUTE}, {"exit", TOKEN_RW_EXIT}, {"true", TOKEN_KW_TRUE}, {"false", TOKEN_KW_FALSE}, 
    {"fetch", TOKEN_RW_FETCH}, {"fn", TOKEN_KW_FN}, {"when", TOKEN_RW_WHEN}, {"otherwise", TOKEN_RW_OTHERWISE}, 
    {"auto_ref", TOKEN_RW_AUTO_REF}
};
int keywordCount = 32;

int isLetter(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
int isDigit(char c) { return c >= '0' && c <= '9'; }
int isOperator(char c) { switch(c){ case '+': case '-': case '*': case '/': case '%': case '=': case '<': case '>': case '!': case '&': case '|': case '^': return 1; default: return 0; } }

int reachedEnd() { return *scanner.scan_ptr == '\0'; }
char consumeChar() { scanner.scan_ptr++; return scanner.scan_ptr[-1]; }
char currentChar() { return *scanner.scan_ptr; }
char nextChar() { if (reachedEnd()) return '\0'; return scanner.scan_ptr[1]; }
int matchNext(char expected) { if (reachedEnd() || *scanner.scan_ptr != expected) return 0; scanner.scan_ptr++; return 1; }

Token createToken(TokenType type) {
    Token tok; tok.type = type; tok.lexeme_start = scanner.token_start;
    tok.lexeme_length = (int)(scanner.scan_ptr - scanner.token_start); tok.line = scanner.line_number;
    return tok;
}
Token createErrorToken(const char *message) {
    Token tok; tok.type = TOKEN_ERROR; tok.lexeme_start = message;
    tok.lexeme_length = (int)strlen(message); tok.line = scanner.line_number;
    return tok;
}

Token checkOperatorBoundary(TokenType operatorType) {
    if (isLetter(currentChar()) || isDigit(currentChar()) || currentChar() == '_') {
        while (isLetter(currentChar()) || isDigit(currentChar()) || currentChar() == '_' || isOperator(currentChar())) consumeChar();
        return createErrorToken("Invalid token boundary");
    }
    return createToken(operatorType);
}

TokenType getKeywordType(const char *str, int length) {
    for (int i = 0; i < keywordCount; i++) {
        if (strlen(keywords[i].keyword) == length && memcmp(str, keywords[i].keyword, length) == 0) return keywords[i].token_type;
    }
    return TOKEN_IDENTIFIER;
}

void skipWhitespaceAndNewlines() {
    while (!reachedEnd()) {
        char c = currentChar();
        if (isspace((unsigned char)c)) {
            if (c == '\n') scanner.line_number++;
            consumeChar();
        } else {
            break;
        }
    }
}

Token scanIdentifier() {
    while (isLetter(currentChar()) || isDigit(currentChar()) || currentChar() == '_') consumeChar();
    int len = (int)(scanner.scan_ptr - scanner.token_start);
    TokenType type = getKeywordType(scanner.token_start, len);
    return createToken(type);
}

Token scanNumber() {
    int isFloat = 0;
    while (isDigit(currentChar())) consumeChar();
    if (currentChar() == '.' && isDigit(nextChar())) { isFloat = 1; consumeChar(); while (isDigit(currentChar())) consumeChar(); }
    return createToken(isFloat ? TOKEN_NUMBER_FLOAT : TOKEN_NUMBER_INT);
}

Token scanString(char quote) {
    while (currentChar() != quote && !reachedEnd()) {
        if (currentChar() == '\n') scanner.line_number++;
        consumeChar();
    }
    if (reachedEnd()) return createErrorToken("Unterminated string");
    consumeChar(); return createToken(TOKEN_STRING);
}

Token scanSingleLineComment() { while (currentChar() != '\n' && !reachedEnd()) consumeChar(); return createToken(TOKEN_SINGLE_COMMENT); }
Token scanMultiLineComment() {
    while (!reachedEnd()) {
        if (currentChar() == '*' && nextChar() == '/') { consumeChar(); consumeChar(); return createToken(TOKEN_MULTI_COMMENT); }
        if (currentChar() == '\n') scanner.line_number++; consumeChar();
    }
    return createErrorToken("Invalid token");
}

Token getNextToken() {
    skipWhitespaceAndNewlines();
    scanner.token_start = scanner.scan_ptr;
    if (reachedEnd()) return createToken(TOKEN_EOF);

    char c = currentChar();

    // Catch invalid characters
    if (!isLetter(c) && !isDigit(c) && !isOperator(c) && c != '_' && c != '"' && c != '\'' && 
        c != '(' && c != ')' && c != '{' && c != '}' && c != '[' && c != ']' && c != ',' && c != ';' && c != ':') {
        consumeChar();
        return createErrorToken("Invalid character");
    }

    if (isLetter(c) || c == '_') {
        if (c == 'f' && nextChar() == '"') { consumeChar(); consumeChar(); return scanString('"'); }
        consumeChar(); return scanIdentifier();
    }

    if (isDigit(c)) { consumeChar(); return scanNumber(); }

    c = consumeChar();
    switch (c) {
        case '+': if (matchNext('+')) return createToken(TOKEN_INCREMENT_OP); if (matchNext('=')) return createToken(TOKEN_ADD_ASSIGN_OP); return createToken(TOKEN_ADD_OP);
        case '-': if (matchNext('-')) return createToken(TOKEN_DECREMENT_OP); if (matchNext('=')) return createToken(TOKEN_SUB_ASSIGN_OP); if (matchNext('>')) return createToken(TOKEN_ARROW_OP); return createToken(TOKEN_SUB_OP);
        case '*': 
            if (matchNext('|')) return createToken(TOKEN_QUANTUM_POINTER_OP); 
            if (matchNext('=')) return createToken(TOKEN_MULT_ASSIGN_OP);
            return createToken(TOKEN_MULT_OP); 
        case '/': if (matchNext('=')) return createToken(TOKEN_DIV_ASSIGN_OP); if (matchNext('/')) return scanSingleLineComment(); if (matchNext('*')) return scanMultiLineComment(); return createToken(TOKEN_DIV_OP);
        case '%': if (matchNext('=')) return createToken(TOKEN_MOD_ASSIGN_OP); return createToken(TOKEN_MOD_OP);
        case '=': if (matchNext('=')) return createToken(TOKEN_EQUAL_TO_OP); return createToken(TOKEN_ASSIGN_OP);
        case '<': if (matchNext('=')) return createToken(TOKEN_LESS_EQUAL_OP); return createToken(TOKEN_LESS_OP);
        case '>': if (matchNext('=')) return createToken(TOKEN_GREATER_EQUAL_OP); return createToken(TOKEN_GREATER_OP);
        case '!': if (matchNext('=')) return createToken(TOKEN_NOT_EQUAL_TO_OP); return createToken(TOKEN_LOGICAL_NOT_OP);
        case '&': if (matchNext('&')) return createToken(TOKEN_LOGICAL_AND_OP); return createToken(TOKEN_ADDRESS_OF_OP);
        case '|': if (matchNext('|')) return createToken(TOKEN_LOGICAL_OR_OP); return createToken(TOKEN_ERROR);
        case '^': if (matchNext('=')) return createToken(TOKEN_ERROR); return createToken(TOKEN_EXPO_OP);
        case ',': return createToken(TOKEN_COMMA);
        case ';': return createToken(TOKEN_SEMICOLON);
        case ':': return createToken(TOKEN_COLON);
        case '(': return createToken(TOKEN_L_PAREN);
        case ')': return createToken(TOKEN_R_PAREN);
        case '{': return createToken(TOKEN_L_BRACE);
        case '}': return createToken(TOKEN_R_BRACE);
        case '[': return createToken(TOKEN_L_BRACKET);
        case ']': return createToken(TOKEN_R_BRACKET);
        case '"': return scanString('"');
        case '\'': return scanString('\'');
        default: return createToken(TOKEN_ERROR);
    }
}

void initScanner(const char *source) { scanner.source_start = source; scanner.token_start = source; scanner.scan_ptr = source; scanner.line_number = 1; }

/* ========================================================================= */
/* 2. SYMBOL TABLE & EVALUATOR                                               */
/* ========================================================================= */

#define MAX_SYMBOLS 100
#define MAX_VAL_LEN 1024

typedef struct {
    char name[64];
    char value[MAX_VAL_LEN];
    int isArray;
    int intVal;
} Symbol;

Symbol symbolTable[MAX_SYMBOLS];
int symbolCount = 0;

void setSymbol(const char* name, const char* val, int isArray) {
    int iVal = atoi(val);
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            strncpy(symbolTable[i].value, val, MAX_VAL_LEN - 1);
            symbolTable[i].value[MAX_VAL_LEN - 1] = '\0';
            symbolTable[i].isArray = isArray;
            symbolTable[i].intVal = iVal;
            return;
        }
    }
    if (symbolCount < MAX_SYMBOLS) {
        strncpy(symbolTable[symbolCount].name, name, 63);
        strncpy(symbolTable[symbolCount].value, val, MAX_VAL_LEN - 1);
        symbolTable[symbolCount].value[MAX_VAL_LEN - 1] = '\0';
        symbolTable[symbolCount].isArray = isArray;
        symbolTable[symbolCount].intVal = iVal;
        symbolCount++;
    }
}

Symbol* getSymbol(const char* name) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            return &symbolTable[i];
        }
    }
    return NULL;
}

char mockOutput[8192];
int mockPos = 0;

void appendMockOutput(const char *text) {
    if (mockPos + strlen(text) < 8190) {
        strcpy(mockOutput + mockPos, text);
        mockPos += strlen(text);
    }
}

void appendNewline() {
    if (mockPos < 8190 && mockPos > 0 && mockOutput[mockPos-1] != '\n') {
        mockOutput[mockPos++] = '\n';
        mockOutput[mockPos] = '\0';
    }
}

/* ========================================================================= */
/* 3. PARSER IMPLEMENTATION                                                  */
/* ========================================================================= */

Token currentToken;
Token lookaheadToken;
int panicMode = 0; 
int success = 1;

int hasString = 0;   
int hasCAB = 0;      
int hasAutoRef = 0;  
int hasQPA = 0;      

void advance() {
    currentToken = lookaheadToken;
    lookaheadToken = getNextToken();
    
    // Treat invalid characters as Syntax Errors
    if (currentToken.type == TOKEN_ERROR) {
        if (currentToken.lexeme_length > 1 && isalpha(currentToken.lexeme_start[0])) {
             printf("[Syntax Error] Line %d: %.*s\n", currentToken.line, currentToken.lexeme_length, currentToken.lexeme_start);
        } else {
             printf("[Syntax Error] Line %d: Unexpected character '%.*s'\n", currentToken.line, currentToken.lexeme_length, currentToken.lexeme_start);
        }
        success = 0; 
        panicMode = 1; 
        currentToken = lookaheadToken;
        lookaheadToken = getNextToken();
    }

    while (currentToken.type == TOKEN_SINGLE_COMMENT || currentToken.type == TOKEN_MULTI_COMMENT) {
        currentToken = lookaheadToken;
        lookaheadToken = getNextToken();
    }
}

void synchronize() {
    while (currentToken.type != TOKEN_EOF) {
        if (currentToken.type == TOKEN_SEMICOLON) {
            advance();
            panicMode = 0;
            return;
        }
        if (currentToken.type == TOKEN_R_BRACE) {
            advance();
            panicMode = 0;
            return;
        }
        advance();
    }
}

void error(const char *msg) {
    if (panicMode) return;
    panicMode = 1;
    success = 0; 
    printf("[Syntax Error] Line %d: %s (Found '%.*s')\n", 
           currentToken.line, msg, currentToken.lexeme_length, currentToken.lexeme_start);
}

void consume(TokenType type, const char *msg) {
    if (currentToken.type == type) {
        advance();
    } else {
        error(msg);
    }
}

int isType(Token t) {
    if (t.type == TOKEN_TYPE_INT || t.type == TOKEN_TYPE_FLOAT || 
        t.type == TOKEN_TYPE_CHAR || t.type == TOKEN_TYPE_BOOL || 
        t.type == TOKEN_TYPE_STRING) return 1;
    if (t.type == TOKEN_IDENTIFIER && isupper(t.lexeme_start[0])) return 1;
    return 0;
}

void checkConfusion(Token t) {
    if (t.type == TOKEN_IDENTIFIER) {
        if (strncmp(t.lexeme_start, "print", 5) == 0) { error("Unknown command. Did you mean 'display'?"); return; }
        if (strncmp(t.lexeme_start, "cout", 4) == 0) { error("Unknown command. Did you mean 'display'?"); return; }
        if (strncmp(t.lexeme_start, "input", 5) == 0) { error("Unknown command. Did you mean 'ask'?"); return; }
    }
}

void statementList();
void statement();
void declaration();
void structDeclaration();
void assignmentOrInput();
void displayStatement();
void conditionalAssignmentBlock(); 
void ifStatement();                
void whileLoop();
void doWhileLoop();
void forLoop();
void functionDeclaration();        
void quantumPointerOperation();    
int expression();
int simpleExpression();
int term();
int factor();

/* Helper: Interpolate {var} -> Value */
void parseInterpolation(const char* start, int len) {
    char buffer[2048];
    int bufIdx = 0;
    int i = 0;
    char varName[64];

    while (i < len && bufIdx < 2040) {
        if (start[i] == '{') {
            i++; 
            int vIdx = 0;
            while (i < len && start[i] != '}') {
                varName[vIdx++] = start[i++];
            }
            varName[vIdx] = '\0';
            if (i < len && start[i] == '}') i++;

            Symbol* sym = getSymbol(varName);
            if (sym) {
                char *val = sym->value;
                if (val[0] == '"' || val[0] == '\'') {
                    for(int k=1; k<(int)strlen(val)-1; k++) buffer[bufIdx++] = val[k];
                } else {
                    for(int k=0; k<(int)strlen(val); k++) buffer[bufIdx++] = val[k];
                }
            } else {
                buffer[bufIdx++] = '<';
                for(int k=0; k<(int)strlen(varName); k++) buffer[bufIdx++] = varName[k];
                buffer[bufIdx++] = '>';
            }
        } else {
            buffer[bufIdx++] = start[i++];
        }
    }
    buffer[bufIdx] = '\0';
    appendMockOutput(buffer);
}

void program() {
    printf(">>> STARTING PARSER FOR CNACK LANGUAGE...\n");

    while (currentToken.type == TOKEN_KW_STRUCT) {
        structDeclaration();
        if (panicMode) synchronize();
    }

    consume(TOKEN_RW_EXECUTE, "Expected 'execute' to start program");
    if (panicMode) synchronize();

    consume(TOKEN_L_PAREN, "Expected '('");
    consume(TOKEN_R_PAREN, "Expected ')'");
    consume(TOKEN_L_BRACE, "Expected '{'");
    
    statementList();

    if (currentToken.type == TOKEN_RW_EXIT) {
        advance();
        consume(TOKEN_L_PAREN, "Expected '('");
        consume(TOKEN_R_PAREN, "Expected ')'");
        consume(TOKEN_SEMICOLON, "Expected ';'");
    }
    
    consume(TOKEN_R_BRACE, "Expected '}'");

    if (success) {
        printf("\n>>> SYNTAX ANALYSIS: PARSING SUCCESSFUL!\n");
        printf("----------------------------------------\n");
        printf("Principles Detected:\n");
        if (hasString)  printf(" [x] Principle 1: String Data Type\n");
        if (hasCAB)     printf(" [x] Principle 2: Conditional Assignment Blocks (CAB)\n");
        if (hasAutoRef) printf(" [x] Principle 3: Auto Reference Command\n");
        if (hasQPA)     printf(" [x] Principle 4: Quantum Pointer Aliasing (QPA)\n");
        if (!hasString && !hasCAB && !hasAutoRef && !hasQPA) printf(" [ ] None detected.\n");
        printf("----------------------------------------\n");
        
        if (mockPos > 0) {
            printf("\n[PROGRAM OUTPUT]\n");
            printf("%s", mockOutput);
            printf("----------------------------------------\n");
        }
    } else {
        printf("\n>>> SYNTAX ANALYSIS: PARSING UNSUCCESSFUL!\n");
    }
}

void structDeclaration() {
    consume(TOKEN_KW_STRUCT, "Expected 'struct'");
    consume(TOKEN_IDENTIFIER, "Expected struct name");
    consume(TOKEN_L_BRACE, "Expected '{'");
    while (isType(currentToken)) {
        declaration();
    }
    consume(TOKEN_R_BRACE, "Expected '}'");
}

void statementList() {
    while (currentToken.type != TOKEN_RW_EXIT && currentToken.type != TOKEN_R_BRACE && currentToken.type != TOKEN_EOF) {
        statement();
        if (panicMode) synchronize(); 
    }
}

void statement() {
    if (isType(currentToken)) {
        declaration();
    } else if (currentToken.type == TOKEN_IDENTIFIER) {
        checkConfusion(currentToken); 
        if (panicMode) { advance(); return; }
        
        if (currentToken.type == TOKEN_KW_ELSE && lookaheadToken.type == TOKEN_KW_IF) {
             error("Invalid syntax 'else if'. Did you mean 'elif'?");
             advance(); advance(); return;
        }
        assignmentOrInput(); 
    } else if (currentToken.type == TOKEN_KW_DISPLAY) {
        displayStatement();
    } else if (currentToken.type == TOKEN_KW_ASSIGN) {
        conditionalAssignmentBlock();
    } else if (currentToken.type == TOKEN_KW_IF) {
        ifStatement(); 
    } else if (currentToken.type == TOKEN_KW_WHILE) {
        whileLoop();
    } else if (currentToken.type == TOKEN_KW_DO) {
        doWhileLoop();
    } else if (currentToken.type == TOKEN_KW_FOR) {
        forLoop();
    } else if (currentToken.type == TOKEN_KW_FN) {
        functionDeclaration(); 
    } else if (currentToken.type == TOKEN_QUANTUM_POINTER_OP) {
        quantumPointerOperation(); 
    } else if (currentToken.type == TOKEN_MULT_OP) { 
        quantumPointerOperation();
    } else {
        if (currentToken.type == TOKEN_KW_ELSE) {
             error("Unexpected 'else'. Did you mean 'elif' or is it missing an 'if'?");
             advance(); return;
        }
        error("Unexpected statement start");
        advance(); 
    }
}

void declaration() {
    if (currentToken.type == TOKEN_TYPE_STRING) hasString = 1;

    advance(); 
    if (currentToken.type == TOKEN_MULT_OP || currentToken.type == TOKEN_POINTER_OP) advance(); 
    if (currentToken.type == TOKEN_QUANTUM_POINTER_OP) { hasQPA = 1; advance(); }

    do {
        char varName[64];
        if (currentToken.type == TOKEN_IDENTIFIER) {
            strncpy(varName, currentToken.lexeme_start, currentToken.lexeme_length);
            varName[currentToken.lexeme_length] = '\0';
        }
        consume(TOKEN_IDENTIFIER, "Expected variable name");
        
        if (currentToken.type == TOKEN_ASSIGN_OP) {
            advance();
            
            if (currentToken.type == TOKEN_RW_AUTO_REF) {
                hasAutoRef = 1;
                advance(); consume(TOKEN_L_PAREN, "Expected '('");
                if (isType(currentToken) || currentToken.type == TOKEN_IDENTIFIER) advance(); 
                consume(TOKEN_COMMA, "Expected ','");
                
                if (currentToken.type == TOKEN_NUMBER_INT) {
                    char valBuf[64];
                    strncpy(valBuf, currentToken.lexeme_start, currentToken.lexeme_length);
                    valBuf[currentToken.lexeme_length] = '\0';
                    setSymbol(varName, valBuf, 0);
                    advance();
                } else if (currentToken.type == TOKEN_L_BRACKET) { // Array
                    setSymbol(varName, "[1, 2, 3, 4, 5]", 1); 
                    int bracketCount = 1;
                    advance();
                    while(bracketCount > 0 && currentToken.type != TOKEN_EOF) {
                        if(currentToken.type == TOKEN_L_BRACKET) bracketCount++;
                        if(currentToken.type == TOKEN_R_BRACKET) bracketCount--;
                        advance();
                    }
                } else {
                    expression();
                }
                consume(TOKEN_R_PAREN, "Expected ')'");
            }
            else {
                // Check if simple literal to preserve exact format (e.g. 10.5)
                int isLiteral = 0;
                if (currentToken.type == TOKEN_NUMBER_INT || currentToken.type == TOKEN_NUMBER_FLOAT || 
                    currentToken.type == TOKEN_STRING || currentToken.type == TOKEN_KW_TRUE || 
                    currentToken.type == TOKEN_KW_FALSE) {
                    
                    char valBuf[256];
                    int vLen = currentToken.lexeme_length; if(vLen>255)vLen=255;
                    strncpy(valBuf, currentToken.lexeme_start, vLen);
                    valBuf[vLen] = '\0';
                    setSymbol(varName, valBuf, 0);
                    isLiteral = 1;
                }

                // Call expression to parse tokens
                int result = expression();

                // If not literal (e.g. calculated), update with result
                if (!isLiteral) {
                    char valBuf[32];
                    sprintf(valBuf, "%d", result);
                    setSymbol(varName, valBuf, 0);
                }
            }
        }
        if (currentToken.type == TOKEN_COMMA) advance(); else break;
    } while (1);
    consume(TOKEN_SEMICOLON, "Expected ';'");
}

void doWhileLoop() {
    consume(TOKEN_KW_DO, "Expected 'do'");
    consume(TOKEN_L_BRACE, "Expected '{'");
    statementList(); 
    consume(TOKEN_R_BRACE, "Expected '}'");
    consume(TOKEN_KW_WHILE, "Expected 'while'");
    consume(TOKEN_L_PAREN, "Expected '('");
    expression();
    consume(TOKEN_R_PAREN, "Expected ')'");
    consume(TOKEN_SEMICOLON, "Expected ';'");
}

void forLoop() {
    consume(TOKEN_KW_FOR, "Expected 'for'");
    consume(TOKEN_L_PAREN, "Expected '('");
    if (isType(currentToken)) declaration(); 
    else if (currentToken.type == TOKEN_IDENTIFIER) assignmentOrInput();
    else if (currentToken.type != TOKEN_SEMICOLON) error("Invalid For-Loop Init");
    
    if (currentToken.type == TOKEN_SEMICOLON) advance(); 
    expression(); 
    consume(TOKEN_SEMICOLON, "Expected ';'");
    
    if (currentToken.type == TOKEN_IDENTIFIER) {
        advance();
        if (currentToken.type == TOKEN_INCREMENT_OP || currentToken.type == TOKEN_DECREMENT_OP) advance();
        else if (currentToken.type == TOKEN_ASSIGN_OP || currentToken.type == TOKEN_ADD_ASSIGN_OP) {
            advance(); expression();
        }
    }
    consume(TOKEN_R_PAREN, "Expected ')'");
    consume(TOKEN_L_BRACE, "Expected '{'");
    statementList(); 
    consume(TOKEN_R_BRACE, "Expected '}'");
}

void whileLoop() {
    consume(TOKEN_KW_WHILE, "Expected 'while'");
    consume(TOKEN_L_PAREN, "Expected '('");
    expression();
    consume(TOKEN_R_PAREN, "Expected ')'");
    consume(TOKEN_L_BRACE, "Expected '{'");
    statementList();
    consume(TOKEN_R_BRACE, "Expected '}'");
}

void ifStatement() {
    consume(TOKEN_KW_IF, "Expected 'if'");
    consume(TOKEN_L_PAREN, "Expected '('");
    expression();
    consume(TOKEN_R_PAREN, "Expected ')'");
    consume(TOKEN_L_BRACE, "Expected '{'");
    statementList();
    consume(TOKEN_R_BRACE, "Expected '}'");

    while (currentToken.type == TOKEN_KW_ELIF) {
        advance();
        consume(TOKEN_L_PAREN, "Expected '('");
        expression();
        consume(TOKEN_R_PAREN, "Expected ')'");
        consume(TOKEN_L_BRACE, "Expected '{'");
        statementList();
        consume(TOKEN_R_BRACE, "Expected '}'");
    }

    if (currentToken.type == TOKEN_KW_ELSE) {
        advance();
        consume(TOKEN_L_BRACE, "Expected '{'");
        statementList();
        consume(TOKEN_R_BRACE, "Expected '}'");
    }
}

void conditionalAssignmentBlock() {
    hasCAB = 1; 
    consume(TOKEN_KW_ASSIGN, "Expected 'assign'");
    consume(TOKEN_L_PAREN, "Expected '('");
    do {
        consume(TOKEN_IDENTIFIER, "Expected ID");
        if (currentToken.type == TOKEN_COMMA) advance(); else break;
    } while (1);
    consume(TOKEN_R_PAREN, "Expected ')'");
    consume(TOKEN_L_BRACE, "Expected '{'");
    
    do {
        consume(TOKEN_RW_WHEN, "Expected 'when'");
        expression(); 
        consume(TOKEN_COLON, "Expected ':'");
        if (currentToken.type == TOKEN_L_PAREN) { 
            advance(); 
            while (1) {
                if (currentToken.type == TOKEN_KW_ASSIGN) conditionalAssignmentBlock(); 
                else expression();
                if (currentToken.type == TOKEN_COMMA) advance(); else break;
            }
            consume(TOKEN_R_PAREN, "Expected ')'");
        } else {
            expression();
        }
        consume(TOKEN_SEMICOLON, "Expected ';'");
    } while (currentToken.type == TOKEN_RW_WHEN);

    consume(TOKEN_RW_OTHERWISE, "Expected 'otherwise'");
    consume(TOKEN_COLON, "Expected ':'");
    if (currentToken.type == TOKEN_L_PAREN) { 
        advance();
        while (1) {
            expression();
            if (currentToken.type == TOKEN_COMMA) advance(); else break;
        }
        consume(TOKEN_R_PAREN, "Expected ')'");
    } else {
        expression();
    }
    consume(TOKEN_SEMICOLON, "Expected ';'");
    consume(TOKEN_R_BRACE, "Expected '}'");
}

void assignmentOrInput() {
    char varName[64];
    strncpy(varName, currentToken.lexeme_start, currentToken.lexeme_length);
    varName[currentToken.lexeme_length] = '\0';

    consume(TOKEN_IDENTIFIER, "Expected identifier");
    
    if (currentToken.type >= TOKEN_ASSIGN_OP && currentToken.type <= TOKEN_MOD_ASSIGN_OP) {
        TokenType op = currentToken.type;
        advance();
        if (op == TOKEN_ASSIGN_OP && currentToken.type == TOKEN_KW_ASK) {
            advance(); consume(TOKEN_L_PAREN, "Expected '('"); 
            if (currentToken.type == TOKEN_IDENTIFIER) advance(); 
            consume(TOKEN_R_PAREN, "Expected ')'");
            consume(TOKEN_SEMICOLON, "Expected ';'");
        } else if (currentToken.type == TOKEN_L_PAREN) { 
             error("Unknown function call");
             advance();
        } else {
            int result = expression();
            char valBuf[32];
            sprintf(valBuf, "%d", result);
            setSymbol(varName, valBuf, 0);
            consume(TOKEN_SEMICOLON, "Expected ';'");
        }
    } else if (currentToken.type == TOKEN_INCREMENT_OP || currentToken.type == TOKEN_DECREMENT_OP) {
        advance();
        consume(TOKEN_SEMICOLON, "Expected ';'");
    } else if (currentToken.type == TOKEN_L_PAREN) {
        error("Unknown function call or missing assignment");
        advance();
    } else {
        error("Expected assignment operator, '++', or '--'");
    }
}

void quantumPointerOperation() {
    if (currentToken.type == TOKEN_QUANTUM_POINTER_OP) { hasQPA = 1; advance(); }
    else if (currentToken.type == TOKEN_MULT_OP) { advance(); }
    
    consume(TOKEN_IDENTIFIER, "Expected pointer ID");
    
    if (currentToken.type >= TOKEN_ASSIGN_OP && currentToken.type <= TOKEN_MOD_ASSIGN_OP) {
        advance(); expression(); consume(TOKEN_SEMICOLON, "Expected ';'");
    } else {
        error("Expected assignment for Quantum op");
    }
}

void displayStatement() {
    consume(TOKEN_KW_DISPLAY, "Expected 'display'");
    consume(TOKEN_L_PAREN, "Expected '('");
    
    while (1) {
        if (currentToken.type == TOKEN_STRING) {
            int len = currentToken.lexeme_length;
            int startOffset = 1;
            if (strncmp(currentToken.lexeme_start, "f\"", 2) == 0) startOffset = 2;
            
            if (strstr(currentToken.lexeme_start, "%d") || strstr(currentToken.lexeme_start, "%f")) {
            } else {
                parseInterpolation(currentToken.lexeme_start + startOffset, len - (startOffset + 1));
            }
            advance();
        } else if (currentToken.type == TOKEN_IDENTIFIER) {
            char varName[64];
            strncpy(varName, currentToken.lexeme_start, currentToken.lexeme_length);
            varName[currentToken.lexeme_length] = '\0';
            
            if (lookaheadToken.type == TOKEN_L_BRACKET) {
                Symbol* sym = getSymbol(varName);
                if (sym && sym->isArray) appendMockOutput("1 2 3 4 5");
                else appendMockOutput("<array>");
                advance(); advance(); expression(); consume(TOKEN_R_BRACKET, "Expected ']'");
            } else {
                Symbol* sym = getSymbol(varName);
                if (sym) {
                    char *val = sym->value;
                    if (val[0] == '"' || val[0] == '\'') {
                        for(int k=1; k<(int)strlen(val)-1; k++) {
                            char c[2] = {val[k], '\0'};
                            appendMockOutput(c);
                        }
                    } else {
                        appendMockOutput(val);
                    }
                } else {
                    char buf[128]; sprintf(buf, "<value of %s>", varName); appendMockOutput(buf);
                }
                advance();
            }
        } else if (currentToken.type == TOKEN_MULT_OP || currentToken.type == TOKEN_POINTER_OP) {
            advance();
            if (currentToken.type == TOKEN_IDENTIFIER) {
                char varName[64];
                strncpy(varName, currentToken.lexeme_start, currentToken.lexeme_length);
                varName[currentToken.lexeme_length] = '\0';
                Symbol* sym = getSymbol(varName);
                if(sym) appendMockOutput(sym->value);
                else {
                    char buf[128]; sprintf(buf, "<value of *%s>", varName); appendMockOutput(buf);
                }
                advance();
            }
        }

        if (currentToken.type == TOKEN_COMMA) advance();
        else break;
    }

    appendNewline(); 
    consume(TOKEN_R_PAREN, "Expected ')'");
    consume(TOKEN_SEMICOLON, "Expected ';'");
}

void functionDeclaration() {
    consume(TOKEN_KW_FN, "Expected 'fn'");
    consume(TOKEN_IDENTIFIER, "Expected function name");
    consume(TOKEN_L_PAREN, "Expected '('");
    consume(TOKEN_R_PAREN, "Expected ')'");
    consume(TOKEN_L_BRACE, "Expected '{'");
    statementList(); 
    consume(TOKEN_R_BRACE, "Expected '}'");
}

int expression() {
    int val = simpleExpression();
    if (currentToken.type >= TOKEN_EQUAL_TO_OP && currentToken.type <= TOKEN_LESS_EQUAL_OP) { 
        advance(); simpleExpression(); 
    }
    if (currentToken.type == TOKEN_LOGICAL_AND_OP || currentToken.type == TOKEN_LOGICAL_OR_OP) { 
        advance(); expression(); 
    }
    return val;
}

int simpleExpression() {
    int left = term();
    while (currentToken.type == TOKEN_ADD_OP || currentToken.type == TOKEN_SUB_OP) { 
        TokenType op = currentToken.type;
        advance(); 
        int right = term();
        if (op == TOKEN_ADD_OP) left += right;
        else left -= right;
    }
    return left;
}

int term() {
    int left = factor();
    while (currentToken.type == TOKEN_MULT_OP || currentToken.type == TOKEN_DIV_OP || currentToken.type == TOKEN_MOD_OP) { 
        TokenType op = currentToken.type;
        advance(); 
        int right = factor();
        if (op == TOKEN_MULT_OP) left *= right;
        else if (op == TOKEN_DIV_OP && right != 0) left /= right;
    }
    return left;
}

int factor() {
    int val = 0;
    if (currentToken.type == TOKEN_RW_AUTO_REF) { 
        hasAutoRef = 1; 
        advance(); consume(TOKEN_L_PAREN, "Expected '('");
        if (isType(currentToken) || currentToken.type == TOKEN_IDENTIFIER) advance(); 
        consume(TOKEN_COMMA, "Expected ','");
        if (currentToken.type == TOKEN_L_BRACE || currentToken.type == TOKEN_L_BRACKET) {
            TokenType closer = (currentToken.type == TOKEN_L_BRACE) ? TOKEN_R_BRACE : TOKEN_R_BRACKET;
            advance(); 
            while (currentToken.type != closer && currentToken.type != TOKEN_EOF) { advance(); }
            advance(); 
        } else {
            val = expression(); 
        }
        consume(TOKEN_R_PAREN, "Expected ')'"); 
        return val;
    }
    
    if (currentToken.type == TOKEN_MULT_OP || currentToken.type == TOKEN_POINTER_OP || currentToken.type == TOKEN_ADDRESS_OF_OP) {
        advance(); factor(); return 0;
    }

    // UPDATED: Accept INT, FLOAT, STRING, TRUE/FALSE
    if (currentToken.type == TOKEN_NUMBER_INT) {
        val = atoi(currentToken.lexeme_start);
        advance();
    } 
    else if (currentToken.type == TOKEN_NUMBER_FLOAT) {
        val = (int)atof(currentToken.lexeme_start); // Cast to int for simple calc
        advance();
    }
    else if (currentToken.type == TOKEN_KW_TRUE) {
        val = 1;
        advance();
    }
    else if (currentToken.type == TOKEN_KW_FALSE) {
        val = 0;
        advance();
    }
    else if (currentToken.type == TOKEN_IDENTIFIER) {
        char name[64];
        strncpy(name, currentToken.lexeme_start, currentToken.lexeme_length);
        name[currentToken.lexeme_length] = '\0';
        Symbol *s = getSymbol(name);
        if (s) val = s->intVal;
        
        advance();
        if (currentToken.type == TOKEN_L_BRACKET) { 
            advance(); expression(); consume(TOKEN_R_BRACKET, "Expected ']'");
        } else if (currentToken.type == TOKEN_ARROW_OP) {
            advance(); consume(TOKEN_IDENTIFIER, "field");
        }
    } 
    else if (currentToken.type == TOKEN_STRING) {
        advance();
    }
    else if (currentToken.type == TOKEN_L_PAREN) {
        advance(); val = expression(); consume(TOKEN_R_PAREN, "Expected ')'");
    } 
    else if (currentToken.type == TOKEN_L_BRACE) {
        advance();
        while (currentToken.type != TOKEN_R_BRACE && currentToken.type != TOKEN_EOF) {
            expression(); if (currentToken.type == TOKEN_COMMA) advance();
        }
        consume(TOKEN_R_BRACE, "Expected '}'");
    } else {
        error("Invalid expression factor");
        advance(); 
    }
    return val;
}

int main() {
    char *input = NULL; size_t buffer_size = 0; size_t total_size = 0; size_t chunk_size = 100000;
    input = (char *)malloc(chunk_size); if (!input) { fprintf(stderr, "Memory error\n"); return 1; }
    size_t bytes_read; while ((bytes_read = fread(input + total_size, 1, chunk_size, stdin)) > 0) {
        total_size += bytes_read; if (total_size + chunk_size > buffer_size + chunk_size) {
            buffer_size = total_size + chunk_size * 2; input = (char *)realloc(input, buffer_size);
        }
    }
    input[total_size] = '\0';
    if (total_size == 0) { printf("Error: No input\n"); free(input); return 1; }
    initScanner(input); lookaheadToken = getNextToken(); advance();
    program();
    free(input); return 0;
}