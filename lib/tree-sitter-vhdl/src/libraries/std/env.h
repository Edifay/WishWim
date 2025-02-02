#ifndef std_env_h
#define std_env_h
//------------------------------------------------------------------------------

#include "../../TokenType.h"
//------------------------------------------------------------------------------

static void register_std_env_types(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "dayofweek",                       LIBRARY_TYPE);
    token_tree_insert(token_tree, "time_record",                     LIBRARY_TYPE);
    token_tree_insert(token_tree, "directory_items",                 LIBRARY_TYPE);
    token_tree_insert(token_tree, "directory",                       LIBRARY_TYPE);
    token_tree_insert(token_tree, "dir_open_status",                 LIBRARY_TYPE);
    token_tree_insert(token_tree, "dir_open_status_range_record",    LIBRARY_TYPE);
    token_tree_insert(token_tree, "dir_create_status",               LIBRARY_TYPE);
    token_tree_insert(token_tree, "dir_create_status_range_record",  LIBRARY_TYPE);
    token_tree_insert(token_tree, "dir_delete_status",               LIBRARY_TYPE);
    token_tree_insert(token_tree, "dir_delete_status_range_record",  LIBRARY_TYPE);
    token_tree_insert(token_tree, "file_delete_status",              LIBRARY_TYPE);
    token_tree_insert(token_tree, "file_delete_status_range_record", LIBRARY_TYPE);
    token_tree_insert(token_tree, "call_path_element",               LIBRARY_TYPE);
    token_tree_insert(token_tree, "call_path_vector",                LIBRARY_TYPE);
    token_tree_insert(token_tree, "call_path_vector_ptr",            LIBRARY_TYPE);
}
//------------------------------------------------------------------------------

static void register_std_env_constants(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "sunday",               LIBRARY_CONSTANT_ENV);
    token_tree_insert(token_tree, "monday",               LIBRARY_CONSTANT_ENV);
    token_tree_insert(token_tree, "tuesday",              LIBRARY_CONSTANT_ENV);
    token_tree_insert(token_tree, "wednesday",            LIBRARY_CONSTANT_ENV);
    token_tree_insert(token_tree, "thursday",             LIBRARY_CONSTANT_ENV);
    token_tree_insert(token_tree, "friday",               LIBRARY_CONSTANT_ENV);
    token_tree_insert(token_tree, "saturday",             LIBRARY_CONSTANT_ENV);

    token_tree_insert(token_tree, "status_ok",            LIBRARY_CONSTANT_ENV);
    token_tree_insert(token_tree, "status_not_found",     LIBRARY_CONSTANT_ENV);
    token_tree_insert(token_tree, "status_no_directory",  LIBRARY_CONSTANT_ENV);
    token_tree_insert(token_tree, "status_access_denied", LIBRARY_CONSTANT_ENV);
    token_tree_insert(token_tree, "status_item_exists",   LIBRARY_CONSTANT_ENV);
    token_tree_insert(token_tree, "status_not_empty",     LIBRARY_CONSTANT_ENV);
    token_tree_insert(token_tree, "status_no_file",       LIBRARY_CONSTANT_ENV);
    token_tree_insert(token_tree, "status_error",         LIBRARY_CONSTANT_ENV);

    token_tree_insert(token_tree, "dir_separator",        LIBRARY_CONSTANT_ENV);
}
//------------------------------------------------------------------------------

static void register_std_env_functions(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "stop",                LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "finish",              LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "resolution_limit",    LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "localtime",           LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "gmtime",              LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "epoch",               LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "time_to_seconds",     LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "seconds_to_time",     LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "to_string",           LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "file_name",           LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "file_path",           LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "file_line",           LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "minimum",             LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "maximum",             LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "dir_open",            LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "dir_close",           LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "dir_itemexists",      LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "dir_itemisdir",       LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "dir_itemisfile",      LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "dir_workingdir",      LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "dir_createdir",       LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "dir_deletedir",       LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "dir_deletefile",      LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "getenv",              LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "vhdl_version",        LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "tool_type",           LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "tool_vendor",         LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "tool_name",           LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "tool_edition",        LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "tool_version",        LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "deallocate",          LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "get_call_path",       LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "pslassertfailed",     LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "psliscovered",        LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "setpslcoverassert",   LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "getpslcoverassert",   LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "pslisassertcovered",  LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "clearpslstate",       LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "isvhdlassertfailed",  LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "getvhdlassertcount",  LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "clearvhdlassert",     LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "setvhdlassertenable", LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "getvhdlassertenable", LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "setvhdlassertformat", LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "getvhdlassertformat", LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "setvhdlreadseverity", LIBRARY_FUNCTION);
    token_tree_insert(token_tree, "getvhdlreadseverity", LIBRARY_FUNCTION);
}
//------------------------------------------------------------------------------

static void register_std_env_namespaces(TokenTree* token_tree)
{
    token_tree_insert(token_tree, "work", LIBRARY_NAMESPACE);
    token_tree_insert(token_tree, "std",  LIBRARY_NAMESPACE);
    token_tree_insert(token_tree, "ieee", LIBRARY_NAMESPACE);
}
//------------------------------------------------------------------------------

void register_std_env(TokenTree* token_tree)
{

    register_std_env_types     (token_tree);
    register_std_env_constants (token_tree);
    register_std_env_functions (token_tree);
    register_std_env_namespaces(token_tree);
}
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------
