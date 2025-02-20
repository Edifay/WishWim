#ifndef math_real_h
#define math_real_h
//------------------------------------------------------------------------------

#include "../../TokenType.h"
//------------------------------------------------------------------------------

static void register_ieee_math_real_constants(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "math_e",             LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_1_over_e",      LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_pi",            LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_2_pi",          LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_1_over_pi",     LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_pi_over_2",     LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_pi_over_3",     LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_pi_over_4",     LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_3_pi_over_2",   LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_log_of_2",      LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_log_of_10",     LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_log2_of_e",     LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_log10_of_e",    LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_sqrt_2",        LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_1_over_sqrt_2", LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_sqrt_pi",       LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_deg_to_rad",    LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "math_rad_to_deg",    LIBRARY_CONSTANT);
}
//------------------------------------------------------------------------------

static void register_ieee_math_real_functions(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "sign",    LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "ceil",    LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "floor",   LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "round",   LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "trunc",   LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "realmax", LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "realmin", LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "uniform", LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "sqrt",    LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "cbrt",    LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "exp",     LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "log",     LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "log2",    LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "log10",   LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "sin",     LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "cos",     LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "tan",     LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "arcsin",  LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "arccos",  LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "arctan",  LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "sinh",    LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "cosh",    LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "tanh",    LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "arcsinh", LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "arccosh", LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "arctanh", LIBRARY_FUNCTION);
}
//------------------------------------------------------------------------------

void register_ieee_math_real(TokenTree* token_tree)
{
    register_ieee_math_real_constants(token_tree);
    register_ieee_math_real_functions(token_tree);
}
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------

