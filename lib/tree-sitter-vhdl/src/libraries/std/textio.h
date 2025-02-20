#ifndef textio_h
#define textio_h
//------------------------------------------------------------------------------

#include "../../TokenType.h"
//------------------------------------------------------------------------------

static void register_std_textio_types(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "line",              LIBRARY_TYPE);
    token_tree_insert(token_tree, "line_vector",       LIBRARY_TYPE);
    token_tree_insert(token_tree, "text",              LIBRARY_TYPE);
    token_tree_insert(token_tree, "side",              LIBRARY_TYPE);
    token_tree_insert(token_tree, "side_range_record", LIBRARY_TYPE);
    token_tree_insert(token_tree, "width",             LIBRARY_TYPE);
}
//------------------------------------------------------------------------------

static void register_std_textio_constants(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "right",  LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "left",   LIBRARY_CONSTANT);

    // These are files that open std_input and std_output
    token_tree_insert(token_tree, "input",  LIBRARY_CONSTANT);
    token_tree_insert(token_tree, "output", LIBRARY_CONSTANT);
}
//------------------------------------------------------------------------------

static void register_std_textio_functions(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "file_open",     LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "file_close",    LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "file_rewind",   LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "file_seek",     LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "file_truncate", LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "file_state",    LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "file_mode",     LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "file_position", LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "file_size",     LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "file_canseek",  LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "read",          LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "write",         LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "flush",         LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "endfile",       LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "justify",       LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "readline",      LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "sread",         LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "oread",         LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "hread",         LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "writeline",     LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "tee",           LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "owrite",        LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "hwrite",        LIBRARY_FUNCTION);
}
//------------------------------------------------------------------------------

static void register_std_textio_aliases(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "string_read",  LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "bread",        LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "binary_read",  LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "octal_read",   LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "hex_read",     LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "swrite",       LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "string_write", LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "bwrite",       LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "binary_write", LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "octal_write",  LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "hex_write",    LIBRARY_FUNCTION);
}
//------------------------------------------------------------------------------

void register_std_textio(TokenTree* token_tree)
{
    register_std_textio_types    (token_tree);
    register_std_textio_constants(token_tree);
    register_std_textio_functions(token_tree);
    register_std_textio_aliases  (token_tree);
}
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------

