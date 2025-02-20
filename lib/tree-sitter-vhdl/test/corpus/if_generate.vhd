================================================================================
If generate structure
================================================================================

testGenerate: if TEST = 1 generate
  A <= B + C;
elsif TEST = 2 generate
  A <= B + C;
else generate
  A <= B + C;
end generate testGenerate;

--------------------------------------------------------------------------------

(design_file
  (design_unit
    (if_generate_statement
      (label_declaration
        (label))
      (if_generate
        (relational_expression
          (simple_expression
            (name
              (identifier)))
          (relational_operator)
          (simple_expression
            (decimal_integer)))
        (generate_body
          (generate_block
            (concurrent_simple_signal_assignment
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
                        (identifier)))))))))
        (elsif_generate
          (relational_expression
            (simple_expression
              (name
                (identifier)))
            (relational_operator)
            (simple_expression
              (decimal_integer)))
          (generate_body
            (generate_block
              (concurrent_simple_signal_assignment
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
                          (identifier)))))))))
          (else_generate
            (generate_body
              (generate_block
                (concurrent_simple_signal_assignment
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
      (end_generate
        (label)))))

