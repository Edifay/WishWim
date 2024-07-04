(program)
; <- @punctuation.bracket
; ^ @variable
;       ^ @punctuation.bracket

";"
; <- @string

(node _ @wildcard)
;       ^ @punctuation.special
;        ^ @type

name: (identifier)
; <- @property
;   ^ @punctuation.delimiter

(program
  name: (_))
;        ^ @constant

(program
  name: _)
;       ^ @constant

(program
  field: (identifier) @capture
  !negated-field)
; ^ @operator
;  ^ @property

(program
  (identifier)+)
;             ^ @operator

((node) [(node) "test"])
;       ^ @punctuation.bracket
;                     ^ @punctuation.bracket

((identifier) @v
  (#eq? @v "test" local))
;  ^ @punctuation.special
;   ^ @function.call
;       ^ @punctuation.special
;        ^ @type
;          ^ @string
;     ^ @punctuation.special

((comment) . (function_declaration))
;          ^ @operator

; vim:ft=query:
