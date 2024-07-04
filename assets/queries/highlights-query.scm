(string) @string

(escape_sequence) @string

(capture
  (identifier) @type)

(anonymous_node
  (string) @string)

(predicate
  name: (identifier) @function)

(named_node
  name: (identifier) @variable)

(field_definition
  name: (identifier) @property)

(negated_field
  "!" @operator
  (identifier) @property)

(comment) @comment

(quantifier) @operator

(predicate_type) @punctuation

"." @operator

[
  "["
  "]"
  "("
  ")"
] @punctuation

":" @punctuation

[
  "@"
  "#"
] @punctuation.special

"_" @constant

((parameters
  (identifier) @number)
  (#match? @number "^[-+]?[0-9]+(.[0-9]+)?$"))

((program
  .
  (comment)*
  .
  (comment) @keyword)
  (#lua-match? @keyword "^;+ *inherits *:"))

((program
  .
  (comment)*
  .
  (comment) @keyword)
  (#lua-match? @keyword "^;+ *extends *$"))

((comment) @keyword
  (#lua-match? @keyword "^;+%s*format%-ignore%s*$"))

((predicate
  name: (identifier) @_name
  parameters:
    (parameters
      (string
        "\"" @string
        "\"" @string) @string))
  (#any-of? @_name "match" "not-match" "vim-match" "not-vim-match" "lua-match" "not-lua-match"))

((predicate
  name: (identifier) @_name
  parameters:
    (parameters
      (string
        "\"" @string
        "\"" @string) @string
      .
      (string) .))
  (#any-of? @_name "gsub" "not-gsub"))
