;-------------------------------------------------------------------------------
;
; Maintainer: jpt13653903
; Capture Reference: https://neovim.io/doc/user/treesitter.html#_treesitter-language-injections
; Language Reference: https://github.com/stsewd/tree-sitter-comment
;-------------------------------------------------------------------------------
;
(line_comment
  (comment_content) @injection.content
  (#set! injection.language "comment"))

(block_comment
  (comment_content) @injection.content
  (#set! injection.language "comment"))
