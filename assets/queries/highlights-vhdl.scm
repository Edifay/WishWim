(line_comment) @comment
(block_comment) @comment

(decimal_integer @number)

(mode @function)
(library_type @method)
(library_function @function)

(library_constant_std_logic @number)
(string_literal_std_logic @number)

"<=" @function

(string_literal) @string
(character_literal) @string

(identifier @variable)

[
  (library_namespace)
  (selected_name)
] @constant

[
  "access"
  "after"
  "alias"
  "architecture"
  "array"
  "attribute"
  "block"
  "body"
  "component"
  "configuration"
  "context"
  "disconnect"
  "entity"
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
  "sequence"
  "transport"
  "unaffected"
  "view"
  "vunit"
] @keyword

[
  "is"
  "begin"
  "end"
] @keyword

[
  "if"
  "then"
  "elsif"
  "else"
  "case"
] @keyword

[
  "library"
  "use"
] @keyword

[
  "protected"
  "private"
  "pure"
  "impure"
  "inertial"
  "postponed"
  "guarded"
  "inout"
  "linkage"
  "buffer"
  "register"
  "bus"
  "shared"
] @keyword

[
  "to"
  "downto"
  "of"
] @keyword

[
  "process"
  "wait"
  "on"
  "until"
] @keyword

[
  "function"
  "procedure"
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
  "while"
  "loop"
  "next"
  "exit"
  "for"
  "when"
  "return"
] @keyword

[
  "assert"
  "report"
  "severity"
] @keyword

[
  "with"
  "select"
] @keyword

"null" @constant

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
] @function

[
  (unary_operator)
  (logical_operator)
  (shift_operator)
  "mod"
  "not"
  "rem"
] @keyword