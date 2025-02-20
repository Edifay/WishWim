#ifndef numeric_std_h
#define numeric_std_h
//------------------------------------------------------------------------------

#include "../../TokenType.h"
//------------------------------------------------------------------------------

static void register_ieee_numeric_std_types(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "unresolved_unsigned", LIBRARY_TYPE);
    token_tree_insert(token_tree, "unresolved_signed",   LIBRARY_TYPE);
    token_tree_insert(token_tree, "unsigned",            LIBRARY_TYPE);
    token_tree_insert(token_tree, "signed",              LIBRARY_TYPE);
}
//------------------------------------------------------------------------------

static void register_ieee_numeric_std_constants(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "CopyRightNotice", LIBRARY_CONSTANT);
}
//------------------------------------------------------------------------------

static void register_ieee_numeric_std_functions(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "find_leftmost",  LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "find_rightmost", LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "shift_left",     LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "shift_right",    LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "rotate_left",    LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "rotate_right",   LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "resize",         LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_integer",     LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_unsigned",    LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_signed",      LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "std_match",      LIBRARY_FUNCTION);
}
//------------------------------------------------------------------------------

static void register_ieee_numeric_std_aliases(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "u_unsigned", LIBRARY_TYPE);
    token_tree_insert(token_tree, "u_signed",   LIBRARY_TYPE);
}
//------------------------------------------------------------------------------

void register_ieee_numeric_std(TokenTree* token_tree)
{
    register_ieee_numeric_std_types    (token_tree);
    register_ieee_numeric_std_constants(token_tree);
    register_ieee_numeric_std_functions(token_tree);
    register_ieee_numeric_std_aliases  (token_tree);
}
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------

