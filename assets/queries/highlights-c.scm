(identifier) @variable

((identifier) @constant
	(#match? @constant "^[A-Z][A-Z\\d_]*$"))

"break" @keyword
"case" @keyword
"const" @keyword
"continue" @keyword
"default" @keyword
"do" @keyword
"else" @keyword
"enum" @keyword
"extern" @keyword
"for" @keyword
"if" @keyword
"inline" @keyword
"return" @keyword
"sizeof" @keyword
"static" @keyword
"struct" @keyword
"switch" @keyword
"typedef" @keyword
"union" @keyword
"volatile" @keyword
"while" @keyword

"#define" @keyword
"#elif" @keyword
"#else" @keyword
"#endif" @keyword
"#if" @keyword
"#ifdef" @keyword
"#ifndef" @keyword
"#include" @keyword
(preproc_directive) @keyword

"--" @operator
"-" @operator
"-=" @operator
"->" @operator
"=" @operator
"!=" @operator
"*" @operator
"&" @operator
"&&" @operator
"+" @operator
"++" @operator
"+=" @operator
"<" @operator
"==" @operator
">" @operator
"||" @operator

"." @delimiter
";" @delimiter

"goto" @keyword

(false) @keyword
(true) @keyword

(string_literal) @string
(system_lib_string) @string

(null) @constant
(number_literal) @number
(char_literal) @number

(field_identifier) @property
(statement_identifier) @label
(type_identifier) @type
(primitive_type) @keyword
(sized_type_specifier) @type

(call_expression
	function: (identifier) @function)
(call_expression
	function: (field_expression
							field: (field_identifier) @function))
(function_declarator
	declarator: (identifier) @function)
(preproc_function_def
	name: (identifier) @function)

(comment) @comment

((comment) @todo
	(#any-match? @todo "TODO"))

((preproc_arg) @keyword
	(#any-of? @keyword "true" "false"))

((preproc_arg) @number
	(#match? @number "^((0x[0-9A-Fa-f]+)|(0b[0-1]+)|([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)))$"))
