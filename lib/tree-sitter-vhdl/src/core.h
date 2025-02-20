#ifndef core_h
#define core_h
//------------------------------------------------------------------------------

#include "TokenType.h"
//------------------------------------------------------------------------------

static void register_reserved(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "abs",           RESERVED_ABS);
    token_tree_insert(token_tree, "access",        RESERVED_ACCESS);
    token_tree_insert(token_tree, "after",         RESERVED_AFTER);
    token_tree_insert(token_tree, "alias",         RESERVED_ALIAS);
    token_tree_insert(token_tree, "all",           RESERVED_ALL);
    token_tree_insert(token_tree, "and",           RESERVED_AND);
    token_tree_insert(token_tree, "architecture",  RESERVED_ARCHITECTURE);
    token_tree_insert(token_tree, "array",         RESERVED_ARRAY);
    token_tree_insert(token_tree, "assert",        RESERVED_ASSERT);
    token_tree_insert(token_tree, "assume",        RESERVED_ASSUME);
    token_tree_insert(token_tree, "attribute",     RESERVED_ATTRIBUTE);
    token_tree_insert(token_tree, "begin",         RESERVED_BEGIN);
    token_tree_insert(token_tree, "block",         RESERVED_BLOCK);
    token_tree_insert(token_tree, "body",          RESERVED_BODY);
    token_tree_insert(token_tree, "buffer",        RESERVED_BUFFER);
    token_tree_insert(token_tree, "bus",           RESERVED_BUS);
    token_tree_insert(token_tree, "case",          RESERVED_CASE);
    token_tree_insert(token_tree, "component",     RESERVED_COMPONENT);
    token_tree_insert(token_tree, "configuration", RESERVED_CONFIGURATION);
    token_tree_insert(token_tree, "constant",      RESERVED_CONSTANT);
    token_tree_insert(token_tree, "context",       RESERVED_CONTEXT);
    token_tree_insert(token_tree, "cover",         RESERVED_COVER);
    token_tree_insert(token_tree, "default",       RESERVED_DEFAULT);
    token_tree_insert(token_tree, "disconnect",    RESERVED_DISCONNECT);
    token_tree_insert(token_tree, "downto",        RESERVED_DOWNTO);
    token_tree_insert(token_tree, "else",          RESERVED_ELSE);
    token_tree_insert(token_tree, "elsif",         RESERVED_ELSIF);
    token_tree_insert(token_tree, "end",           RESERVED_END);
    token_tree_insert(token_tree, "entity",        RESERVED_ENTITY);
    token_tree_insert(token_tree, "exit",          RESERVED_EXIT);
    token_tree_insert(token_tree, "fairness",      RESERVED_FAIRNESS);
    token_tree_insert(token_tree, "file",          RESERVED_FILE);
    token_tree_insert(token_tree, "for",           RESERVED_FOR);
    token_tree_insert(token_tree, "force",         RESERVED_FORCE);
    token_tree_insert(token_tree, "function",      RESERVED_FUNCTION);
    token_tree_insert(token_tree, "generate",      RESERVED_GENERATE);
    token_tree_insert(token_tree, "generic",       RESERVED_GENERIC);
    token_tree_insert(token_tree, "group",         RESERVED_GROUP);
    token_tree_insert(token_tree, "guarded",       RESERVED_GUARDED);
    token_tree_insert(token_tree, "if",            RESERVED_IF);
    token_tree_insert(token_tree, "impure",        RESERVED_IMPURE);
    token_tree_insert(token_tree, "in",            RESERVED_IN);
    token_tree_insert(token_tree, "inertial",      RESERVED_INERTIAL);
    token_tree_insert(token_tree, "inout",         RESERVED_INOUT);
    token_tree_insert(token_tree, "is",            RESERVED_IS);
    token_tree_insert(token_tree, "label",         RESERVED_LABEL);
    token_tree_insert(token_tree, "library",       RESERVED_LIBRARY);
    token_tree_insert(token_tree, "linkage",       RESERVED_LINKAGE);
    token_tree_insert(token_tree, "literal",       RESERVED_LITERAL);
    token_tree_insert(token_tree, "loop",          RESERVED_LOOP);
    token_tree_insert(token_tree, "map",           RESERVED_MAP);
    token_tree_insert(token_tree, "mod",           RESERVED_MOD);
    token_tree_insert(token_tree, "nand",          RESERVED_NAND);
    token_tree_insert(token_tree, "new",           RESERVED_NEW);
    token_tree_insert(token_tree, "next",          RESERVED_NEXT);
    token_tree_insert(token_tree, "nor",           RESERVED_NOR);
    token_tree_insert(token_tree, "not",           RESERVED_NOT);
    token_tree_insert(token_tree, "null",          RESERVED_NULL);
    token_tree_insert(token_tree, "of",            RESERVED_OF);
    token_tree_insert(token_tree, "on",            RESERVED_ON);
    token_tree_insert(token_tree, "open",          RESERVED_OPEN);
    token_tree_insert(token_tree, "or",            RESERVED_OR);
    token_tree_insert(token_tree, "others",        RESERVED_OTHERS);
    token_tree_insert(token_tree, "out",           RESERVED_OUT);
    token_tree_insert(token_tree, "package",       RESERVED_PACKAGE);
    token_tree_insert(token_tree, "parameter",     RESERVED_PARAMETER);
    token_tree_insert(token_tree, "port",          RESERVED_PORT);
    token_tree_insert(token_tree, "postponed",     RESERVED_POSTPONED);
    token_tree_insert(token_tree, "procedure",     RESERVED_PROCEDURE);
    token_tree_insert(token_tree, "process",       RESERVED_PROCESS);
    token_tree_insert(token_tree, "property",      RESERVED_PROPERTY);
    token_tree_insert(token_tree, "protected",     RESERVED_PROTECTED);
    token_tree_insert(token_tree, "private",       RESERVED_PRIVATE);
    token_tree_insert(token_tree, "pure",          RESERVED_PURE);
    token_tree_insert(token_tree, "range",         RESERVED_RANGE);
    token_tree_insert(token_tree, "record",        RESERVED_RECORD);
    token_tree_insert(token_tree, "register",      RESERVED_REGISTER);
    token_tree_insert(token_tree, "reject",        RESERVED_REJECT);
    token_tree_insert(token_tree, "release",       RESERVED_RELEASE);
    token_tree_insert(token_tree, "rem",           RESERVED_REM);
    token_tree_insert(token_tree, "report",        RESERVED_REPORT);
    token_tree_insert(token_tree, "restrict",      RESERVED_RESTRICT);
    token_tree_insert(token_tree, "return",        RESERVED_RETURN);
    token_tree_insert(token_tree, "rol",           RESERVED_ROL);
    token_tree_insert(token_tree, "ror",           RESERVED_ROR);
    token_tree_insert(token_tree, "select",        RESERVED_SELECT);
    token_tree_insert(token_tree, "sequence",      RESERVED_SEQUENCE);
    token_tree_insert(token_tree, "severity",      RESERVED_SEVERITY);
    token_tree_insert(token_tree, "signal",        RESERVED_SIGNAL);
    token_tree_insert(token_tree, "shared",        RESERVED_SHARED);
    token_tree_insert(token_tree, "sla",           RESERVED_SLA);
    token_tree_insert(token_tree, "sll",           RESERVED_SLL);
    token_tree_insert(token_tree, "sra",           RESERVED_SRA);
    token_tree_insert(token_tree, "srl",           RESERVED_SRL);
    token_tree_insert(token_tree, "strong",        RESERVED_STRONG);
    token_tree_insert(token_tree, "subtype",       RESERVED_SUBTYPE);
    token_tree_insert(token_tree, "then",          RESERVED_THEN);
    token_tree_insert(token_tree, "to",            RESERVED_TO);
    token_tree_insert(token_tree, "transport",     RESERVED_TRANSPORT);
    token_tree_insert(token_tree, "type",          RESERVED_TYPE);
    token_tree_insert(token_tree, "unaffected",    RESERVED_UNAFFECTED);
    token_tree_insert(token_tree, "units",         RESERVED_UNITS);
    token_tree_insert(token_tree, "until",         RESERVED_UNTIL);
    token_tree_insert(token_tree, "use",           RESERVED_USE);
    token_tree_insert(token_tree, "variable",      RESERVED_VARIABLE);
    token_tree_insert(token_tree, "view",          RESERVED_VIEW);
    token_tree_insert(token_tree, "vmode",         RESERVED_VMODE);
    token_tree_insert(token_tree, "vpkg",          RESERVED_VPKG);
    token_tree_insert(token_tree, "vprop",         RESERVED_VPROP);
    token_tree_insert(token_tree, "vunit",         RESERVED_VUNIT);
    token_tree_insert(token_tree, "wait",          RESERVED_WAIT);
    token_tree_insert(token_tree, "when",          RESERVED_WHEN);
    token_tree_insert(token_tree, "while",         RESERVED_WHILE);
    token_tree_insert(token_tree, "with",          RESERVED_WITH);
    token_tree_insert(token_tree, "xnor",          RESERVED_XNOR);
    token_tree_insert(token_tree, "xor",           RESERVED_XOR);
}
//------------------------------------------------------------------------------

static void register_directives(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "protect",          DIRECTIVE_PROTECT);
    token_tree_insert(token_tree, "warning",          DIRECTIVE_WARNING);
    token_tree_insert(token_tree, "error",            DIRECTIVE_ERROR);

    token_tree_insert(token_tree, "VHDL_VERSION",     DIRECTIVE_CONSTANT_BUILTIN);
    token_tree_insert(token_tree, "TOOL_TYPE",        DIRECTIVE_CONSTANT_BUILTIN);
    token_tree_insert(token_tree, "TOOL_VENDOR",      DIRECTIVE_CONSTANT_BUILTIN);
    token_tree_insert(token_tree, "TOOL_NAME",        DIRECTIVE_CONSTANT_BUILTIN);
    token_tree_insert(token_tree, "TOOL_EDITION",     DIRECTIVE_CONSTANT_BUILTIN);
    token_tree_insert(token_tree, "TOOL_VERSION",     DIRECTIVE_CONSTANT_BUILTIN);

    token_tree_insert(token_tree, "\r",               DIRECTIVE_NEWLINE);
    token_tree_insert(token_tree, "\n",               DIRECTIVE_NEWLINE);
    token_tree_insert(token_tree, "\r\n",             DIRECTIVE_NEWLINE);
    token_tree_insert(token_tree, "\n\r",             DIRECTIVE_NEWLINE);
}
//------------------------------------------------------------------------------

static void register_delimiters(TokenTree* token_tree)
{
    // token_tree_insert(token_tree, "&",   DELIMITER_AMPERSAND);
    // token_tree_insert(token_tree, "'",   DELIMITER_TICK);
    // token_tree_insert(token_tree, "(",   DELIMITER_LEFT_PARENTHESIS);
    // token_tree_insert(token_tree, ")",   DELIMITER_RIGHT_PARENTHESIS);
    // token_tree_insert(token_tree, "*",   DELIMITER_MULTIPLY);
    // token_tree_insert(token_tree, "+",   DELIMITER_PLUS_SIGN);
    // token_tree_insert(token_tree, ",",   DELIMITER_COMMA);
    // token_tree_insert(token_tree, "-",   DELIMITER_MINUS_SIGN);
    // token_tree_insert(token_tree, ".",   DELIMITER_DOT);
    // token_tree_insert(token_tree, "/",   DELIMITER_DIVIDE);
    // token_tree_insert(token_tree, ":",   DELIMITER_COLON);
    // token_tree_insert(token_tree, ";",   DELIMITER_SEMICOLON);
    // token_tree_insert(token_tree, "<",   DELIMITER_LESS_THAN_SIGN);
    // token_tree_insert(token_tree, "=",   DELIMITER_EQUALS_SIGN);
    // token_tree_insert(token_tree, ">",   DELIMITER_GREATER_THAN_SIGN);
    token_tree_insert(token_tree, "`",   DELIMITER_GRAVE_ACCENT);
    // token_tree_insert(token_tree, "|",   DELIMITER_VERTICAL_BAR);
    // token_tree_insert(token_tree, "[",   DELIMITER_LEFT_SQUARE_BRACKET);
    // token_tree_insert(token_tree, "]",   DELIMITER_RIGHT_SQUARE_BRACKET);
    // token_tree_insert(token_tree, "?",   DELIMITER_QUESTION_MARK);
    // token_tree_insert(token_tree, "@",   DELIMITER_COMMERCIAL_AT);
    // token_tree_insert(token_tree, "^",   DELIMITER_CIRCUMFLEX);

    // token_tree_insert(token_tree, "=>",  DELIMITER_ARROW);
    // token_tree_insert(token_tree, "**",  DELIMITER_EXPONENTIATE);
    // token_tree_insert(token_tree, ":=",  DELIMITER_VARIABLE_ASSIGNMENT);
    // token_tree_insert(token_tree, "/=",  DELIMITER_INEQUALITY);
    // token_tree_insert(token_tree, ">=",  DELIMITER_GREATER_THAN_OR_EQUAL);
    // token_tree_insert(token_tree, "<=",  DELIMITER_LESS_THAN_OR_EQUAL);
    // token_tree_insert(token_tree, "<=",  DELIMITER_SIGNAL_ASSIGNMENT);
    token_tree_insert(token_tree, "<>",  DELIMITER_BOX);
    // token_tree_insert(token_tree, "??",  DELIMITER_CONDITION_CONVERSION);
    // token_tree_insert(token_tree, "?=",  DELIMITER_MATCHING_EQUALITY);
    // token_tree_insert(token_tree, "?/=", DELIMITER_MATCHING_INEQUALITY);
    // token_tree_insert(token_tree, "?<",  DELIMITER_MATCHING_LESS_THAN);
    // token_tree_insert(token_tree, "?<=", DELIMITER_MATCHING_LESS_THAN_OR_EQUAL);
    // token_tree_insert(token_tree, "?>",  DELIMITER_MATCHING_GREATER_THAN);
    // token_tree_insert(token_tree, "?>=", DELIMITER_MATCHING_GREATER_THAN_OR_EQUAL);
    // token_tree_insert(token_tree, "<<",  DELIMITER_DOUBLE_LESS_THAN);
    // token_tree_insert(token_tree, ">>",  DELIMITER_DOUBLE_GREATER_THAN);

    token_tree_insert(token_tree, "--",  LINE_COMMENT_START);
    token_tree_insert(token_tree, "/*",  BLOCK_COMMENT_START);
}
//------------------------------------------------------------------------------

static void register_operator_symbols(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "\"??\"",   TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"and\"",  TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"or\"",   TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"nand\"", TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"nor\"",  TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"xor\"",  TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"xnor\"", TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"=\"",    TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"/=\"",   TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"<\"",    TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"<=\"",   TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\">\"",    TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\">=\"",   TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"?=\"",   TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"?/=\"",  TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"?<\"",   TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"?<=\"",  TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"?>\"",   TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"?>=\"",  TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"sll\"",  TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"srl\"",  TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"sla\"",  TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"sra\"",  TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"rol\"",  TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"ror\"",  TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"+\"",    TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"-\"",    TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"&\"",    TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"*\"",    TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"/\"",    TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"mod\"",  TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"rem\"",  TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"**\"",   TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"abs\"",  TOKEN_OPERATOR_SYMBOL);
    token_tree_insert(token_tree, "\"not\"",  TOKEN_OPERATOR_SYMBOL);
}
//------------------------------------------------------------------------------

static void register_attributes(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "base",               ATTRIBUTE_TYPE);
    token_tree_insert(token_tree, "left",               ATTRIBUTE_VALUE);
    token_tree_insert(token_tree, "right",              ATTRIBUTE_VALUE);
    token_tree_insert(token_tree, "high",               ATTRIBUTE_VALUE);
    token_tree_insert(token_tree, "low",                ATTRIBUTE_VALUE);
    token_tree_insert(token_tree, "ascending",          ATTRIBUTE_VALUE);
    token_tree_insert(token_tree, "length",             ATTRIBUTE_PURE_FUNCTION);
    token_tree_insert(token_tree, "range",              ATTRIBUTE_RANGE);
    token_tree_insert(token_tree, "reverse_range",      ATTRIBUTE_RANGE);
    token_tree_insert(token_tree, "subtype",            ATTRIBUTE_SUBTYPE);
    token_tree_insert(token_tree, "image",              ATTRIBUTE_PURE_FUNCTION);
    token_tree_insert(token_tree, "pos",                ATTRIBUTE_PURE_FUNCTION);
    token_tree_insert(token_tree, "succ",               ATTRIBUTE_PURE_FUNCTION);
    token_tree_insert(token_tree, "pred",               ATTRIBUTE_PURE_FUNCTION);
    token_tree_insert(token_tree, "leftof",             ATTRIBUTE_PURE_FUNCTION);
    token_tree_insert(token_tree, "rightof",            ATTRIBUTE_PURE_FUNCTION);
    token_tree_insert(token_tree, "value",              ATTRIBUTE_PURE_FUNCTION);
    token_tree_insert(token_tree, "val",                ATTRIBUTE_PURE_FUNCTION);
    token_tree_insert(token_tree, "designated_subtype", ATTRIBUTE_SUBTYPE);
    token_tree_insert(token_tree, "reflect",            ATTRIBUTE_IMPURE_FUNCTION);
    token_tree_insert(token_tree, "left",               ATTRIBUTE_FUNCTION);
    token_tree_insert(token_tree, "right",              ATTRIBUTE_FUNCTION);
    token_tree_insert(token_tree, "high",               ATTRIBUTE_FUNCTION);
    token_tree_insert(token_tree, "low",                ATTRIBUTE_FUNCTION);
    token_tree_insert(token_tree, "length",             ATTRIBUTE_FUNCTION);
    token_tree_insert(token_tree, "ascending",          ATTRIBUTE_FUNCTION);
    token_tree_insert(token_tree, "index",              ATTRIBUTE_SUBTYPE);
    token_tree_insert(token_tree, "element",            ATTRIBUTE_SUBTYPE);
    token_tree_insert(token_tree, "delayed",            ATTRIBUTE_SIGNAL);
    token_tree_insert(token_tree, "stable",             ATTRIBUTE_SIGNAL);
    token_tree_insert(token_tree, "quiet",              ATTRIBUTE_SIGNAL);
    token_tree_insert(token_tree, "transaction",        ATTRIBUTE_SIGNAL);
    token_tree_insert(token_tree, "event",              ATTRIBUTE_FUNCTION);
    token_tree_insert(token_tree, "active",             ATTRIBUTE_FUNCTION);
    token_tree_insert(token_tree, "last_event",         ATTRIBUTE_FUNCTION);
    token_tree_insert(token_tree, "last_active",        ATTRIBUTE_FUNCTION);
    token_tree_insert(token_tree, "last_value",         ATTRIBUTE_FUNCTION);
    token_tree_insert(token_tree, "driving",            ATTRIBUTE_FUNCTION);
    token_tree_insert(token_tree, "driving_value",      ATTRIBUTE_FUNCTION);
    token_tree_insert(token_tree, "simple_name",        ATTRIBUTE_VALUE);
    token_tree_insert(token_tree, "instance_name",      ATTRIBUTE_VALUE);
    token_tree_insert(token_tree, "path_name",          ATTRIBUTE_VALUE);
    token_tree_insert(token_tree, "record",             ATTRIBUTE_TYPE);
    token_tree_insert(token_tree, "value",              ATTRIBUTE_VALUE);
    token_tree_insert(token_tree, "signal",             ATTRIBUTE_SIGNAL);
    token_tree_insert(token_tree, "converse",           ATTRIBUTE_MODE_VIEW);
}
//------------------------------------------------------------------------------

static void register_base_specifiers(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "b",  BASE_SPECIFIER_BINARY);
    token_tree_insert(token_tree, "o",  BASE_SPECIFIER_OCTAL);
    token_tree_insert(token_tree, "x",  BASE_SPECIFIER_HEX);
    token_tree_insert(token_tree, "ub", BASE_SPECIFIER_BINARY);
    token_tree_insert(token_tree, "uo", BASE_SPECIFIER_OCTAL);
    token_tree_insert(token_tree, "ux", BASE_SPECIFIER_HEX);
    token_tree_insert(token_tree, "sb", BASE_SPECIFIER_BINARY);
    token_tree_insert(token_tree, "so", BASE_SPECIFIER_OCTAL);
    token_tree_insert(token_tree, "sx", BASE_SPECIFIER_HEX);
    token_tree_insert(token_tree, "d",  BASE_SPECIFIER_DECIMAL);
}
//------------------------------------------------------------------------------

void register_core(TokenTree* token_tree)
{
    register_reserved        (token_tree);
    register_directives      (token_tree);
    register_delimiters      (token_tree);
    register_operator_symbols(token_tree);
    register_attributes      (token_tree);
    register_base_specifiers (token_tree);
}
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------

