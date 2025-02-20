================================================================================
Tool directives inside process blocks
================================================================================

process(all) begin
  `if DEBUG = "Yes" then
    for n in 1 to 10 loop
      A := A + B(n);
    end loop;
  `else
    for n in 1 to 15 loop
      A := A + B(n);
    end loop;
  `end if
end process;

--------------------------------------------------------------------------------

(design_file
  (design_unit
    (process_statement
      (sensitivity_specification
        (ALL))
      (sequential_block
        (if_conditional_analysis
          (conditional_analysis_expression
            (conditional_analysis_relation
              (identifier)
              (string_literal))))
        (loop_statement
          (for_loop
            (parameter_specification
              (identifier)
              (simple_range
                (simple_expression
                  (decimal_integer))
                (simple_expression
                  (decimal_integer)))))
          (loop_body
            (simple_variable_assignment
              (name
                (identifier))
              (variable_assignment)
              (conditional_or_unaffected_expression
                (simple_expression
                  (simple_expression
                    (name
                      (identifier)))
                  (adding_operator)
                  (simple_expression
                    (name
                      (identifier)
                      (parenthesis_group
                        (association_or_range_list
                          (association_element
                            (conditional_expression
                              (simple_expression
                                (name
                                  (identifier)))))))))))))
          (end_loop))
        (else_conditional_analysis)
        (loop_statement
          (for_loop
            (parameter_specification
              (identifier)
              (simple_range
                (simple_expression
                  (decimal_integer))
                (simple_expression
                  (decimal_integer)))))
          (loop_body
            (simple_variable_assignment
              (name
                (identifier))
              (variable_assignment)
              (conditional_or_unaffected_expression
                (simple_expression
                  (simple_expression
                    (name
                      (identifier)))
                  (adding_operator)
                  (simple_expression
                    (name
                      (identifier)
                      (parenthesis_group
                        (association_or_range_list
                          (association_element
                            (conditional_expression
                              (simple_expression
                                (name
                                  (identifier)))))))))))))
          (end_loop)))
      (end_conditional_analysis)
      (end_process))))

