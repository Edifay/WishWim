; Methods
; --------------------

; NOTE: This query is a bit of a work around for the fact that the dart grammar doesn't
; specifically identify a node as a function call
(((identifier) @function (#match? @function "^_?[a-z]"))
 . (selector . (argument_part))) @function

; Annotations
; --------------------
(annotation
  name: (identifier) @attribute)

; Operators and Tokens
; --------------------

(template_substitution
  (identifier_dollar_escaped) @variable
)

(escape_sequence) @string.escape

[
  "("
  ")"
  "["
  "]"
  "{"
  "}"
]  @punctuation.bracket


[
 "@"
 "=>"
 ".."
 "??"
 "=="
 "?"
 ":"
 "&&"
 "%"
 "<"
 ">"
 "="
 ">="
 "<="
 "||"
 "~/"
 (increment_operator)
 (is_operator)
 (prefix_operator)
 (equality_operator)
 (additive_operator)
] @operator

; Delimiters
; --------------------
[
  ";"
  "."
  ","
] @punctuation.delimiter

; Types
; --------------------
((type_identifier) @type.builtin
  (#match? @type.builtin "^(int|double|String|bool|List|Set|Map|Runes|Symbol)$"))
(type_identifier) @type
(class_definition
  name: (identifier) @type)
(constructor_signature
  name: (identifier) @type)
(scoped_identifier
  scope: (identifier) @type)
(function_signature
  name: (identifier) @function)
(getter_signature
  (identifier) @function)
(setter_signature
  name: (identifier) @function)
(type_identifier) @type

((scoped_identifier
  scope: (identifier) @type
  name: (identifier) @type)
 (#match? @type "^[a-zA-Z]"))

; Enums
; -------------------
(enum_declaration
  name: (identifier) @type)
(enum_constant
  name: (identifier) @constant)

; Variables
; --------------------
; var keyword
(inferred_type) @keyword

((identifier) @type
 (#match? @type "^_?[A-Z].*[a-z]"))

("Function" @type)

(this) @variable


(unconditional_assignable_selector
  (identifier) @property)

(conditional_assignable_selector
  (identifier) @property)

(cascade_section
  (cascade_selector
    (identifier) @property))

; assignments
(assignment_expression
  left: (assignable_expression) @variable)

(this) @variable

; Parameters
; --------------------
(formal_parameter
    name: (identifier) @identifier)

(named_argument
  (label (identifier) @identifier))

; Literals
; --------------------
[
    (hex_integer_literal)
    (decimal_integer_literal)
    (decimal_floating_point_literal)
] @number

(string_literal) @string
(symbol_literal (identifier) @constant) @constant
(true) @keyword
(false) @keyword
(null_literal) @constant

(documentation_comment) @comment
(comment) @comment

; Keywords
; --------------------
[
    (assert_builtin)
    (break_builtin)
    (const_builtin)
    (part_of_builtin)
    (rethrow_builtin)
    (void_type)
    "abstract"
    "as"
    "async"
    "async*"
    "await"
    "base"
    "case"
    "catch"
    "class"
    "continue"
    "covariant"
    "default"
    "deferred"
    "do"
    "dynamic"
    "else"
    "enum"
    "export"
    "extends"
    "extension"
    "external"
    "factory"
    "final"
    "finally"
    "for"
    "Function"
    "get"
    "hide"
    "if"
    "implements"
    "import"
    "in"
    "interface"
    "is"
    "late"
    "library"
    "mixin"
    "new"
    "on"
    "operator"
    "part"
    "required"
    "return"
    "sealed"
    "set"
    "show"
    "static"
    "super"
    "switch"
    "sync*"
    "throw"
    "try"
    "typedef"
    "var"
    "when"
    "while"
    "with"
    "yield"
] @keyword

; Variable
(identifier) @variable