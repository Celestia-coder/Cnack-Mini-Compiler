/* CNACK LEXICAL ANALYZER - Web Version with Specific Token Types */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========== TOKEN DEFINITIONS ========== */

/* Token Types - Specific for each token */
typedef enum
{
    /* --- Separators --- */
    TOKEN_COMMA,     // ,
    TOKEN_SEMICOLON, // ;
    TOKEN_COLON,     // :
    TOKEN_L_PAREN,   // (
    TOKEN_R_PAREN,   // )
    TOKEN_L_BRACE,   // {
    TOKEN_R_BRACE,   // }
    TOKEN_L_BRACKET, // [
    TOKEN_R_BRACKET, // ]

    /* --- Arithmetic Operators --- */
    TOKEN_ADD_OP,  // +
    TOKEN_SUB_OP,  // -
    TOKEN_MULT_OP, // *
    TOKEN_DIV_OP,  // /
    TOKEN_MOD_OP,  // %
    TOKEN_EXPO_OP, // ^

    /* --- Assignment Operators --- */
    TOKEN_ASSIGN_OP,      // =
    TOKEN_ADD_ASSIGN_OP,  // +=
    TOKEN_SUB_ASSIGN_OP,  // -=
    TOKEN_MULT_ASSIGN_OP, // *=
    TOKEN_DIV_ASSIGN_OP,  // /=
    TOKEN_MOD_ASSIGN_OP,  // %=

    /* --- Unary Operators --- */
    TOKEN_ADDRESS_OF_OP, // & (for dereferencing pointer)
    TOKEN_INCREMENT_OP,  // ++
    TOKEN_DECREMENT_OP,  // --

    /* --- Relational Operators --- */
    TOKEN_EQUAL_TO_OP,      // ==
    TOKEN_NOT_EQUAL_TO_OP,  // !=
    TOKEN_GREATER_OP,       // >
    TOKEN_GREATER_EQUAL_OP, // >=
    TOKEN_LESS_OP,          // <
    TOKEN_LESS_EQUAL_OP,    // <=

    /* --- Logical Operators --- */
    TOKEN_LOGICAL_NOT_OP, // !
    TOKEN_LOGICAL_AND_OP, // &&
    TOKEN_LOGICAL_OR_OP,  // ||

    /* --- Pointer Structure Operator --- */
    TOKEN_POINTER_OP,         // * (for signaling a pointer)
    TOKEN_ARROW_OP,           // -> (for accessing a pointer)
    TOKEN_QUANTUM_POINTER_OP, // *| (special operator)

    /* --- Literals --- */
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER_INT,   // 10
    TOKEN_NUMBER_FLOAT, // 10.1
    TOKEN_STRING,       // "string" or 'string'

    /* --- Data Types --- */
    TOKEN_TYPE_INT,
    TOKEN_TYPE_FLOAT,
    TOKEN_TYPE_CHAR,
    TOKEN_TYPE_BOOL,
    TOKEN_TYPE_STRING,

    /* --- Keywords --- */
    TOKEN_KW_CONST,
    TOKEN_KW_IF,
    TOKEN_KW_ELSE,
    TOKEN_KW_ELIF,
    TOKEN_KW_SWITCH,
    TOKEN_KW_CASE,
    TOKEN_KW_DEFAULT,
    TOKEN_KW_ASSIGN,
    TOKEN_KW_FOR,
    TOKEN_KW_WHILE,
    TOKEN_KW_DO,
    TOKEN_KW_BREAK,
    TOKEN_KW_CONTINUE,
    TOKEN_KW_ASK,
    TOKEN_KW_DISPLAY,
    TOKEN_KW_TRUE,
    TOKEN_KW_FALSE,
    TOKEN_KW_FN,
    TOKEN_KW_STRUCT,

    /* --- Reserved Words --- */
    TOKEN_RW_EXECUTE,
    TOKEN_RW_EXIT,
    TOKEN_RW_FETCH,
    TOKEN_RW_WHEN,
    TOKEN_RW_OTHERWISE,
    TOKEN_RW_AUTO_REF,

    /* --- COMMENTS --- */
    TOKEN_SINGLE_COMMENT,
    TOKEN_MULTI_COMMENT,

    /* --- Special Tokens --- */
    TOKEN_ERROR,
    TOKEN_EOF
} TokenType;

/* Token Structure */
typedef struct
{
    TokenType type;           /* Kind of token it is*/
    const char *lexeme_start; /* Start of lexeme in source */
    int lexeme_length;        /* Length of the lexeme */
    int line;                 /* Line of the code the lexeme is found */
} Token;

/* ========== GLOBAL VARIABLES ========== */

/* Scanner state */
typedef struct
{
    const char *source_start; /* Start of entire source code */
    const char *token_start;  /* Start of current token being scanned */
    const char *scan_ptr;     /* Current scanning position */
    int line_number;          /* Current line number */
} Scanner;

Scanner scanner;

/* Keywords mapping structure */
typedef struct
{
    const char *keyword;
    TokenType token_type;
} KeywordMapping;

/* Keywords and Reserved words list with their specific token types */
KeywordMapping keywords[] = {
    {"int", TOKEN_TYPE_INT},
    {"float", TOKEN_TYPE_FLOAT},
    {"char", TOKEN_TYPE_CHAR},
    {"bool", TOKEN_TYPE_BOOL},
    {"string", TOKEN_TYPE_STRING},
    {"const", TOKEN_KW_CONST},
    {"if", TOKEN_KW_IF},
    {"else", TOKEN_KW_ELSE},
    {"elif", TOKEN_KW_ELIF},
    {"switch", TOKEN_KW_SWITCH},
    {"case", TOKEN_KW_CASE},
    {"default", TOKEN_KW_DEFAULT},
    {"assign", TOKEN_KW_ASSIGN},
    {"struct", TOKEN_KW_STRUCT},
    {"for", TOKEN_KW_FOR},
    {"while", TOKEN_KW_WHILE},
    {"do", TOKEN_KW_DO},
    {"break", TOKEN_KW_BREAK},
    {"continue", TOKEN_KW_CONTINUE},
    {"ask", TOKEN_KW_ASK},
    {"display", TOKEN_KW_DISPLAY},
    {"execute", TOKEN_RW_EXECUTE},
    {"exit", TOKEN_RW_EXIT},
    {"true", TOKEN_KW_TRUE},
    {"false", TOKEN_KW_FALSE},
    {"fetch", TOKEN_RW_FETCH},
    {"fn", TOKEN_KW_FN},
    {"when", TOKEN_RW_WHEN},
    {"otherwise", TOKEN_RW_OTHERWISE},
    {"auto_ref", TOKEN_RW_AUTO_REF}};
int keywordCount = 30;

/* Original keyword list (kept for compatibility) */
// char *keywordList[] = {
//     "int", "float", "char", "bool", "string",
//     "const", "if", "else", "elif", "switch",
//     "case", "default", "assign", "struct", "for", "while",
//     "do", "break", "continue", "ask", "display",
//     "execute", "exit", "true", "false", "fetch",
//     "fn", "when", "otherwise", "auto_ref"};

/* ========== CHARACTER UTILITY FUNCTIONS ========== */

int isLetter(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

int isDigit(char c)
{
    return c >= '0' && c <= '9';
}

int isWhitespace(char c)
{
    switch (c)
    {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
        return 1;
    default:
        return 0;
    }
}

int isOperator(char c)
{
    switch (c)
    {
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '=':
    case '<':
    case '>':
    case '!':
    case '&':
    case '|':
    case '^':
    case '~':
        return 1;
    default:
        return 0;
    }
}

int isSeparator(char c)
{
    switch (c)
    {
    case ',':
    case ';':
    case ':':
    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
        return 1;
    default:
        return 0;
    }
}

/* ========== SCANNER HELPER FUNCTIONS ========== */

/* Check if we've reached the end of source */
int reachedEnd()
{
    return *scanner.scan_ptr == '\0';
}

/* Get current character and move forward */
char consumeChar()
{
    scanner.scan_ptr++;
    return scanner.scan_ptr[-1];
}

/* Look at current character without consuming */
char currentChar()
{
    return *scanner.scan_ptr;
}

/* Look at next character without consuming */
char nextChar()
{
    if (reachedEnd())
        return '\0';
    return scanner.scan_ptr[1];
}

/* Check if next char matches, consume if true */
int matchNext(char expected)
{
    if (reachedEnd())
        return 0;
    if (*scanner.scan_ptr != expected)
        return 0;
    scanner.scan_ptr++;
    return 1;
}

/* ========== TOKEN CREATION FUNCTIONS ========== */

/* Create a token with current lexeme */
Token createToken(TokenType type)
{
    Token tok;
    tok.type = type;
    tok.lexeme_start = scanner.token_start;
    tok.lexeme_length = (int)(scanner.scan_ptr - scanner.token_start);
    tok.line = scanner.line_number;
    return tok;
}

/* Create an error token with message */
Token createErrorToken(const char *message)
{
    Token tok;
    tok.type = TOKEN_ERROR;
    tok.lexeme_start = message;
    tok.lexeme_length = (int)strlen(message);
    tok.line = scanner.line_number;
    return tok;
}

/* ========== TOKEN UTILITY FUNCTIONS ========== */

TokenType getKeywordType(const char *str, int length)
{
    for (int i = 0; i < keywordCount; i++)
    {
        if (strlen(keywords[i].keyword) == length &&
            memcmp(str, keywords[i].keyword, length) == 0)
        {
            return keywords[i].token_type;
        }
    }
    return TOKEN_IDENTIFIER; /* Not a keyword, it's an identifier */
}

const char *getTokenTypeName(TokenType type)
{
    switch (type)
    {
    /* Separators */
    case TOKEN_COMMA:
        return "COMMA";
    case TOKEN_SEMICOLON:
        return "SEMICOLON";
    case TOKEN_COLON:
        return "COLON";
    case TOKEN_L_PAREN:
        return "L_PAREN";
    case TOKEN_R_PAREN:
        return "R_PAREN";
    case TOKEN_L_BRACE:
        return "L_BRACE";
    case TOKEN_R_BRACE:
        return "R_BRACE";
    case TOKEN_L_BRACKET:
        return "L_BRACKET";
    case TOKEN_R_BRACKET:
        return "R_BRACKET";

    /* Arithmetic Operators */
    case TOKEN_ADD_OP:
        return "ADD_OP";
    case TOKEN_SUB_OP:
        return "SUB_OP";
    case TOKEN_MULT_OP:
        return "MULT_OP";
    case TOKEN_DIV_OP:
        return "DIV_OP";
    case TOKEN_MOD_OP:
        return "MOD_OP";
    case TOKEN_EXPO_OP:
        return "EXPO_OP";

    /* Assignment Operators */
    case TOKEN_ASSIGN_OP:
        return "ASSIGN_OP";
    case TOKEN_ADD_ASSIGN_OP:
        return "ADD_ASSIGN_OP";
    case TOKEN_SUB_ASSIGN_OP:
        return "SUB_ASSIGN_OP";
    case TOKEN_MULT_ASSIGN_OP:
        return "MULT_ASSIGN_OP";
    case TOKEN_DIV_ASSIGN_OP:
        return "DIV_ASSIGN_OP";
    case TOKEN_MOD_ASSIGN_OP:
        return "MOD_ASSIGN_OP";

    /* Unary Operators */
    case TOKEN_ADDRESS_OF_OP:
        return "ADDRESS_OF_OP";
    case TOKEN_INCREMENT_OP:
        return "INCREMENT_OP";
    case TOKEN_DECREMENT_OP:
        return "DECREMENT_OP";

    /* Relational Operators */
    case TOKEN_EQUAL_TO_OP:
        return "EQUAL_TO_OP";
    case TOKEN_NOT_EQUAL_TO_OP:
        return "NOT_EQUAL_TO_OP";
    case TOKEN_GREATER_OP:
        return "GREATER_OP";
    case TOKEN_GREATER_EQUAL_OP:
        return "GREATER_EQUAL_OP";
    case TOKEN_LESS_OP:
        return "LESS_OP";
    case TOKEN_LESS_EQUAL_OP:
        return "LESS_EQUAL_OP";

    /* Logical Operators */
    case TOKEN_LOGICAL_NOT_OP:
        return "LOGICAL_NOT_OP";
    case TOKEN_LOGICAL_AND_OP:
        return "LOGICAL_AND_OP";
    case TOKEN_LOGICAL_OR_OP:
        return "LOGICAL_OR_OP";

    /* Pointer Structure Operator */
    case TOKEN_POINTER_OP:
        return "POINTER_OP";
    case TOKEN_ARROW_OP:
        return "ARROW_OP";
    case TOKEN_QUANTUM_POINTER_OP:
        return "QUANTUM_POINTER_OP";

    /* Literals */
    case TOKEN_IDENTIFIER:
        return "IDENTIFIER";
    case TOKEN_NUMBER_INT:
        return "NUMBER_INT";
    case TOKEN_NUMBER_FLOAT:
        return "NUMBER_FLOAT";
    case TOKEN_STRING:
        return "STRING";

    /* Data Types */
    case TOKEN_TYPE_INT:
        return "TYPE_INT";
    case TOKEN_TYPE_FLOAT:
        return "TYPE_FLOAT";
    case TOKEN_TYPE_CHAR:
        return "TYPE_CHAR";
    case TOKEN_TYPE_BOOL:
        return "TYPE_BOOL";
    case TOKEN_TYPE_STRING:
        return "TYPE_STRING";

    /* Keywords */
    case TOKEN_KW_CONST:
        return "KW_CONST";
    case TOKEN_KW_IF:
        return "KW_IF";
    case TOKEN_KW_ELSE:
        return "KW_ELSE";
    case TOKEN_KW_ELIF:
        return "KW_ELIF";
    case TOKEN_KW_SWITCH:
        return "KW_SWITCH";
    case TOKEN_KW_CASE:
        return "KW_CASE";
    case TOKEN_KW_DEFAULT:
        return "KW_DEFAULT";
    case TOKEN_KW_ASSIGN:
        return "KW_ASSIGN";
    case TOKEN_KW_FOR:
        return "KW_FOR";
    case TOKEN_KW_WHILE:
        return "KW_WHILE";
    case TOKEN_KW_DO:
        return "KW_DO";
    case TOKEN_KW_BREAK:
        return "KW_BREAK";
    case TOKEN_KW_CONTINUE:
        return "KW_CONTINUE";
    case TOKEN_KW_ASK:
        return "KW_ASK";
    case TOKEN_KW_DISPLAY:
        return "KW_DISPLAY";
    case TOKEN_KW_TRUE:
        return "KW_TRUE";
    case TOKEN_KW_FALSE:
        return "KW_FALSE";
    case TOKEN_KW_FN:
        return "KW_FN";
    case TOKEN_KW_STRUCT:
        return "KW_STRUCT";

    /* Reserved Words */
    case TOKEN_RW_EXECUTE:
        return "RW_EXECUTE";
    case TOKEN_RW_EXIT:
        return "RW_EXIT";
    case TOKEN_RW_FETCH:
        return "RW_FETCH";
    case TOKEN_RW_WHEN:
        return "RW_WHEN";
    case TOKEN_RW_OTHERWISE:
        return "RW_OTHERWISE";
    case TOKEN_RW_AUTO_REF:
        return "RW_AUTO_REF";

    /* Comments */
    case TOKEN_SINGLE_COMMENT:
        return "SINGLE_COMMENT";
    case TOKEN_MULTI_COMMENT:
        return "MULTI_COMMENT";

    /* Special */
    case TOKEN_ERROR:
        return "ERROR";
    case TOKEN_EOF:
        return "EOF";

    default:
        return "UNKNOWN";
    }
}

void printToken(Token t)
{
    /* Print in table format using pointer and length */
    printf("%-6d | %-20s | %.*s\n", t.line, getTokenTypeName(t.type),
           t.lexeme_length, t.lexeme_start);
}

void initScanner(const char *source)
{
    scanner.source_start = source;
    scanner.token_start = source;
    scanner.scan_ptr = source;
    scanner.line_number = 1;
}

void resetScanner()
{
    scanner.line_number = 1;
}

/* ========== LEXICAL ANALYZER ========== */

/* Skip all whitespace and track line numbers */
void skipWhitespaceAndNewlines()
{
    while (!reachedEnd())
    {
        char c = currentChar();
        if (c == ' ' || c == '\t' || c == '\r')
        {
            consumeChar();
        }
        else if (c == '\n')
        {
            scanner.line_number++;
            consumeChar();
        }
        else
        {
            break;
        }
    }
}

/* Scan identifier or keyword */
Token scanIdentifier()
{
    while (isLetter(currentChar()) || isDigit(currentChar()) || currentChar() == '_')
    {
        consumeChar();
    }

    /* Check if it's a keyword and return specific token type */
    int len = (int)(scanner.scan_ptr - scanner.token_start);
    TokenType type = getKeywordType(scanner.token_start, len);
    return createToken(type);
}

/* Scan number (integer or decimal) */
Token scanNumber()
{
    int isFloat = 0;

    while (isDigit(currentChar()))
    {
        consumeChar();
    }

    /* Check for decimal point */
    if (currentChar() == '.' && isDigit(nextChar()))
    {
        isFloat = 1;
        consumeChar(); /* consume the dot */
        while (isDigit(currentChar()))
        {
            consumeChar();
        }
    }

    return createToken(isFloat ? TOKEN_NUMBER_FLOAT : TOKEN_NUMBER_INT);
}

/* Scan string literal */
Token scanString(char quote)
{
    while (currentChar() != quote && !reachedEnd())
    {
        if (currentChar() == '\n')
            scanner.line_number++;
        consumeChar();
    }

    if (reachedEnd())
    {
        return createErrorToken("Unterminated string");
    }

    consumeChar(); /* closing quote */
    return createToken(TOKEN_STRING);
}

/* Scan single-line comment */
Token scanSingleLineComment()
{
    while (currentChar() != '\n' && !reachedEnd())
    {
        consumeChar();
    }
    return createToken(TOKEN_SINGLE_COMMENT);
}

/* Scan multi-line comment */
Token scanMultiLineComment()
{
    while (!reachedEnd())
    {
        if (currentChar() == '*' && nextChar() == '/')
        {
            consumeChar(); /* consume * */
            consumeChar(); /* consume / */
            return createToken(TOKEN_MULTI_COMMENT);
        }
        if (currentChar() == '\n')
        {
            scanner.line_number++;
        }
        consumeChar();
    }
    return createErrorToken("Unterminated comment");
}

/* Main token scanner */
Token getNextToken()
{
    /* Skip whitespace */
    skipWhitespaceAndNewlines();

    /* Mark start of new token */
    scanner.token_start = scanner.scan_ptr;

    /* Check for EOF */
    if (reachedEnd())
    {
        return createToken(TOKEN_EOF);
    }

    /* Peek at current character without consuming */
    char c = currentChar();

    /* Check for letters - identifiers or keywords */
    if (isLetter(c) || c == '_')
    {
        consumeChar();
        return scanIdentifier();
    }

    /* Check for digits - numbers */
    if (isDigit(c))
    {
        consumeChar();
        return scanNumber();
    }

    /* Now consume the character for operator/separator checking */
    c = consumeChar();

    /* Check for operators and separators with specific token types */
    switch (c)
    {
    case '+':
        if (matchNext('+'))
            return createToken(TOKEN_INCREMENT_OP);
        if (matchNext('='))
            return createToken(TOKEN_ADD_ASSIGN_OP);
        return createToken(TOKEN_ADD_OP);

    case '-':
        if (matchNext('-'))
            return createToken(TOKEN_DECREMENT_OP);
        if (matchNext('='))
            return createToken(TOKEN_SUB_ASSIGN_OP);
        if (matchNext('>'))
            return createToken(TOKEN_ARROW_OP);
        return createToken(TOKEN_SUB_OP);

    case '*':
        /* Check for *| (quantum pointer operator) */
        if (currentChar() == '|')
        {
            consumeChar(); /* consume the | */
            return createToken(TOKEN_QUANTUM_POINTER_OP);
        }
        /* Check for *= (multiply assign) */
        if (currentChar() == '=')
        {
            consumeChar(); /* consume the = */
            return createToken(TOKEN_MULT_ASSIGN_OP);
        }
        /* Check if it's likely a pointer: * NOT followed by whitespace and followed by identifier
         * This catches cases like: int *ptr, int*ptr
         * But allows: a * b (with spaces) to be multiplication */
        if (!isWhitespace(currentChar()) && (isLetter(currentChar()) || currentChar() == '_' || currentChar() == '*'))
        {
            return createToken(TOKEN_POINTER_OP);
        }
        /* Otherwise it's multiplication */
        return createToken(TOKEN_MULT_OP);

    case '/':
        if (matchNext('='))
        {
            return createToken(TOKEN_DIV_ASSIGN_OP);
        }
        else if (matchNext('/'))
        {
            return scanSingleLineComment();
        }
        else if (matchNext('*'))
        {
            return scanMultiLineComment();
        }
        return createToken(TOKEN_DIV_OP);

    case '%':
        if (matchNext('='))
            return createToken(TOKEN_MOD_ASSIGN_OP);
        return createToken(TOKEN_MOD_OP);

    case '=':
        if (matchNext('='))
            return createToken(TOKEN_EQUAL_TO_OP);
        return createToken(TOKEN_ASSIGN_OP);

    case '<':
        if (matchNext('='))
            return createToken(TOKEN_LESS_EQUAL_OP);
        return createToken(TOKEN_LESS_OP);

    case '>':
        if (matchNext('='))
            return createToken(TOKEN_GREATER_EQUAL_OP);
        return createToken(TOKEN_GREATER_OP);

    case '!':
        if (matchNext('='))
            return createToken(TOKEN_NOT_EQUAL_TO_OP);
        return createToken(TOKEN_LOGICAL_NOT_OP);

    case '&':
        if (matchNext('&'))
            return createToken(TOKEN_LOGICAL_AND_OP);
        return createToken(TOKEN_ADDRESS_OF_OP);

    case '|':
        if (matchNext('|'))
            return createToken(TOKEN_LOGICAL_OR_OP);
        return createErrorToken("Unexpected character '|'");

    case '^':
        return createToken(TOKEN_EXPO_OP);

    case '~':
        return createErrorToken("Unexpected character '~'");

    /* Separators */
    case ',':
        return createToken(TOKEN_COMMA);
    case ';':
        return createToken(TOKEN_SEMICOLON);
    case ':':
        return createToken(TOKEN_COLON);
    case '(':
        return createToken(TOKEN_L_PAREN);
    case ')':
        return createToken(TOKEN_R_PAREN);
    case '{':
        return createToken(TOKEN_L_BRACE);
    case '}':
        return createToken(TOKEN_R_BRACE);
    case '[':
        return createToken(TOKEN_L_BRACKET);
    case ']':
        return createToken(TOKEN_R_BRACKET);

    /* Strings */
    case '"':
        return scanString('"');
    case '\'':
        return scanString('\'');

    /* Unknown character */
    default:
        return createErrorToken("Unexpected character");
    }
}

/* Analyze code from string */
void analyzeCode(const char *code)
{
    Token token;

    /* Initialize scanner */
    initScanner(code);

    printf(">>> LEXICAL ANALYSIS RESULTS:\n");
    printf("--------------------------------------------------\n");

    /* Print table header */
    printf("LINE   | TOKEN TYPE           | LEXEME\n");
    printf("-------|----------------------|---------------------------\n");

    /* Process all tokens including EOF */
    do
    {
        token = getNextToken();
        printToken(token);
    } while (token.type != TOKEN_EOF);
}

/* ========== MAIN PROGRAM ========== */

int main()
{
    char *input = NULL;
    size_t buffer_size = 0;
    size_t total_size = 0;
    size_t chunk_size = 1024;

    /* Allocate initial buffer */
    input = (char *)malloc(chunk_size);
    if (input == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }

    /* Read all input from stdin */
    size_t bytes_read;
    while ((bytes_read = fread(input + total_size, 1, chunk_size, stdin)) > 0)
    {
        total_size += bytes_read;

        /* If buffer is full, expand it */
        if (total_size + chunk_size > buffer_size + chunk_size)
        {
            buffer_size = total_size + chunk_size;
            char *new_input = (char *)realloc(input, buffer_size);
            if (new_input == NULL)
            {
                fprintf(stderr, "Error: Memory reallocation failed\n");
                free(input);
                return 1;
            }
            input = new_input;
        }
    }

    /* Null-terminate the string */
    input[total_size] = '\0';

    /* Check if we got any input */
    if (total_size == 0)
    {
        printf("Error: No input provided\n");
        free(input);
        return 1;
    }

    /* Analyze the input code */
    analyzeCode(input);

    /* Clean up */
    free(input);

    return 0;
}