#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif


typedef struct {
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

static void Parser_ErrorAt(Token *token, const char *message);
static void Parser_Advance();
static void Parser_Consume(TokenType type, const char *message);
static void Parser_End();
static void Parser_CompileNumber();
static void Parser_CompileGrouping();
static void Parser_CompileExpression();
static void Parser_CompileUnary();
static void Parser_CompileBinary();


typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY
} Precedence;


typedef void (*ParseFn)();


typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;


Parser parser;
Chunk *compiling_chunk;
ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {Parser_CompileGrouping, NULL,                 PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,                   NULL,                 PREC_NONE}, 
  [TOKEN_RIGHT_BRACE]   = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_COMMA]         = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_DOT]           = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_MINUS]         = {Parser_CompileUnary,    Parser_CompileBinary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,                   Parser_CompileBinary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_SLASH]         = {NULL,                   Parser_CompileBinary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,                   Parser_CompileBinary, PREC_FACTOR},
  [TOKEN_BANG]          = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_EQUAL]         = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_GREATER]       = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_GREATER_EQUAL] = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_LESS]          = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_LESS_EQUAL]    = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_IDENTIFIER]    = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_STRING]        = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_NUMBER]        = {Parser_CompileNumber,   NULL,                 PREC_NONE},
  [TOKEN_AND]           = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_CLASS]         = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_ELSE]          = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_FALSE]         = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_FOR]           = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_FUN]           = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_IF]            = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_NIL]           = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_OR]            = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_PRINT]         = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_RETURN]        = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_SUPER]         = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_THIS]          = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_TRUE]          = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_VAR]           = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_WHILE]         = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_ERROR]         = {NULL,                   NULL,                 PREC_NONE},
  [TOKEN_EOF]           = {NULL,                   NULL,                 PREC_NONE},
};


static ParseRule *ParseRule_Get(TokenType type)
{
    return &rules[type];
}


static Chunk *get_current_chunk()
{
    return compiling_chunk;
}


bool compile(const char *source, Chunk *chunk)
{
    Scanner_Init(source);
    compiling_chunk = chunk;
    
    parser.had_error = false;
    parser.panic_mode = false;

    Parser_Advance();
    Parser_CompileExpression();
    Parser_Consume(TOKEN_EOF, "Expect end of expression.");
    Parser_End();

    return !parser.had_error;
}


static void Parser_ErrorAtCurrent(const char *message)
{
    Parser_ErrorAt(&parser.current, message);
}


static void Parser_Error(const char *message)
{
    Parser_ErrorAt(&parser.previous, message);
}


static void Parser_ErrorAt(Token *token, const char *message)
{
    if (parser.panic_mode) {
        return;
    }

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // nothing
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.had_error = true;
}


static void Parser_Advance()
{
    parser.previous = parser.current;

    for (;;) {
        parser.current = Scanner_GetToken();
        if (parser.current.type != TOKEN_ERROR) {
            break;
        }

        Parser_ErrorAtCurrent(parser.current.start);
    }
}


static void Parser_Consume(TokenType type, const char *message)
{
    if (parser.current.type == type) {
        Parser_Advance();
        return;
    }

    Parser_ErrorAtCurrent(message);
}


static void Parser_EmitByte(uint8_t byte)
{
    Chunk_Write(get_current_chunk(), byte, parser.previous.line);
}


static void Parser_EmitReturn()
{
    Parser_EmitByte(OP_RETURN);
}


static void Parser_End()
{
    Parser_EmitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!parser.had_error) {
        Chunk_Disassemble(get_current_chunk(), "code");
    }
#endif
}


static void Parser_EmitBytes(uint8_t byte1, uint8_t byte2)
{
    Parser_EmitByte(byte1);
    Parser_EmitByte(byte2);
}


static void Parser_EmitConstant(Value value)
{
    Chunk_WriteConstant(get_current_chunk(), value, parser.previous.line);
}


static void Parser_ParsePrecedence(Precedence precedence)
{
    ParseFn prefix_rule, infix_rule;

    Parser_Advance();
    prefix_rule = ParseRule_Get(parser.previous.type)->prefix;
    if (prefix_rule == NULL) {
        Parser_Error("Expect expression.");
        return;
    }

    prefix_rule();

    while (precedence <= ParseRule_Get(parser.current.type)->precedence) {
        Parser_Advance();
        infix_rule = ParseRule_Get(parser.previous.type)->infix;
        infix_rule();
    }
}


static void Parser_CompileExpression()
{
    Parser_ParsePrecedence(PREC_ASSIGNMENT);
}


static void Parser_CompileNumber()
{
    double value = strtod(parser.previous.start, NULL);
    Parser_EmitConstant(value);
}


static void Parser_CompileGrouping()
{
    Parser_CompileExpression();
    Parser_Consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}


static void Parser_CompileUnary()
{
    TokenType operator_type;

    operator_type = parser.previous.type;
    Parser_ParsePrecedence(PREC_UNARY);

    switch (operator_type) {
        case TOKEN_MINUS:
            Parser_EmitByte(OP_NEGATE);
            break;
        default:
            return;
    }
}


static void Parser_CompileBinary()
{
    TokenType operator_type;
    ParseRule *rule;

    operator_type = parser.previous.type;
    rule = ParseRule_Get(operator_type);
    Parser_ParsePrecedence((Precedence)(rule->precedence + 1));

    switch (operator_type) {
        case TOKEN_PLUS:
            Parser_EmitByte(OP_ADD);
            break;
        case TOKEN_MINUS:
            Parser_EmitByte(OP_SUBSTRACT);
            break;
        case TOKEN_STAR:
            Parser_EmitByte(OP_MULTIPLY);
            break;
        case TOKEN_SLASH:
            Parser_EmitByte(OP_DIVIDE);
            break;
        default:
            return;
    }
}
