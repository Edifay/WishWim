;From nvim-treesitter/nvim-treesitter
;(atx_heading (inline) @function)
(setext_heading (paragraph) @text.title)
(code_span @comment)

"-" @number

[
	(atx_h1_marker)
	(atx_h2_marker)
	(atx_h3_marker)
	(atx_h4_marker)
	(atx_h5_marker)
	(atx_h6_marker)
	(setext_h1_underline)
	(setext_h2_underline)
	] @function

[
	(link_title)
	(indented_code_block)
	(fenced_code_block)
	] @comment

[
	(fenced_code_block_delimiter)
	] @string

(code_fence_content) @string

[
	(link_destination)
	] @string

[
	(link_label)
	] @string

[
	(list_marker_plus)
	(list_marker_minus)
	(list_marker_star)
	(list_marker_dot)
	(list_marker_parenthesis)
	(thematic_break)
	] @variable

[
	(block_continuation)
	(block_quote_marker)
	] @comment

[
	(backslash_escape)
	] @string
