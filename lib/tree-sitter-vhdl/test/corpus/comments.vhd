================================================================================
Comment edge-cases
================================================================================

--

/**/

-- Single Line

--SingleLineNoSpace

-- Single line comments can end in */

/* Single line block */

/*SingleLineBlockNoSpace*/

/* Block comments /* do not nest */

/*
Multi-line block
Multi-line block
Multi-line block
*/

/*
 * Multi-line scroll
 * Multi-line scroll
 * Multi-line scroll
 */

--------------------------------------------------------------------------------

(design_file
  (line_comment
    (comment_content))
  (block_comment
    (comment_content))
  (line_comment
    (comment_content))
  (line_comment
    (comment_content))
  (line_comment
    (comment_content))
  (block_comment
    (comment_content))
  (block_comment
    (comment_content))
  (block_comment
    (comment_content))
  (block_comment
    (comment_content))
  (block_comment
    (comment_content)))

