;-------------------------------------------------------------------------------
;
; Maintainer: Chris44442
; Capture Reference: https://docs.helix-editor.com/themes.html#syntax-highlighting
;-------------------------------------------------------------------------------
;
(line_comment) @comment

(block_comment) @comment

(identifier) @variable

[
  "access"
  "after"
  "alias"
  "architecture"
  "array"
  ; "assume"
  "attribute"
  "block"
  "body"
  "component"
  "configuration"
  "context"
  ; "cover"
  "disconnect"
  "entity"
  ; "fairness"
  "file"
  "force"
  "generate"
  "generic"
  "group"
  "label"
  "literal"
  "map"
  "new"
  "package"
  "parameter"
  "port"
  "property"
  "range"
  "reject"
  "release"
  ; "restrict"
  "sequence"
  "transport"
  "unaffected"
  "view"
  ; "vmode"
  ; "vpkg"
  ; "vprop"
  "vunit"
] @keyword

[
  (ALL)
  (OTHERS)
  "<>"
  (DEFAULT)
  (OPEN)
] @constant

[
  "is"
  "begin"
  "end"
] @keyword

(parameter_specification
  "in" @keyword)

[
  "process"
  "wait"
  "on"
  "until"
] @keyword

(timeout_clause
  "for" @keyword)

[
  "function"
  "procedure"
] @keyword

[
  "to"
  "downto"
  "of"
] @keyword

[
  "library"
  "use"
] @keyword

[
  "subtype"
  "type"
  "record"
  "units"
  "constant"
  "signal"
  "variable"
] @keyword

[
  "protected"
  "private"
  "pure"
  "impure"
  "inertial"
  "postponed"
  "guarded"
  "out"
  "inout"
  "linkage"
  "buffer"
  "register"
  "bus"
  "shared"
] @keyword

(mode
  "in" @keyword)

(force_mode
  "in" @keyword)

[
  "while"
  "loop"
  "next"
  "exit"
] @keyword

(for_loop
  "for" @keyword)

(block_configuration
  "for" @keyword)

(configuration_specification
  "for" @keyword)

(component_configuration
  "for" @keyword)

(end_for
  "for" @keyword)

"return" @keyword

[
  "assert"
  "report"
  "severity"
] @keyword

[
  "if"
  "then"
  "elsif"
  "case"
] @keyword

(when_element
  "when" @keyword)

(case_generate_alternative
  "when" @keyword)

(else_statement
  "else" @keyword)

(else_generate
  "else" @keyword)

[
  "with"
  "select"
] @keyword

(when_expression
  "when" @keyword)

(else_expression
  "else" @keyword)

(else_waveform
  "else" @keyword)

(else_expression_or_unaffected
  "else" @keyword)

"null" @constant

(user_directive) @keyword

(protect_directive) @keyword

(warning_directive) @keyword

(error_directive) @keyword

(if_conditional_analysis
  "if" @keyword)

(if_conditional_analysis
  "then" @keyword)

(elsif_conditional_analysis
  "elsif" @keyword)

(else_conditional_analysis
  "else" @keyword)

(end_conditional_analysis
  "end" @keyword)

(end_conditional_analysis
  "if" @keyword)

(directive_body) @keyword

(directive_constant_builtin) @constant

(directive_error) @keyword

(directive_protect) @keyword

(directive_warning) @keyword

[
  (condition_conversion)
  (relational_operator)
  (sign)
  (adding_operator)
  (exponentiate)
  (variable_assignment)
  (signal_assignment)
  "*"
  "/"
  ":"
  "=>"
] @operator

[
  (unary_operator)
  (logical_operator)
  (shift_operator)
  "mod"
  "not"
  "rem"
] @keyword

[
  "'"
  ","
  "."
  ";"
] @punctuation

[
  "("
  ")"
  "["
  "]"
  "<<"
  ">>"
] @punctuation

"@" @punctuation

[
  (decimal_integer)
  (string_literal_std_logic)
] @number

(decimal_float) @number

(bit_string_length) @type

(bit_string_base) @type

(bit_string_value) @number

(based_literal
  (based_base) @type
  (based_integer) @number)

(based_literal
  (based_base) @type
  (based_float) @number)

(string_literal) @string

(character_literal) @string

(library_constant_std_logic) @constant

(library_constant) @constant

(library_function) @function

(library_constant_boolean) @constant

(library_constant_character) @character

(library_constant_debug) @keyword

(unit) @keyword

(library_constant_unit) @keyword

(label) @label

(generic_map_aspect
  "generic" @constructor
  "map" @constructor)

(port_map_aspect
  "port" @constructor
  "map" @constructor)

(selection
  (identifier) @variable)

(_
  view: (_) @type)

(_
  type: (_) @type)

(_
  library: (_) @namespace)

(_
  package: (_) @namespace)

(_
  entity: (_) @namespace)

(_
  component: (_) @namespace)

(_
  configuration: (_) @type)

(_
  architecture: (_) @type)

(_
  function: (_) @function)

(_
  procedure: (_) @method)

(_
  attribute: (_) @attribute)

(_
  constant: (_) @constant)

(_
  generic: (_) @variable)

(_
  view: (name
    (_)) @type)

(_
  type: (name
    (_)) @type)

(_
  entity: (name
    (_)) @namespace)

(_
  component: (name
    (_)) @namespace)

(_
  configuration: (name
    (_)) @namespace)

(library_type) @type

(ERROR) @error
