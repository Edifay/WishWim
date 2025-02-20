================================================================================
Basic architecture, testing blocks
================================================================================

/*------------------------------------------------------------------------------

A multi-line block comment
------------------------------------------------------------------------------*/

architecture EXAMPLE of STRUCTURE is
  subtype  DIGIT is integer range 0 to 9;
begin
  DIGIT_A <= 3; -- A line-comment
end EXAMPLE;

--------------------------------------------------------------------------------

(design_file
  (block_comment
    (comment_content))
  (design_unit
    (architecture_definition
      architecture: (identifier)
      entity: (name
        (identifier))
    (architecture_head
      (subtype_declaration
        type: (identifier)
        (subtype_indication
          type: (name
            (library_type))
        (range_constraint
          (simple_range
            (simple_expression
              (decimal_integer))
            (simple_expression
              (decimal_integer)))))))
    (concurrent_block
      (concurrent_simple_signal_assignment
        (name
          (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (decimal_integer))))))
    (line_comment
      (comment_content))
    (end_architecture
      architecture: (identifier)))))

