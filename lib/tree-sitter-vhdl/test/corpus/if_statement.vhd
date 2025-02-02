================================================================================
If statements
================================================================================

process(all) begin
  if TEST = 1 then
    A <= B + C;
  elsif TEST = 2 then
    A <= B + C;
  elsif TEST = 3 then
    A <= B + C;
  else
    A <= B + C;
  end if;
end process;

--------------------------------------------------------------------------------

(design_file
  (design_unit
    (process_statement
      (sensitivity_specification
        (ALL))
      (sequential_block
        (if_statement_block
          (if_statement
            (relational_expression
              (simple_expression
                (name
                  (identifier)))
              (relational_operator)
              (simple_expression
                (decimal_integer)))
            (if_statement_body
              (simple_waveform_assignment
                (name
                  (identifier))
                (signal_assignment)
                (waveform
                  (waveform_element
                    (simple_expression
                      (simple_expression
                        (name
                          (identifier)))
                      (adding_operator)
                      (simple_expression
                        (name
                          (identifier))))))))
            (elsif_statement
              (relational_expression
                (simple_expression
                  (name
                    (identifier)))
                (relational_operator)
                (simple_expression
                  (decimal_integer)))
              (if_statement_body
                (simple_waveform_assignment
                  (name
                    (identifier))
                  (signal_assignment)
                  (waveform
                    (waveform_element
                      (simple_expression
                        (simple_expression
                          (name
                            (identifier)))
                        (adding_operator)
                        (simple_expression
                          (name
                            (identifier))))))))
              (elsif_statement
                (relational_expression
                  (simple_expression
                    (name
                      (identifier)))
                  (relational_operator)
                  (simple_expression
                    (decimal_integer)))
                (if_statement_body
                  (simple_waveform_assignment
                    (name
                      (identifier))
                    (signal_assignment)
                    (waveform
                      (waveform_element
                        (simple_expression
                          (simple_expression
                            (name
                              (identifier)))
                          (adding_operator)
                          (simple_expression
                            (name
                              (identifier))))))))
                (else_statement
                  (if_statement_body
                    (simple_waveform_assignment
                      (name
                        (identifier))
                      (signal_assignment)
                      (waveform
                        (waveform_element
                          (simple_expression
                            (simple_expression
                              (name
                                (identifier)))
                            (adding_operator)
                            (simple_expression
                              (name
                                (identifier))))))))))))
          (end_if)))
      (end_process))))

