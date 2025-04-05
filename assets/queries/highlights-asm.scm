; General
(label
  [(ident) (word)] @label)

(reg) @variable

(meta
  kind: (_) @function)

(instruction
  kind: (_) @function)

(const
  name: (word) @constant)

; Comments
[
  (line_comment)
  (block_comment)
] @comment

; Literals
(int) @number

(float) @number

(string) @string

; Keywords
[
  "byte"
  "word"
  "dword"
  "qword"
  "ptr"
  "rel"
  "label"
  "const"
] @keyword

; Operators & Punctuation
[
  "+"
  "-"
  "*"
  "/"
  "%"
  "|"
  "^"
  "&"
] @operator

[
  "("
  ")"
  "["
  "]"
] @punctuation.bracket

[
  ","
  ":"
] @punctuation.delimiter
