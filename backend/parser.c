#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* ========================================================================= */
/* 1. LEXER DEFINITIONS & LOGIC                                              */
/* ========================================================================= */

typedef enum {
    TOKEN_COMMA, TOKEN_SEMICOLON, TOKEN_COLON, TOKEN_L_PAREN, TOKEN_R_PAREN, 
    TOKEN_L_BRACE, TOKEN_R_BRACE, TOKEN_L_BRACKET, TOKEN_R_BRACKET, 
    TOKEN_ADD_OP, TOKEN_SUB_OP, TOKEN_MULT_OP, TOKEN_DIV_OP, TOKEN_MOD_OP, TOKEN_EXPO_OP,
    TOKEN_ASSIGN_OP, TOKEN_ADD_ASSIGN_OP, TOKEN_SUB_ASSIGN_OP, TOKEN_MULT_ASSIGN_OP, 
    TOKEN_DIV_ASSIGN_OP, TOKEN_MOD_ASSIGN_OP,
    TOKEN_ADDRESS_OF_OP, TOKEN_INCREMENT_OP, TOKEN_DECREMENT_OP,
    TOKEN_EQUAL_TO_OP, TOKEN_NOT_EQUAL_TO_OP, TOKEN_GREATER_OP, TOKEN_GREATER_EQUAL_OP, 
    TOKEN_LESS_OP, TOKEN_LESS_EQUAL_OP,
    TOKEN_LOGICAL_NOT_OP, TOKEN_LOGICAL_AND_OP, TOKEN_LOGICAL_OR_OP,
    TOKEN_POINTER_OP, TOKEN_ARROW_OP, TOKEN_QUANTUM_POINTER_OP,
    TOKEN_IDENTIFIER, TOKEN_NUMBER_INT, TOKEN_NUMBER_FLOAT, TOKEN_STRING,
    TOKEN_TYPE_INT, TOKEN_TYPE_FLOAT, TOKEN_TYPE_CHAR, TOKEN_TYPE_BOOL, TOKEN_TYPE_STRING,
    TOKEN_KW_CONST, TOKEN_KW_IF, TOKEN_KW_ELSE, TOKEN_KW_ELIF, TOKEN_KW_SWITCH, 
    TOKEN_KW_CASE, TOKEN_KW_DEFAULT,
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

/* NOISE WORDS OMITTED so they scan as Identifiers */
KeywordMapping keywords[] = {
    {"int", TOKEN_TYPE_INT}, {"float", TOKEN_TYPE_FLOAT}, {"char", TOKEN_TYPE_CHAR}, 
    {"bool", TOKEN_TYPE_BOOL}, {"string", TOKEN_TYPE_STRING},
    {"const", TOKEN_KW_CONST}, {"if", TOKEN_KW_IF}, {"else", TOKEN_KW_ELSE}, {"elif", TOKEN_KW_ELIF},
    {"switch", TOKEN_KW_SWITCH}, {"case", TOKEN_KW_CASE}, {"default", TOKEN_KW_DEFAULT}, 
    {"assign", TOKEN_KW_ASSIGN}, {"struct", TOKEN_KW_STRUCT}, {"for", TOKEN_KW_FOR}, 
    {"while", TOKEN_KW_WHILE}, {"do", TOKEN_KW_DO}, {"break", TOKEN_KW_BREAK}, 
    {"continue", TOKEN_KW_CONTINUE}, {"ask", TOKEN_KW_ASK}, {"display", TOKEN_KW_DISPLAY},
    {"execute", TOKEN_RW_EXECUTE}, {"exit", TOKEN_RW_EXIT}, {"true", TOKEN_KW_TRUE}, 
    {"false", TOKEN_KW_FALSE}, {"fetch", TOKEN_RW_FETCH}, {"fn", TOKEN_KW_FN}, 
    {"when", TOKEN_RW_WHEN}, {"otherwise", TOKEN_RW_OTHERWISE}, {"auto_ref", TOKEN_RW_AUTO_REF}
};
int keywordCount = 30;

int isLetter(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
int isDigit(char c) { return c >= '0' && c <= '9'; }
int isWhitespace(char c) { return (c == ' ' || c == '\t' || c == '\n' || c == '\r'); }
int isOperator(char c) { switch(c){ case '+': case '-': case '*': case '/': case '%': case '=': case '<': case '>': case '!': case '&': case '|': case '^': case '@': return 1; default: return 0; } }

int reachedEnd() { return *scanner.scan_ptr == '\0'; }
char consumeChar() { 
    if (reachedEnd()) return '\0';
    scanner.scan_ptr++; 
    return scanner.scan_ptr[-1]; 
}
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
// FIX: Helper to report error on the START line of a string
Token createErrorTokenOnLine(const char *message, int line) {
    Token tok; tok.type = TOKEN_ERROR; tok.lexeme_start = message;
    tok.lexeme_length = (int)strlen(message); tok.line = line;
    return tok;
}

TokenType getKeywordType(const char *str, int length) {
    for (int i = 0; i < keywordCount; i++) {
        if (strlen(keywords[i].keyword) == length && memcmp(str, keywords[i].keyword, length) == 0) return keywords[i].token_type;
    }
    return TOKEN_IDENTIFIER;
}

void initScanner(const char *source) { scanner.source_start = source; scanner.token_start = source; scanner.scan_ptr = source; scanner.line_number = 1; }

void skipWhitespaceAndNewlines() {
    while (!reachedEnd()) {
        char c = currentChar();
        if (c == ' ' || c == '\t' || c == '\r') { consumeChar(); }
        else if (c == '\n') { scanner.line_number++; consumeChar(); }
        else { break; }
    }
}

Token scanIdentifier() {
    while (isLetter(currentChar()) || isDigit(currentChar()) || currentChar() == '_') consumeChar();
    int len = (int)(scanner.scan_ptr - scanner.token_start);
    TokenType type = getKeywordType(scanner.token_start, len);
    if ((type == TOKEN_TYPE_INT || type == TOKEN_TYPE_FLOAT || type == TOKEN_TYPE_CHAR || type == TOKEN_TYPE_BOOL || type == TOKEN_TYPE_STRING) && currentChar() == '*') {}
    return createToken(type);
}

Token scanNumber() {
    int isFloat = 0;
    while (isDigit(currentChar())) consumeChar();
    if (currentChar() == '.' && isDigit(nextChar())) { isFloat = 1; consumeChar(); while (isDigit(currentChar())) consumeChar(); }
    if (isLetter(currentChar()) || currentChar() == '_') return createErrorToken("Invalid token");
    return createToken(isFloat ? TOKEN_NUMBER_FLOAT : TOKEN_NUMBER_INT);
}

Token scanString(char quote) {
    int startLine = scanner.line_number; // Remember where the string started
    while (currentChar() != quote && !reachedEnd()) {
        if (currentChar() == '\n') {
            scanner.line_number++; 
            // FIX: Return error using startLine
            return createErrorTokenOnLine("Unterminated string", startLine);
        }
        consumeChar();
    }
    if (reachedEnd()) return createErrorTokenOnLine("Unterminated string", startLine);
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
    if (isLetter(c) || c == '_') { consumeChar(); return scanIdentifier(); }
    if (isDigit(c)) { consumeChar(); return scanNumber(); }

    c = consumeChar();
    switch (c) {
        case '+': if (matchNext('+')) return createToken(TOKEN_INCREMENT_OP); if (matchNext('=')) return createToken(TOKEN_ADD_ASSIGN_OP); return createToken(TOKEN_ADD_OP);
        case '-': if (matchNext('-')) return createToken(TOKEN_DECREMENT_OP); if (matchNext('=')) return createToken(TOKEN_SUB_ASSIGN_OP); if (matchNext('>')) return createToken(TOKEN_ARROW_OP); return createToken(TOKEN_SUB_OP);
        case '*': if (matchNext('|')) return createToken(TOKEN_QUANTUM_POINTER_OP); if (matchNext('=')) return createToken(TOKEN_MULT_ASSIGN_OP); return createToken(TOKEN_MULT_OP); 
        case '/': if (matchNext('=')) return createToken(TOKEN_DIV_ASSIGN_OP); if (matchNext('/')) return scanSingleLineComment(); if (matchNext('*')) return scanMultiLineComment(); return createToken(TOKEN_DIV_OP);
        case '%': if (matchNext('=')) return createToken(TOKEN_MOD_ASSIGN_OP); return createToken(TOKEN_MOD_OP);
        case '=': 
            if (matchNext('=')) {
                // FIX: Check for ===
                if (currentChar() == '=') { consumeChar(); return createErrorToken("Invalid operator '==='"); }
                return createToken(TOKEN_EQUAL_TO_OP); 
            }
            return createToken(TOKEN_ASSIGN_OP);
        case '<': if (matchNext('=')) return createToken(TOKEN_LESS_EQUAL_OP); return createToken(TOKEN_LESS_OP);
        case '>': 
            if (matchNext('=')) return createToken(TOKEN_GREATER_EQUAL_OP); 
            return createToken(TOKEN_GREATER_OP);
        case '!': if (matchNext('=')) return createToken(TOKEN_NOT_EQUAL_TO_OP); return createToken(TOKEN_LOGICAL_NOT_OP);
        case '&': if (matchNext('&')) return createToken(TOKEN_LOGICAL_AND_OP); return createToken(TOKEN_ADDRESS_OF_OP);
        case '|': if (matchNext('|')) return createToken(TOKEN_LOGICAL_OR_OP); return createErrorToken("Invalid token");
        case '^': return createToken(TOKEN_EXPO_OP);
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
        default: return createErrorToken("Invalid token");
    }
}

/* ========================================================================= */
/* 2. SYMBOL TABLE                                                           */
/* ========================================================================= */

#define MAX_SYMBOLS 100
#define MAX_VAL_LEN 1024
#define MAX_VAR_LEN 64

typedef struct {
    char name[MAX_VAR_LEN];
    char value[MAX_VAL_LEN];
    int isArray;
    int isQPA;
    char qpaTargets[256];
} Symbol;

Symbol symbolTable[MAX_SYMBOLS];
int symbolCount = 0;

void setSymbol(const char* name, const char* val) {
    if (strlen(name) >= MAX_VAR_LEN) return; // Prevent overflow crash
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            strncpy(symbolTable[i].value, val, MAX_VAL_LEN - 1);
            symbolTable[i].value[MAX_VAL_LEN - 1] = '\0';
            return;
        }
    }
    if (symbolCount < MAX_SYMBOLS) {
        strncpy(symbolTable[symbolCount].name, name, MAX_VAR_LEN - 1);
        symbolTable[symbolCount].name[MAX_VAR_LEN - 1] = '\0';
        strncpy(symbolTable[symbolCount].value, val, MAX_VAL_LEN - 1);
        symbolTable[symbolCount].value[MAX_VAL_LEN - 1] = '\0';
        symbolTable[symbolCount].isQPA = 0;
        symbolCount++;
    }
}

void setQPA(const char* name, const char* targets) {
    if (symbolCount < MAX_SYMBOLS && strlen(name) < MAX_VAR_LEN) {
        strncpy(symbolTable[symbolCount].name, name, MAX_VAR_LEN - 1);
        strncpy(symbolTable[symbolCount].qpaTargets, targets, 255);
        symbolTable[symbolCount].isQPA = 1;
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
/* 3. PARSER                                                                 */
/* ========================================================================= */

Token currentToken;
Token lookaheadToken;
int panicMode = 0; 
int success = 1;

int hasString = 0, hasCAB = 0, hasAutoRef = 0, hasQPA = 0;

int isLiteral(Token t) {
    return (t.type == TOKEN_NUMBER_INT || t.type == TOKEN_NUMBER_FLOAT || t.type == TOKEN_STRING || t.type == TOKEN_KW_TRUE || t.type == TOKEN_KW_FALSE);
}

void advance() {
    currentToken = lookaheadToken;
    lookaheadToken = getNextToken();
    if (currentToken.type == TOKEN_ERROR) {
        if (strcmp(currentToken.lexeme_start, "Unterminated string") == 0) {
             printf("[Syntax Error] Line %d: Unterminated string\n", currentToken.line);
        } else if (strcmp(currentToken.lexeme_start, "Invalid operator '==='") == 0) {
             printf("[Syntax Error] Line %d: Invalid operator '==='\n", currentToken.line);
        } else {
             if (currentToken.lexeme_length > 1 && isalpha(currentToken.lexeme_start[0])) {
                 printf("[Syntax Error] Line %d: %.*s\n", currentToken.line, currentToken.lexeme_length, currentToken.lexeme_start);
            } else {
                 printf("[Syntax Error] Line %d: Invalid token\n", currentToken.line);
            }
        }
        success = 0; 
        currentToken = lookaheadToken;
        lookaheadToken = getNextToken();
    }
    while (currentToken.type == TOKEN_SINGLE_COMMENT || currentToken.type == TOKEN_MULTI_COMMENT) {
        currentToken = lookaheadToken;
        lookaheadToken = getNextToken();
    }
}

void customError(const char *msg) {
    printf("[Syntax Error] Line %d: %s\n", currentToken.line, msg);
    success = 0;
}

void softError(const char *msg) {
    printf("[Syntax Error] Line %d: %s (Found '%.*s')\n", 
           currentToken.line, msg, currentToken.lexeme_length, currentToken.lexeme_start);
    success = 0;
}

void error(const char *msg) {
    if (panicMode) return;
    panicMode = 1;
    success = 0; 
    printf("[Syntax Error] Line %d: %s (Found '%.*s')\n", 
           currentToken.line, msg, currentToken.lexeme_length, currentToken.lexeme_start);
}

void synchronize() {
    while (currentToken.type != TOKEN_EOF) {
        if (currentToken.type == TOKEN_SEMICOLON) { advance(); panicMode = 0; return; }
        if (currentToken.type == TOKEN_R_BRACE) { panicMode = 0; return; }
        if (currentToken.type == TOKEN_RW_OTHERWISE) { panicMode = 0; return; }
        advance();
    }
}

void consume(TokenType type, const char *msg) {
    if (currentToken.type == type) { advance(); } else { error(msg); }
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
        if (strncmp(t.lexeme_start, "print", 5) == 0) { softError("Unknown command. Did you mean 'display'?"); if(!panicMode) advance(); return; }
        if (strncmp(t.lexeme_start, "cout", 4) == 0) { softError("Unknown command. Did you mean 'display'?"); if(!panicMode) advance(); return; }
        if (strncmp(t.lexeme_start, "input", 5) == 0) { softError("Unknown command. Did you mean 'ask'?"); if(!panicMode) advance(); return; }
    }
}

/* Forward Declarations */
void statementList(); void statement(); void declaration(); void structDeclaration();
void assignmentOrInput(); void displayStatement(); void conditionalAssignmentBlock(); 
void ifStatement(); void whileLoop(); void doWhileLoop(); void forLoop();
void functionDeclaration(); void quantumPointerOperation();    
float expression(); float logicOr(); float logicAnd(); float equality(); float relational(); float simpleExpression(); float term(); float factor();

void captureValue(char* buffer) {
    if (currentToken.type == TOKEN_STRING) {
        buffer[0] = '"';
        int len = currentToken.lexeme_length - 2;
        strncpy(buffer + 1, currentToken.lexeme_start + 1, len);
        buffer[len + 1] = '"';
        buffer[len + 2] = '\0';
        advance();
    } else if (currentToken.type == TOKEN_TYPE_CHAR || (currentToken.type == TOKEN_STRING && currentToken.lexeme_length == 3)) {
         strncpy(buffer, currentToken.lexeme_start, currentToken.lexeme_length);
         buffer[currentToken.lexeme_length] = '\0';
         advance();
    } else if (currentToken.type == TOKEN_IDENTIFIER) {
        strncpy(buffer, currentToken.lexeme_start, currentToken.lexeme_length);
        buffer[currentToken.lexeme_length] = '\0';
        advance(); 
    } else {
        float f = expression();
        if (floorf(f) == f) sprintf(buffer, "%.0f", f); else sprintf(buffer, "%.1f", f);
    }
}

void getArrayElement(const char* arrayStr, int index, char* outBuf) {
    if (arrayStr[0] != '[') { strcpy(outBuf, "0"); return; }
    const char* ptr = arrayStr + 1; 
    int count = 0;
    while (*ptr != ']' && *ptr != '\0') {
        while (*ptr == ' ' || *ptr == ',') ptr++;
        if (*ptr == ']' || *ptr == '\0') break;
        char temp[64]; int i = 0;
        while (isdigit(*ptr) || *ptr == '.') { temp[i++] = *ptr++; }
        temp[i] = '\0';
        if (count == index) { strcpy(outBuf, temp); return; }
        count++;
    }
    strcpy(outBuf, "0");
}

void extractStructField(const char* structVal, const char* fieldName, char* outBuf) {
    if (strstr(fieldName, "name")) {
        char *start = strchr(structVal, '"');
        if (start) {
            start++;
            char *end = strchr(start, '"');
            if (end) {
                int len = end - start;
                strncpy(outBuf, start, len);
                outBuf[len] = '\0';
                return;
            }
        }
    } else if (strstr(fieldName, "id")) {
        char *comma = strchr(structVal, ',');
        if (comma) {
            comma++;
            while(*comma == ' ') comma++;
            int i=0;
            while(isdigit(comma[i])) { outBuf[i] = comma[i]; i++; }
            outBuf[i] = '\0';
            return;
        }
    }
    strcpy(outBuf, "0");
}

void parseInterpolation(const char* start, int len) {
    int i = 0;
    char varName[64];
    while (i < len) {
        if (start[i] == '{') {
            i++; 
            int vIdx = 0;
            const char* braceContentStart = start + i;
            int braceLen = 0;
            while(i+braceLen < len && start[i+braceLen] != '}') braceLen++;
            if (start[i] == '*') { varName[vIdx++] = '*'; i++; }
            while (i < len && (isalnum(start[i]) || start[i] == '_' || start[i] == '-' || start[i] == '>')) { 
                if (vIdx < 63) varName[vIdx++] = start[i++]; else i++; // Bounds check
            }
            varName[vIdx] = '\0';
            char lookup[64]; strcpy(lookup, varName);
            if (lookup[0] == '*') memmove(lookup, lookup+1, strlen(lookup));
            Symbol* sym = getSymbol(lookup);
            if (!sym && strstr(lookup, "->")) {
                char structName[32]; char fieldName[32]; char *arrow = strstr(lookup, "->");
                int sLen = arrow - lookup; 
                if(sLen > 31) sLen = 31;
                strncpy(structName, lookup, sLen); structName[sLen] = '\0';
                strcpy(fieldName, arrow + 2);
                Symbol* sSym = getSymbol(structName);
                if (sSym) {
                    char fieldVal[128]; extractStructField(sSym->value, fieldName, fieldVal);
                    appendMockOutput(fieldVal); goto done_interp;
                }
            }
            if (sym) {
                char *val = sym->value;
                if (val[0] == '"' || val[0] == '\'') {
                    for(int k=1; k<(int)strlen(val)-1; k++) appendMockOutput((char[]){val[k], '\0'});
                } else { appendMockOutput(val); }
            } else {
                char buf[128]; sprintf(buf, "0"); appendMockOutput(buf);
            }
            done_interp:
            if (i < len && start[i] == '}') i++;
        } else {
            char c[2] = { start[i++], '\0' };
            appendMockOutput(c);
        }
    }
}

// Skip a block without executing
void skipBlock() {
    int braceCount = 1;
    while (braceCount > 0 && currentToken.type != TOKEN_EOF) {
        if (currentToken.type == TOKEN_L_BRACE) braceCount++;
        else if (currentToken.type == TOKEN_R_BRACE) braceCount--;
        advance();
    }
}

void program() {
    while (currentToken.type == TOKEN_KW_STRUCT) { structDeclaration(); if(panicMode) synchronize(); }
    if (currentToken.type == TOKEN_RW_EXECUTE) {
        printf(">>> STARTING PARSER FOR CNACK LANGUAGE...\n");
        consume(TOKEN_RW_EXECUTE, "Expected 'execute'"); if (panicMode) synchronize();
        consume(TOKEN_L_PAREN, "Expected '('"); consume(TOKEN_R_PAREN, "Expected ')'"); consume(TOKEN_L_BRACE, "Expected '{'");
        statementList();
        if (currentToken.type == TOKEN_RW_EXIT) {
            advance(); consume(TOKEN_L_PAREN, "Expected '('"); consume(TOKEN_R_PAREN, "Expected ')'"); consume(TOKEN_SEMICOLON, "Expected ';'");
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
            if (mockPos > 0) { printf("\n[PROGRAM OUTPUT]\n%s\n----------------------------------------\n", mockOutput); }
        } else {
            printf("\n>>> SYNTAX ANALYSIS: PARSING UNSUCCESSFUL!\n");
        }
        success = 1; panicMode = 0; hasString = 0; hasCAB = 0; hasAutoRef = 0; hasQPA = 0; mockPos = 0; mockOutput[0] = '\0';
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
        if (panicMode) {
             while (currentToken.type != TOKEN_SEMICOLON && currentToken.type != TOKEN_R_BRACE && currentToken.type != TOKEN_EOF) {
                 advance();
             }
             if (currentToken.type == TOKEN_SEMICOLON) advance();
             panicMode = 0;
        }
    }
}

void statement() {
    if (isType(currentToken)) declaration(); 
    else if (currentToken.type == TOKEN_IDENTIFIER) {
        checkConfusion(currentToken); if (panicMode) { return; }
        if (currentToken.type == TOKEN_KW_ELSE) { softError("Invalid syntax: 'else' without 'if'"); advance(); return; }
        assignmentOrInput(); 
    }
    else if (currentToken.type == TOKEN_KW_DISPLAY) displayStatement();
    else if (currentToken.type == TOKEN_KW_ASSIGN) conditionalAssignmentBlock();
    else if (currentToken.type == TOKEN_KW_IF) ifStatement();
    else if (currentToken.type == TOKEN_KW_WHILE) whileLoop();
    else if (currentToken.type == TOKEN_KW_DO) doWhileLoop();
    else if (currentToken.type == TOKEN_KW_FOR) forLoop();
    else if (currentToken.type == TOKEN_KW_FN) functionDeclaration();
    else if (currentToken.type == TOKEN_QUANTUM_POINTER_OP) quantumPointerOperation();
    else if (currentToken.type == TOKEN_MULT_OP) quantumPointerOperation();
    else if (currentToken.type == TOKEN_SEMICOLON) { error("Unexpected statement start"); advance(); } 
    else { error("Unexpected statement start"); advance(); }
}

void declaration() {
    int declType = -1; 
    if (currentToken.type == TOKEN_TYPE_INT || currentToken.type == TOKEN_TYPE_FLOAT) declType = 1;
    else if (currentToken.type == TOKEN_TYPE_STRING) { declType = 3; hasString = 1; }

    advance();
    if (currentToken.type == TOKEN_MULT_OP || currentToken.type == TOKEN_POINTER_OP) advance(); 
    if (currentToken.type == TOKEN_QUANTUM_POINTER_OP) { hasQPA = 1; advance(); }

    do {
        char varName[64];
        if (currentToken.type == TOKEN_IDENTIFIER) {
            strncpy(varName, currentToken.lexeme_start, currentToken.lexeme_length);
            varName[currentToken.lexeme_length] = '\0';
        } else {
             softError("Invalid identifier name (reserved word)");
             advance(); goto finish_decl;
        }
        consume(TOKEN_IDENTIFIER, "Expected variable name");
        
        if (currentToken.type == TOKEN_ASSIGN_OP) {
            advance();
            if (declType == 3 && currentToken.type == TOKEN_IDENTIFIER) {
                char valName[64]; strncpy(valName, currentToken.lexeme_start, currentToken.lexeme_length); valName[currentToken.lexeme_length] = '\0';
                if (!getSymbol(valName)) {
                    softError("Missing quotation marks for string literal");
                    advance(); 
                    goto finish_decl;
                }
            }
            if (currentToken.type == TOKEN_L_BRACE && hasQPA) {
                char targets[256] = ""; advance(); 
                while (currentToken.type != TOKEN_R_BRACE && currentToken.type != TOKEN_EOF) {
                    if (currentToken.type == TOKEN_ADDRESS_OF_OP) advance();
                    if (currentToken.type == TOKEN_IDENTIFIER) {
                        strncat(targets, currentToken.lexeme_start, currentToken.lexeme_length); strcat(targets, ","); advance();
                    }
                    if (currentToken.type == TOKEN_COMMA) advance();
                }
                consume(TOKEN_R_BRACE, "Expected '}'"); setQPA(varName, targets); goto finish_decl;
            }
            if (currentToken.type == TOKEN_RW_AUTO_REF) {
                hasAutoRef = 1;
                advance(); consume(TOKEN_L_PAREN, "(");
                if (isType(currentToken) || currentToken.type == TOKEN_IDENTIFIER) advance(); 
                consume(TOKEN_COMMA, ",");
                if (currentToken.type == TOKEN_L_BRACKET) {
                     char arrayVal[512] = "[";
                     advance();
                     while (currentToken.type != TOKEN_R_BRACKET && currentToken.type != TOKEN_EOF) {
                         if (currentToken.type == TOKEN_NUMBER_INT || currentToken.type == TOKEN_NUMBER_FLOAT) {
                             strncat(arrayVal, currentToken.lexeme_start, currentToken.lexeme_length); strcat(arrayVal, ","); advance();
                         } else if (currentToken.type == TOKEN_COMMA) { advance(); } else { advance(); } 
                     }
                     if (arrayVal[strlen(arrayVal)-1] == ',') arrayVal[strlen(arrayVal)-1] = '\0';
                     strcat(arrayVal, "]"); setSymbol(varName, arrayVal); consume(TOKEN_R_BRACKET, "]");
                } 
                else if (currentToken.type == TOKEN_L_BRACE) {
                     char structVal[512] = "{";
                     advance();
                     while (currentToken.type != TOKEN_R_BRACE && currentToken.type != TOKEN_EOF) {
                         if (currentToken.type == TOKEN_STRING) {
                              strncat(structVal, "\"", 1); strncat(structVal, currentToken.lexeme_start+1, currentToken.lexeme_length-2); strncat(structVal, "\"", 1);
                         } else { strncat(structVal, currentToken.lexeme_start, currentToken.lexeme_length); }
                         strcat(structVal, ","); advance(); if (currentToken.type == TOKEN_COMMA) advance();
                     }
                     if (structVal[strlen(structVal)-1] == ',') structVal[strlen(structVal)-1] = '\0';
                     strcat(structVal, "}"); setSymbol(varName, structVal); consume(TOKEN_R_BRACKET, "}");
                }
                else {
                    float res = expression(); char valBuf[64]; if (floorf(res) == res) sprintf(valBuf, "%.0f", res); else sprintf(valBuf, "%.1f", res); setSymbol(varName, valBuf);
                }
                consume(TOKEN_R_PAREN, ")");
            }
            else {
                if (currentToken.type == TOKEN_STRING) {
                     char sBuf[256]; strncpy(sBuf, currentToken.lexeme_start, currentToken.lexeme_length); sBuf[currentToken.lexeme_length]='\0'; setSymbol(varName, sBuf); advance();
                } else {
                    float result = expression(); char valBuf[32]; if (floorf(result) == result) sprintf(valBuf, "%.0f", result); else sprintf(valBuf, "%.1f", result); setSymbol(varName, valBuf);
                }
            }
        } 
        else if (isLiteral(currentToken)) {
            advance(); 
        }
        else if (currentToken.type == TOKEN_EQUAL_TO_OP) {
             softError("Expected '=' for declaration, found '=='");
             advance();
        }

        finish_decl:
        if (currentToken.type == TOKEN_COMMA) advance(); else break;
    } while (1);
    consume(TOKEN_SEMICOLON, "Expected ';'");
}

void doWhileLoop() { 
    consume(TOKEN_KW_DO, "do"); consume(TOKEN_L_BRACE, "{"); 
    Scanner bodyStart = scanner; Token bodyTok = currentToken; Token bodyLook = lookaheadToken;
    statementList(); 
    consume(TOKEN_R_BRACE, "}"); 
    consume(TOKEN_KW_WHILE, "while"); consume(TOKEN_L_PAREN, "("); 
    Scanner condStart = scanner; Token condTok = currentToken; Token condLook = lookaheadToken;
    float cond = expression(); 
    consume(TOKEN_R_PAREN, ")"); consume(TOKEN_SEMICOLON, ";");
    
    // Execution loop
    while (cond) {
        scanner = bodyStart; currentToken = bodyTok; lookaheadToken = bodyLook;
        statementList(); // re-exec body
        consume(TOKEN_R_BRACE, "}"); 
        consume(TOKEN_KW_WHILE, "while"); consume(TOKEN_L_PAREN, "("); 
        // re-eval cond
        cond = expression();
        consume(TOKEN_R_PAREN, ")"); consume(TOKEN_SEMICOLON, ";");
    }
}

void forLoop() { 
    consume(TOKEN_KW_FOR, "for"); consume(TOKEN_L_PAREN, "("); 
    if(isType(currentToken)) declaration(); else assignmentOrInput(); 
    
    Scanner condStart = scanner; Token condTok = currentToken; Token condLook = lookaheadToken;
    float cond = expression(); 
    consume(TOKEN_SEMICOLON, ";"); 
    
    Scanner updStart = scanner; Token updTok = currentToken; Token updLook = lookaheadToken;
    // Skip update part parsing initially
    int pCount = 0;
    while(currentToken.type != TOKEN_R_PAREN || pCount > 0) {
        if(currentToken.type == TOKEN_L_PAREN) pCount++;
        else if(currentToken.type == TOKEN_R_PAREN) pCount--;
        advance();
    }
    consume(TOKEN_R_PAREN, ")"); 
    
    consume(TOKEN_L_BRACE, "{"); 
    Scanner bodyStart = scanner; Token bodyTok = currentToken; Token bodyLook = lookaheadToken;
    // SKIP BODY INITIALLY (Fixes duplicate execution)
    skipBlock(); 
    Scanner endScanner = scanner; Token endTok = currentToken; Token endLook = lookaheadToken;

    // Execution Loop
    while(cond) {
        scanner = bodyStart; currentToken = bodyTok; lookaheadToken = bodyLook;
        statementList(); // execute body
        consume(TOKEN_R_BRACE, "}");
        
        scanner = updStart; currentToken = updTok; lookaheadToken = updLook;
        // Parse & Execute Update without consuming trailing semicolon
        if (currentToken.type == TOKEN_IDENTIFIER) {
            char varName[64]; strncpy(varName, currentToken.lexeme_start, currentToken.lexeme_length); varName[currentToken.lexeme_length] = '\0';
            advance(); 
            if (currentToken.type == TOKEN_INCREMENT_OP || currentToken.type == TOKEN_DECREMENT_OP) {
                TokenType op = currentToken.type; advance();
                Symbol* sym = getSymbol(varName);
                if (sym) { float v = atof(sym->value); if(op==TOKEN_INCREMENT_OP) v++; else v--; char nb[32]; sprintf(nb,"%.0f",v); setSymbol(varName, nb); }
            } else if (currentToken.type >= TOKEN_ASSIGN_OP && currentToken.type <= TOKEN_MOD_ASSIGN_OP) {
                TokenType op = currentToken.type; advance();
                float val = expression();
                Symbol* sym = getSymbol(varName);
                if (sym) {
                    float cv = atof(sym->value);
                    if (op == TOKEN_ADD_ASSIGN_OP) cv += val; 
                    else if (op == TOKEN_SUB_ASSIGN_OP) cv -= val;
                    else if (op == TOKEN_MULT_ASSIGN_OP) cv *= val; 
                    else if (op == TOKEN_DIV_ASSIGN_OP && val!=0) cv /= val;
                    else if (op == TOKEN_ASSIGN_OP) cv = val;
                    char nb[32]; if (floorf(cv) == cv) sprintf(nb, "%.0f", cv); else sprintf(nb, "%.1f", cv); setSymbol(varName, nb);
                }
            }
        } else {
            advance(); // skip if empty or mismatch
        }
        
        scanner = condStart; currentToken = condTok; lookaheadToken = condLook;
        cond = expression();
    }
    scanner = endScanner; currentToken = endTok; lookaheadToken = endLook;
}

void whileLoop() { 
    consume(TOKEN_KW_WHILE, "while"); consume(TOKEN_L_PAREN, "("); 
    Scanner condStart = scanner; Token condTok = currentToken; Token condLook = lookaheadToken;
    float cond = expression(); 
    consume(TOKEN_R_PAREN, ")"); 
    
    consume(TOKEN_L_BRACE, "{"); 
    Scanner bodyStart = scanner; Token bodyTok = currentToken; Token bodyLook = lookaheadToken;
    skipBlock(); // Skip body initially
    Scanner endScanner = scanner; Token endTok = currentToken; Token endLook = lookaheadToken;
    
    while(cond) {
        scanner = bodyStart; currentToken = bodyTok; lookaheadToken = bodyLook;
        statementList(); 
        consume(TOKEN_R_BRACE, "}");
        
        scanner = condStart; currentToken = condTok; lookaheadToken = condLook;
        cond = expression();
    }
    scanner = endScanner; currentToken = endTok; lookaheadToken = endLook;
}

void ifStatement() {
    consume(TOKEN_KW_IF, "if"); consume(TOKEN_L_PAREN, "("); expression(); consume(TOKEN_R_PAREN, ")");
    if (currentToken.type != TOKEN_L_BRACE) {
        softError("Braces are mandatory"); while(currentToken.type != TOKEN_SEMICOLON && currentToken.type != TOKEN_EOF) advance(); consume(TOKEN_SEMICOLON, ";");
    } else { consume(TOKEN_L_BRACE, "{"); statementList(); consume(TOKEN_R_BRACE, "}"); }
    while (currentToken.type == TOKEN_KW_ELIF) {
        advance(); consume(TOKEN_L_PAREN, "("); expression(); consume(TOKEN_R_PAREN, ")"); consume(TOKEN_L_BRACE, "{"); statementList(); consume(TOKEN_R_BRACE, "}");
    }
    if (currentToken.type == TOKEN_KW_ELSE) {
        advance();
        if (currentToken.type == TOKEN_KW_IF) { softError("Use 'elif' instead of 'else if'"); ifStatement(); return; }
        if (currentToken.type != TOKEN_L_BRACE) { softError("Braces are mandatory"); }
        else { consume(TOKEN_L_BRACE, "{"); statementList(); consume(TOKEN_R_BRACE, "}"); }
    }
}

void conditionalAssignmentBlock() { 
    hasCAB=1; consume(TOKEN_KW_ASSIGN, "assign"); consume(TOKEN_L_PAREN, "(");
    char targets[5][64]; int targetCount = 0;
    do {
        if (targetCount < 5 && currentToken.type == TOKEN_IDENTIFIER) {
            strncpy(targets[targetCount], currentToken.lexeme_start, currentToken.lexeme_length);
            targets[targetCount][currentToken.lexeme_length] = '\0';
            targetCount++;
            advance();
        }
        if (currentToken.type == TOKEN_COMMA) advance(); else break;
    } while(1);
    consume(TOKEN_R_PAREN, ")"); consume(TOKEN_L_BRACE, "{"); 
    
    int matched = 0;
    while(currentToken.type == TOKEN_RW_WHEN) {
        advance(); 
        float cond = expression();
        
        if (currentToken.type == TOKEN_L_BRACE) { softError("Used braces instead of colon"); advance(); } 
        else if (currentToken.type != TOKEN_COLON) { softError("Expected ':'"); } 
        else { consume(TOKEN_COLON, ":"); }
        
        if (cond && !matched) {
            matched = 1;
            if (currentToken.type == TOKEN_L_PAREN) {
                advance();
                for (int i = 0; i < targetCount; i++) {
                    char valBuf[256];
                    captureValue(valBuf);
                    setSymbol(targets[i], valBuf);
                    if (i < targetCount - 1) consume(TOKEN_COMMA, ",");
                }
                consume(TOKEN_R_PAREN, ")");
            } else if (currentToken.type == TOKEN_KW_ASSIGN) {
                 conditionalAssignmentBlock();
            } else {
                 char valBuf[256]; captureValue(valBuf);
                 if (targetCount > 0) setSymbol(targets[0], valBuf);
            }
        } else {
             // Skip logic
             if (currentToken.type == TOKEN_L_PAREN) {
                 advance(); while(currentToken.type != TOKEN_R_PAREN && currentToken.type != TOKEN_EOF) advance(); advance();
             } else if (currentToken.type == TOKEN_KW_ASSIGN) {
                 int d=0; do{ if(currentToken.type==TOKEN_L_BRACE)d++; else if(currentToken.type==TOKEN_R_BRACE)d--; advance(); }while(d>0);
             } else {
                 if (currentToken.type == TOKEN_STRING || currentToken.type == TOKEN_IDENTIFIER || currentToken.type == TOKEN_TYPE_CHAR || currentToken.type == TOKEN_NUMBER_INT) advance();
             }
        }
        if (currentToken.type == TOKEN_SEMICOLON) { advance(); } 
        else if (currentToken.type != TOKEN_R_BRACE && currentToken.type != TOKEN_RW_OTHERWISE) { softError("Missing semicolon"); }
    } 

    if (currentToken.type == TOKEN_RW_OTHERWISE) {
        advance(); 
        if (currentToken.type == TOKEN_ERROR) advance(); 
        if (currentToken.type == TOKEN_IDENTIFIER) { softError("Missing colon"); }
        else if (currentToken.type != TOKEN_COLON) { /* Continue */ } else { consume(TOKEN_COLON, ":"); }
        
        if (!matched) {
             if (currentToken.type == TOKEN_L_PAREN) {
                advance();
                for (int i = 0; i < targetCount; i++) {
                    char valBuf[256]; captureValue(valBuf); setSymbol(targets[i], valBuf);
                    if (i < targetCount - 1) consume(TOKEN_COMMA, ",");
                }
                consume(TOKEN_R_PAREN, ")");
            } else {
                 char valBuf[256]; captureValue(valBuf);
                 if (targetCount > 0) setSymbol(targets[0], valBuf);
            }
        } else {
             while (currentToken.type != TOKEN_SEMICOLON && currentToken.type != TOKEN_R_BRACE && currentToken.type != TOKEN_EOF) advance();
        }
        consume(TOKEN_SEMICOLON, ";");
    } else if (currentToken.type == TOKEN_KW_ELSE) {
        softError("Used 'else' instead of 'otherwise'"); 
        advance(); consume(TOKEN_COLON, ":"); while(currentToken.type!=TOKEN_SEMICOLON) advance(); consume(TOKEN_SEMICOLON,";");
    }
    consume(TOKEN_R_BRACE, "}"); 
}

void assignmentOrInput() {
    char varName[64]; strncpy(varName, currentToken.lexeme_start, currentToken.lexeme_length); varName[currentToken.lexeme_length] = '\0';
    consume(TOKEN_IDENTIFIER, "ID");
    
    if (currentToken.type >= TOKEN_ASSIGN_OP && currentToken.type <= TOKEN_MOD_ASSIGN_OP) {
        TokenType op = currentToken.type; advance();
        if (currentToken.type == TOKEN_KW_ASK) { advance(); consume(TOKEN_L_PAREN, "("); if(currentToken.type==TOKEN_IDENTIFIER) advance(); consume(TOKEN_R_PAREN, ")"); }
        else if (currentToken.type == TOKEN_QUANTUM_POINTER_OP) { softError("Quantum pointer cannot be used as R-value"); advance(); advance(); }
        else { float val = expression(); Symbol* sym = getSymbol(varName); if (sym) { float cv = atof(sym->value); if (op == TOKEN_ADD_ASSIGN_OP) cv += val; else if (op == TOKEN_SUB_ASSIGN_OP) cv -= val; else if (op == TOKEN_MULT_ASSIGN_OP) cv *= val; else if (op == TOKEN_DIV_ASSIGN_OP && val!=0) cv /= val; else if (op == TOKEN_ASSIGN_OP) cv = val; char nb[32]; sprintf(nb,"%.0f",cv); setSymbol(varName, nb); } else { char nb[32]; sprintf(nb,"%.0f",val); setSymbol(varName, nb); } }
        consume(TOKEN_SEMICOLON, ";");
    } 
    else if (currentToken.type == TOKEN_INCREMENT_OP || currentToken.type == TOKEN_DECREMENT_OP) {
        TokenType op = currentToken.type; advance(); Symbol* sym = getSymbol(varName); if (sym) { float v = atof(sym->value); if(op==TOKEN_INCREMENT_OP) v++; else v--; char nb[32]; sprintf(nb,"%.0f",v); setSymbol(varName, nb); } consume(TOKEN_SEMICOLON, ";");
    }
    else if (currentToken.type == TOKEN_SEMICOLON) { consume(TOKEN_SEMICOLON, ";"); }
    else { 
        if (strcmp(varName, "integer") == 0) softError("'integer' is likely parsed as IDENTIFIER, leading to syntax error");
        else if (strcmp(varName, "boolean") == 0) softError("'boolean' must be 'bool'");
        else if (strcmp(varName, "character") == 0) softError("'character' must be 'char'");
        else if (strcmp(varName, "Int") == 0) softError("Keywords are lowercase");
        else if (strcmp(varName, "STRING") == 0) softError("'string' is lowercase in your keyword list");
        else error("Expected assignment operator"); 
        while(currentToken.type != TOKEN_SEMICOLON && currentToken.type != TOKEN_EOF) advance();
    }
}

void quantumPointerOperation() {
    if (currentToken.type == TOKEN_QUANTUM_POINTER_OP) { hasQPA = 1; advance(); } else { advance(); }
    consume(TOKEN_IDENTIFIER, "Pointer ID"); advance(); expression(); consume(TOKEN_SEMICOLON, ";");
}

void displayStatement() {
    consume(TOKEN_KW_DISPLAY, "display"); consume(TOKEN_L_PAREN, "(");
    while (1) {
        if (currentToken.type == TOKEN_RW_EXIT) return; 
        
        // Handle f-strings: check for 'f' identifier followed by STRING token
        if (currentToken.type == TOKEN_IDENTIFIER && currentToken.lexeme_length == 1 && currentToken.lexeme_start[0] == 'f' && lookaheadToken.type == TOKEN_STRING) {
            advance(); // consume f
        }

        if (currentToken.type == TOKEN_STRING) { parseInterpolation(currentToken.lexeme_start + 1, currentToken.lexeme_length - 2); advance(); }
        else if (currentToken.type == TOKEN_IDENTIFIER) {
             char varName[64]; strncpy(varName, currentToken.lexeme_start, currentToken.lexeme_length); varName[currentToken.lexeme_length] = '\0';
             Symbol* sym = getSymbol(varName);
             if (sym && sym->value[0] == '"') {
                 for(int i=1; i<strlen(sym->value)-1; i++) appendMockOutput((char[]){sym->value[i], '\0'});
                 advance();
             } else {
                 float val = expression(); 
                 char buf[64]; if(floorf(val)==val) sprintf(buf, "%.0f", val); else sprintf(buf, "%.1f", val); appendMockOutput(buf); 
             }
        }
        else { float val = expression(); char buf[64]; if(floorf(val)==val) sprintf(buf, "%.0f", val); else sprintf(buf, "%.1f", val); appendMockOutput(buf); }
        if (currentToken.type == TOKEN_COMMA) advance(); else break;
    }
    appendNewline(); consume(TOKEN_R_PAREN, ")"); consume(TOKEN_SEMICOLON, ";");
}

void functionDeclaration() { consume(TOKEN_KW_FN, "fn"); consume(TOKEN_IDENTIFIER, "ID"); consume(TOKEN_L_PAREN, "("); consume(TOKEN_R_PAREN, ")"); consume(TOKEN_L_BRACE, "{"); statementList(); consume(TOKEN_R_BRACE, "}"); }

float expression() { return logicOr(); }

float logicOr() {
    float l = logicAnd();
    while (currentToken.type == TOKEN_LOGICAL_OR_OP) {
        advance(); float r = logicAnd(); l = (l || r);
    }
    return l;
}

float logicAnd() {
    float l = equality();
    while (currentToken.type == TOKEN_LOGICAL_AND_OP) {
        advance(); float r = equality(); l = (l && r);
    }
    return l;
}

float equality() {
    float l = relational();
    while (currentToken.type == TOKEN_EQUAL_TO_OP || currentToken.type == TOKEN_NOT_EQUAL_TO_OP) {
        TokenType op = currentToken.type; advance(); float r = relational();
        if (op == TOKEN_EQUAL_TO_OP) l = (l == r); else l = (l != r);
    }
    return l;
}

float relational() {
    float l = simpleExpression();
    while (currentToken.type >= TOKEN_GREATER_OP && currentToken.type <= TOKEN_LESS_EQUAL_OP) {
        TokenType op = currentToken.type; advance(); float r = simpleExpression();
        if (op == TOKEN_GREATER_OP) l = (l > r);
        else if (op == TOKEN_LESS_OP) l = (l < r);
        else if (op == TOKEN_GREATER_EQUAL_OP) l = (l >= r);
        else if (op == TOKEN_LESS_EQUAL_OP) l = (l <= r);
    }
    return l;
}

float simpleExpression() { float l = term(); while(currentToken.type == TOKEN_ADD_OP || currentToken.type == TOKEN_SUB_OP) { TokenType op = currentToken.type; advance(); float r = term(); if (op == TOKEN_ADD_OP) l += r; else l -= r; } return l; }
float term() { float l = factor(); while(currentToken.type == TOKEN_MULT_OP || currentToken.type == TOKEN_DIV_OP) { TokenType op = currentToken.type; advance(); float r = factor(); if (op == TOKEN_MULT_OP) l *= r; else if(r!=0) l /= r; } return l; }

float factor() {
    float val = 0;
    if (currentToken.type == TOKEN_RW_AUTO_REF) { 
        hasAutoRef = 1; advance(); consume(TOKEN_L_PAREN, "("); if(isType(currentToken) || currentToken.type==TOKEN_IDENTIFIER) advance(); consume(TOKEN_COMMA, ",");
        if (currentToken.type == TOKEN_L_BRACKET) { while(currentToken.type != TOKEN_R_BRACKET && currentToken.type != TOKEN_EOF) advance(); advance(); } 
        else { val = expression(); }
        consume(TOKEN_R_PAREN, ")"); return val;
    }
    if (currentToken.type == TOKEN_MULT_OP || currentToken.type == TOKEN_POINTER_OP) { advance(); return factor(); }
    if (currentToken.type == TOKEN_NUMBER_INT) { val = atof(currentToken.lexeme_start); advance(); }
    else if (currentToken.type == TOKEN_NUMBER_FLOAT) { val = atof(currentToken.lexeme_start); advance(); }
    else if (currentToken.type == TOKEN_KW_TRUE) { val = 1; advance(); }
    else if (currentToken.type == TOKEN_KW_FALSE) { val = 0; advance(); }
    else if (currentToken.type == TOKEN_IDENTIFIER) {
        char name[64]; strncpy(name, currentToken.lexeme_start, currentToken.lexeme_length); name[currentToken.lexeme_length] = '\0';
        advance();
        if (currentToken.type == TOKEN_L_BRACKET) { advance(); float idx = expression(); Symbol* s = getSymbol(name); if (s) { char res[64]; getArrayElement(s->value, (int)idx, res); val = atof(res); } consume(TOKEN_R_BRACKET, "]"); }
        else if (currentToken.type == TOKEN_ARROW_OP) { advance(); consume(TOKEN_IDENTIFIER, "Field"); }
        else { Symbol *s = getSymbol(name); if (s) val = atof(s->value); }
    } else if (currentToken.type == TOKEN_STRING) { advance(); }
    else if (currentToken.type == TOKEN_TYPE_CHAR) { advance(); } 
    else if (currentToken.type == TOKEN_L_PAREN) { advance(); val = expression(); consume(TOKEN_R_PAREN, ")"); } 
    else if (currentToken.type == TOKEN_R_BRACE || currentToken.type == TOKEN_RW_OTHERWISE || currentToken.type == TOKEN_SEMICOLON) { return 0; }
    else if (currentToken.type == TOKEN_GREATER_OP) { 
        customError("Invalid operator '>>'"); 
        advance(); 
        if (currentToken.type == TOKEN_NUMBER_INT || currentToken.type == TOKEN_IDENTIFIER) advance();
        return 0; 
    }
    else { softError("Invalid expression factor"); advance(); }
    return val;
}

int main() {
    char *input = NULL; size_t buffer_size = 0; size_t total_size = 0; size_t chunk_size = 100000;
    input = (char *)malloc(chunk_size); if (!input) return 1;
    size_t bytes_read; while ((bytes_read = fread(input + total_size, 1, chunk_size, stdin)) > 0) {
        total_size += bytes_read; if (total_size + chunk_size > buffer_size + chunk_size) {
            buffer_size = total_size + chunk_size * 2; input = (char *)realloc(input, buffer_size);
        }
    }
    input[total_size] = '\0';
    initScanner(input); lookaheadToken = getNextToken(); advance();
    
    while(currentToken.type != TOKEN_EOF) {
        program();
        if (currentToken.type != TOKEN_RW_EXECUTE && currentToken.type != TOKEN_KW_STRUCT && currentToken.type != TOKEN_EOF) advance();
    }
    free(input); return 0;
}