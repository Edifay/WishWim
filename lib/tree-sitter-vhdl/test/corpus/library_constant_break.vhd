================================================================================
Library constants used in names used to break the parser
================================================================================

architecture Behaviour of MyModule is
begin
  A <= Output.Valid;
  B <= Output.Enabled;
end Behaviour;

--------------------------------------------------------------------------------

(design_file
  (design_unit
    (architecture_definition
      architecture: (identifier)
      entity: (name
        (identifier))
      (architecture_head)
      (concurrent_block
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (simple_expression
                (name
                  (library_constant)
                  (selection
                    (identifier)))))))
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (simple_expression
                (name
                  (library_constant)
                  (selection
                    (identifier))))))))
      (end_architecture
        architecture: (identifier)))))

