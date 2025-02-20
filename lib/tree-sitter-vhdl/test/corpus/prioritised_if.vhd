================================================================================
if statement
================================================================================

process(all) begin
  if A1 then
    A <= B;
  elsif A2 then
    A <= B;
  elsif A3 then
    A <= B;
  elsif A4 then
    A <= B;
  elsif A5 then
    A <= B;
  elsif A6 then
    A <= B;
  elsif A7 then
    A <= B;
  elsif A8 then
    A <= B;
  elsif A9 then
    A <= B;
  else
    A <= B;
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
            (simple_expression
              (name
                (identifier)))
            (if_statement_body
              (simple_waveform_assignment
                (name
                  (identifier))
                (signal_assignment)
                (waveform
                  (waveform_element
                    (simple_expression
                      (name
                        (identifier)))))))
            (elsif_statement
              (simple_expression
                (name
                  (identifier)))
              (if_statement_body
                (simple_waveform_assignment
                  (name
                    (identifier))
                  (signal_assignment)
                  (waveform
                    (waveform_element
                      (simple_expression
                        (name
                          (identifier)))))))
              (elsif_statement
                (simple_expression
                  (name
                    (identifier)))
                (if_statement_body
                  (simple_waveform_assignment
                    (name
                      (identifier))
                    (signal_assignment)
                    (waveform
                      (waveform_element
                        (simple_expression
                          (name
                            (identifier)))))))
                (elsif_statement
                  (simple_expression
                    (name
                      (identifier)))
                  (if_statement_body
                    (simple_waveform_assignment
                      (name
                        (identifier))
                      (signal_assignment)
                      (waveform
                        (waveform_element
                          (simple_expression
                            (name
                              (identifier)))))))
                  (elsif_statement
                    (simple_expression
                      (name
                        (identifier)))
                    (if_statement_body
                      (simple_waveform_assignment
                        (name
                          (identifier))
                        (signal_assignment)
                        (waveform
                          (waveform_element
                            (simple_expression
                              (name
                                (identifier)))))))
                    (elsif_statement
                      (simple_expression
                        (name
                          (identifier)))
                      (if_statement_body
                        (simple_waveform_assignment
                          (name
                            (identifier))
                          (signal_assignment)
                          (waveform
                            (waveform_element
                              (simple_expression
                                (name
                                  (identifier)))))))
                      (elsif_statement
                        (simple_expression
                          (name
                            (identifier)))
                        (if_statement_body
                          (simple_waveform_assignment
                            (name
                              (identifier))
                            (signal_assignment)
                            (waveform
                              (waveform_element
                                (simple_expression
                                  (name
                                    (identifier)))))))
                        (elsif_statement
                          (simple_expression
                            (name
                              (identifier)))
                          (if_statement_body
                            (simple_waveform_assignment
                              (name
                                (identifier))
                              (signal_assignment)
                              (waveform
                                (waveform_element
                                  (simple_expression
                                    (name
                                      (identifier)))))))
                          (elsif_statement
                            (simple_expression
                              (name
                                (identifier)))
                            (if_statement_body
                              (simple_waveform_assignment
                                (name
                                  (identifier))
                                (signal_assignment)
                                (waveform
                                  (waveform_element
                                    (simple_expression
                                      (name
                                        (identifier)))))))
                            (else_statement
                              (if_statement_body
                                (simple_waveform_assignment
                                  (name
                                    (identifier))
                                  (signal_assignment)
                                  (waveform
                                    (waveform_element
                                      (simple_expression
                                        (name
                                          (identifier)))))))))))))))))
          (end_if)))
      (end_process))))

================================================================================
if generate
================================================================================

MyLabel: if A1 generate
  A <= B;
elsif A2 generate
  A <= B;
elsif A3 generate
  A <= B;
elsif A4 generate
  A <= B;
elsif A5 generate
  A <= B;
elsif A6 generate
  A <= B;
elsif A7 generate
  A <= B;
elsif A8 generate
  A <= B;
elsif A9 generate
  A <= B;
else generate
  A <= B;
end generate;

--------------------------------------------------------------------------------

(design_file
  (design_unit
    (if_generate_statement
      (label_declaration
        (label))
      (if_generate
        (simple_expression
          (name
            (identifier)))
        (generate_body
          (generate_block
            (concurrent_simple_signal_assignment
              (name
                (identifier))
              (signal_assignment)
              (waveform
                (waveform_element
                  (simple_expression
                    (name
                      (identifier))))))))
        (elsif_generate
          (simple_expression
            (name
              (identifier)))
          (generate_body
            (generate_block
              (concurrent_simple_signal_assignment
                (name
                  (identifier))
                (signal_assignment)
                (waveform
                  (waveform_element
                    (simple_expression
                      (name
                        (identifier))))))))
          (elsif_generate
            (simple_expression
              (name
                (identifier)))
            (generate_body
              (generate_block
                (concurrent_simple_signal_assignment
                  (name
                    (identifier))
                  (signal_assignment)
                  (waveform
                    (waveform_element
                      (simple_expression
                        (name
                          (identifier))))))))
            (elsif_generate
              (simple_expression
                (name
                  (identifier)))
              (generate_body
                (generate_block
                  (concurrent_simple_signal_assignment
                    (name
                      (identifier))
                    (signal_assignment)
                    (waveform
                      (waveform_element
                        (simple_expression
                          (name
                            (identifier))))))))
              (elsif_generate
                (simple_expression
                  (name
                    (identifier)))
                (generate_body
                  (generate_block
                    (concurrent_simple_signal_assignment
                      (name
                        (identifier))
                      (signal_assignment)
                      (waveform
                        (waveform_element
                          (simple_expression
                            (name
                              (identifier))))))))
                (elsif_generate
                  (simple_expression
                    (name
                      (identifier)))
                  (generate_body
                    (generate_block
                      (concurrent_simple_signal_assignment
                        (name
                          (identifier))
                        (signal_assignment)
                        (waveform
                          (waveform_element
                            (simple_expression
                              (name
                                (identifier))))))))
                  (elsif_generate
                    (simple_expression
                      (name
                        (identifier)))
                    (generate_body
                      (generate_block
                        (concurrent_simple_signal_assignment
                          (name
                            (identifier))
                          (signal_assignment)
                          (waveform
                            (waveform_element
                              (simple_expression
                                (name
                                  (identifier))))))))
                    (elsif_generate
                      (simple_expression
                        (name
                          (identifier)))
                      (generate_body
                        (generate_block
                          (concurrent_simple_signal_assignment
                            (name
                              (identifier))
                            (signal_assignment)
                            (waveform
                              (waveform_element
                                (simple_expression
                                  (name
                                    (identifier))))))))
                      (elsif_generate
                        (simple_expression
                          (name
                            (identifier)))
                        (generate_body
                          (generate_block
                            (concurrent_simple_signal_assignment
                              (name
                                (identifier))
                              (signal_assignment)
                              (waveform
                                (waveform_element
                                  (simple_expression
                                    (name
                                      (identifier))))))))
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
                                      (name
                                        (identifier))))))))))))))))))
      (end_generate))))

