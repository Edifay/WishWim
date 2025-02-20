================================================================================
Strings that contain only std_logic
================================================================================

architecture Behaviour of MyModule is
  function "-" (left, right: std_logic) return std_logic;
begin
  A <= "0" or "1" or "u" or "x" or "z" or "w" or "l" or "h" or "-";

  B <= "00000";
  B <= "11111";
  B <= "uuuuu";
  B <= "xxxxx";
  B <= "zzzzz";
  B <= "wwwww";
  B <= "lllll";
  B <= "hhhhh";
  B <= "-----";

  C <= "hello"; -- Starts with std_logic, but then goes to string
  C <= "01101""h"; -- Has a quote in the middle of the string
end Behaviour;

--------------------------------------------------------------------------------

(design_file
  (design_unit
    (architecture_definition
      architecture: (identifier)
      entity: (name
        (identifier))
      (architecture_head
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (library_constant)
                    (library_constant))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type)))))
      (concurrent_block
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (logical_expression
                (logical_expression
                  (logical_expression
                    (logical_expression
                      (logical_expression
                        (logical_expression
                          (logical_expression
                            (logical_expression
                              (simple_expression
                                (string_literal_std_logic))
                              (logical_operator)
                              (simple_expression
                                (string_literal_std_logic)))
                            (logical_operator)
                            (simple_expression
                              (string_literal_std_logic)))
                          (logical_operator)
                          (simple_expression
                            (string_literal_std_logic)))
                        (logical_operator)
                        (simple_expression
                          (string_literal_std_logic)))
                      (logical_operator)
                      (simple_expression
                        (string_literal_std_logic)))
                    (logical_operator)
                    (simple_expression
                      (string_literal_std_logic)))
                  (logical_operator)
                  (simple_expression
                    (string_literal_std_logic)))
                (logical_operator)
                (simple_expression
                  (string_literal_std_logic))))))
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (simple_expression
                (string_literal_std_logic)))))
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (simple_expression
                (string_literal_std_logic)))))
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (simple_expression
                (string_literal_std_logic)))))
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (simple_expression
                (string_literal_std_logic)))))
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (simple_expression
                (string_literal_std_logic)))))
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (simple_expression
                (string_literal_std_logic)))))
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (simple_expression
                (string_literal_std_logic)))))
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (simple_expression
                (string_literal_std_logic)))))
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (simple_expression
                (string_literal_std_logic)))))
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (simple_expression
                (string_literal)))))
        (line_comment
          (comment_content))
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (simple_expression
                (string_literal))))))
      (line_comment
        (comment_content))
      (end_architecture
        architecture: (identifier)))))

