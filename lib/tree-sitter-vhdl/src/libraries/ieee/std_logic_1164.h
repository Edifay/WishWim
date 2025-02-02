#ifndef std_logic_1164_h
#define std_logic_1164_h
//------------------------------------------------------------------------------

#include "../../TokenType.h"
//------------------------------------------------------------------------------

static void register_ieee_std_logic_1164_types(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "std_ulogic",        LIBRARY_TYPE);
    token_tree_insert(token_tree, "std_ulogic_vector", LIBRARY_TYPE);
    token_tree_insert(token_tree, "std_logic",         LIBRARY_TYPE);
    token_tree_insert(token_tree, "std_logic_vector",  LIBRARY_TYPE);
    token_tree_insert(token_tree, "x01",               LIBRARY_TYPE);
    token_tree_insert(token_tree, "x01z",              LIBRARY_TYPE);
    token_tree_insert(token_tree, "ux01",              LIBRARY_TYPE);
    token_tree_insert(token_tree, "ux01z",             LIBRARY_TYPE);
}
//------------------------------------------------------------------------------

static void register_ieee_std_logic_1164_constants(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "\"0", STRING_LITERAL_STD_LOGIC_START);
    token_tree_insert(token_tree, "\"1", STRING_LITERAL_STD_LOGIC_START);
    token_tree_insert(token_tree, "\"u", STRING_LITERAL_STD_LOGIC_START);
    token_tree_insert(token_tree, "\"x", STRING_LITERAL_STD_LOGIC_START);
    token_tree_insert(token_tree, "\"z", STRING_LITERAL_STD_LOGIC_START);
    token_tree_insert(token_tree, "\"w", STRING_LITERAL_STD_LOGIC_START);
    token_tree_insert(token_tree, "\"l", STRING_LITERAL_STD_LOGIC_START);
    token_tree_insert(token_tree, "\"h", STRING_LITERAL_STD_LOGIC_START);
    token_tree_insert(token_tree, "\"-", STRING_LITERAL_STD_LOGIC_START);

    token_tree_insert(token_tree, "\"0\"", TOKEN_STRING_LITERAL_STD_LOGIC);
    token_tree_insert(token_tree, "\"1\"", TOKEN_STRING_LITERAL_STD_LOGIC);
    token_tree_insert(token_tree, "\"u\"", TOKEN_STRING_LITERAL_STD_LOGIC);
    token_tree_insert(token_tree, "\"x\"", TOKEN_STRING_LITERAL_STD_LOGIC);
    token_tree_insert(token_tree, "\"z\"", TOKEN_STRING_LITERAL_STD_LOGIC);
    token_tree_insert(token_tree, "\"w\"", TOKEN_STRING_LITERAL_STD_LOGIC);
    token_tree_insert(token_tree, "\"l\"", TOKEN_STRING_LITERAL_STD_LOGIC);
    token_tree_insert(token_tree, "\"h\"", TOKEN_STRING_LITERAL_STD_LOGIC);
    token_tree_insert(token_tree, "\"-\"", TOKEN_STRING_LITERAL_STD_LOGIC);
}
//------------------------------------------------------------------------------

static void register_ieee_std_logic_1164_functions(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "resolved",             LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_bit",               LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_bitvector",         LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_stdulogic",         LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_stdlogicvector",    LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_stdulogicvector",   LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_01",                LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_x01",               LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_x01z",              LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_ux01",              LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "is_x",                 LIBRARY_FUNCTION);
}
//------------------------------------------------------------------------------

static void register_ieee_std_logic_1164_aliases(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "to_bit_vector",        LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_bv",                LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_std_logic_vector",  LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_slv",               LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_std_ulogic_vector", LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_sulv",              LIBRARY_FUNCTION);
}
//------------------------------------------------------------------------------

void register_ieee_std_logic_1164(TokenTree* token_tree)
{
    register_ieee_std_logic_1164_types    (token_tree);
    register_ieee_std_logic_1164_constants(token_tree);
    register_ieee_std_logic_1164_functions(token_tree);
    register_ieee_std_logic_1164_aliases  (token_tree);
}
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------

