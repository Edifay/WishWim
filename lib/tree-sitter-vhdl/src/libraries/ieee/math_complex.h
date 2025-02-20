#ifndef math_complex_h
#define math_complex_h
//------------------------------------------------------------------------------

#include "../../TokenType.h"
//------------------------------------------------------------------------------

static void register_ieee_math_complex_types(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "complex",         LIBRARY_TYPE);
    token_tree_insert(token_tree, "positive_real",   LIBRARY_TYPE);
    token_tree_insert(token_tree, "principal_value", LIBRARY_TYPE);
    token_tree_insert(token_tree, "complex_polar",   LIBRARY_TYPE);
}
//------------------------------------------------------------------------------

static void register_ieee_math_complex_constants(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "math_cbase_1", LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_cbase_j", LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_czero",   LIBRARY_CONSTANT);
}
//------------------------------------------------------------------------------

static void register_ieee_math_complex_functions(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "cmplx",               LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "get_principal_value", LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "complex_to_polar",    LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "polar_to_complex",    LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "arg",                 LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "conj",                LIBRARY_FUNCTION);
}
//------------------------------------------------------------------------------

void register_ieee_math_complex(TokenTree* token_tree)
{
    register_ieee_math_complex_types    (token_tree);
    register_ieee_math_complex_constants(token_tree);
    register_ieee_math_complex_functions(token_tree);
}
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------

