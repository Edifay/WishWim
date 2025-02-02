================================================================================
Built-in libraries, operator overloading and a basic architecture body
================================================================================

library work;
    use work.whatnot.all;

library std;
    use std.whatnot.all;

library ieee;
    use ieee.whatnot.all;

entity Full_Adder is
  port(
    Clk   : in  std_logic;
    Reset : in  std_logic
  );
end entity Full_Adder;

-- A body of entity Full_Adder:
architecture DataFlow of Full_Adder is
  signal A, B: Bit;

  function "-"    (A : std_logic) return integer;
  function "??"   (A : std_logic) return integer;
  function "and"  (A : std_logic) return integer;
  function "or"   (A : std_logic) return integer;
  function "nand" (A : std_logic) return integer;
  function "nor"  (A : std_logic) return integer;
  function "xor"  (A : std_logic) return integer;
  function "xnor" (A : std_logic) return integer;
  function "="    (A : std_logic) return integer;
  function "/="   (A : std_logic) return integer;
  function "<"    (A : std_logic) return integer;
  function "<="   (A : std_logic) return integer;
  function ">"    (A : std_logic) return integer;
  function ">="   (A : std_logic) return integer;
  function "?="   (A : std_logic) return integer;
  function "?/="  (A : std_logic) return integer;
  function "?<"   (A : std_logic) return integer;
  function "?<="  (A : std_logic) return integer;
  function "?>"   (A : std_logic) return integer;
  function "?>="  (A : std_logic) return integer;
  function "sll"  (A : std_logic) return integer;
  function "srl"  (A : std_logic) return integer;
  function "sla"  (A : std_logic) return integer;
  function "sra"  (A : std_logic) return integer;
  function "rol"  (A : std_logic) return integer;
  function "ror"  (A : std_logic) return integer;
  function "+"    (A : std_logic) return integer;
  function "-"    (A : std_logic) return integer;
  function "&"    (A : std_logic) return integer;
  function "*"    (A : std_logic) return integer;
  function "/"    (A : std_logic) return integer;
  function "mod"  (A : std_logic) return integer;
  function "rem"  (A : std_logic) return integer;
  function "**"   (A : std_logic) return integer;
  function "abs"  (A : std_logic) return integer;
  function "not"  (A : std_logic) return integer;

  function MyFunc (A : std_logic) return integer;
begin
  MyInstance: entity work.Whatnot port map(
    Clock => Clk,
    Reset => Reset
  );

  A    <= X xor Y;
  B    <= A and Cin;
  Sum  <= A xor Cin;
  Cout <= B or (X and Y);

  process(A, B, C) is
    constant B : float := 8.291;
    variable A : integer := 5;
  begin
    if(rising_edge(Clk)) then
        A <= 5;
    else
        C <= 7;
    end if;
    for n in 0 to 3 loop
      case(WrRegisters.BenchTesting.HBW_Select(n)) is
        when "01"   => opHBW_SEL(n+1) <= '1';
        when "10"   => opHBW_SEL(n+1) <= '0';
        when others => opHBW_SEL(n+1) <= ipUSER_DIP(n);
      end case;
    end loop;
  end process;
  opHBW_SEL_N <= not(opHBW_SEL);
end architecture DataFlow;

--------------------------------------------------------------------------------

(design_file
  (design_unit
    (library_clause
      (logical_name_list
        library: (library_namespace)))
    (use_clause
      (selected_name_list
        (selected_name
          library: (library_namespace)
          package: (identifier)
          (ALL))))
    (library_clause
      (logical_name_list
        library: (library_namespace)))
    (use_clause
      (selected_name_list
        (selected_name
          library: (library_namespace)
          package: (identifier)
          (ALL))))
    (library_clause
      (logical_name_list
        library: (library_namespace)))
    (use_clause
      (selected_name_list
        (selected_name
          library: (library_namespace)
          package: (identifier)
          (ALL))))
    (entity_declaration
      entity: (identifier)
      (entity_head
        (port_clause
          (interface_list
            (interface_declaration
              (identifier_list
                (identifier))
              (simple_mode_indication
                (mode)
                (subtype_indication
                  type: (name
                    (library_type)))))
            (interface_declaration
              (identifier_list
                (identifier))
              (simple_mode_indication
                (mode)
                (subtype_indication
                  type: (name
                    (library_type))))))))
      (end_entity
        entity: (identifier))))
  (line_comment
    (comment_content))
  (design_unit
    (architecture_definition
      architecture: (identifier)
      entity: (name
        (identifier))
      (architecture_head
        (signal_declaration
          (identifier_list
            (identifier)
            (identifier))
          (subtype_indication
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (operator_symbol)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type))))
        (subprogram_declaration
          (function_specification
            function: (identifier)
            (parameter_list_specification
              (interface_list
                (interface_declaration
                  (identifier_list
                    (identifier))
                  (simple_mode_indication
                    (subtype_indication
                      type: (name
                        (library_type)))))))
            type: (name
              (library_type)))))
      (concurrent_block
        (component_instantiation_statement
          (label_declaration
            (label))
          (instantiated_unit
            library: (library_namespace)
            entity: (name
              (identifier)))
          (port_map_aspect
            (association_list
              (association_element
                (name
                  (identifier))
                (conditional_expression
                  (simple_expression
                    (name
                      (identifier)))))
              (association_element
                (name
                  (identifier))
                (conditional_expression
                  (simple_expression
                    (name
                      (identifier))))))))
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (logical_expression
                (simple_expression
                  (name
                    (identifier)))
                (logical_operator)
                (simple_expression
                  (name
                    (identifier)))))))
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (logical_expression
                (simple_expression
                  (name
                    (identifier)))
                (logical_operator)
                (simple_expression
                  (name
                    (identifier)))))))
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (logical_expression
                (simple_expression
                  (name
                    (identifier)))
                (logical_operator)
                (simple_expression
                  (name
                    (identifier)))))))
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (logical_expression
                (simple_expression
                  (name
                    (identifier)))
                (logical_operator)
                (simple_expression
                  (parenthesis_expression
                    (element_association_list
                      (element_association
                        (conditional_expression
                          (logical_expression
                            (simple_expression
                              (name
                                (identifier)))
                            (logical_operator)
                            (simple_expression
                              (name
                                (identifier)))))))))))))
        (process_statement
          (sensitivity_specification
            (sensitivity_list
              (name
                (identifier))
              (name
                (identifier))
              (name
                (identifier))))
          (process_head
            (constant_declaration
              (identifier_list
                constant: (identifier))
              (subtype_indication
                type: (name
                  (library_type)))
              (initialiser
                (variable_assignment)
                (conditional_expression
                  (simple_expression
                    (decimal_float)))))
            (variable_declaration
              (identifier_list
                (identifier))
              (subtype_indication
                type: (name
                  (library_type)))
              (initialiser
                (variable_assignment)
                (conditional_expression
                  (simple_expression
                    (decimal_integer))))))
          (sequential_block
            (if_statement_block
              (if_statement
                (simple_expression
                  (parenthesis_expression
                    (element_association_list
                      (element_association
                        (conditional_expression
                          (simple_expression
                            (name
                              (library_function)
                              (parenthesis_group
                                (association_or_range_list
                                  (association_element
                                    (conditional_expression
                                      (simple_expression
                                        (name
                                          (identifier))))))))))))))
                (if_statement_body
                  (simple_waveform_assignment
                    (name
                      (identifier))
                    (signal_assignment)
                    (waveform
                      (waveform_element
                        (simple_expression
                          (decimal_integer))))))
                (else_statement
                  (if_statement_body
                    (simple_waveform_assignment
                      (name
                        (identifier))
                      (signal_assignment)
                      (waveform
                        (waveform_element
                          (simple_expression
                            (decimal_integer))))))))
              (end_if))
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
                (case_statement
                  (case_expression
                    (simple_expression
                      (parenthesis_expression
                        (element_association_list
                          (element_association
                            (conditional_expression
                              (simple_expression
                                (name
                                  (identifier)
                                  (selection
                                    (identifier))
                                  (selection
                                    (identifier))
                                  (parenthesis_group
                                    (association_or_range_list
                                      (association_element
                                        (conditional_expression
                                          (simple_expression
                                            (name
                                              (identifier)))))))))))))))
                  (case_body
                    (case_statement_alternative
                      (when_element
                        (simple_expression
                          (string_literal_std_logic)))
                      (simple_waveform_assignment
                        (name
                          (identifier)
                          (parenthesis_group
                            (association_or_range_list
                              (association_element
                                (conditional_expression
                                  (simple_expression
                                    (simple_expression
                                      (name
                                        (identifier)))
                                    (adding_operator)
                                    (simple_expression
                                      (decimal_integer))))))))
                        (signal_assignment)
                        (waveform
                          (waveform_element
                            (simple_expression
                              (name
                                (library_constant_std_logic)))))))
                    (case_statement_alternative
                      (when_element
                        (simple_expression
                          (string_literal_std_logic)))
                      (simple_waveform_assignment
                        (name
                          (identifier)
                          (parenthesis_group
                            (association_or_range_list
                              (association_element
                                (conditional_expression
                                  (simple_expression
                                    (simple_expression
                                      (name
                                        (identifier)))
                                    (adding_operator)
                                    (simple_expression
                                      (decimal_integer))))))))
                        (signal_assignment)
                        (waveform
                          (waveform_element
                            (simple_expression
                              (name
                                (library_constant_std_logic)))))))
                    (case_statement_alternative
                      (when_element
                        (OTHERS))
                      (simple_waveform_assignment
                        (name
                          (identifier)
                          (parenthesis_group
                            (association_or_range_list
                              (association_element
                                (conditional_expression
                                  (simple_expression
                                    (simple_expression
                                      (name
                                        (identifier)))
                                    (adding_operator)
                                    (simple_expression
                                      (decimal_integer))))))))
                        (signal_assignment)
                        (waveform
                          (waveform_element
                            (simple_expression
                              (name
                                (identifier)
                                (parenthesis_group
                                  (association_or_range_list
                                    (association_element
                                      (conditional_expression
                                        (simple_expression
                                          (name
                                            (identifier))))))))))))))
                  (end_case)))
              (end_loop)))
          (end_process))
        (concurrent_simple_signal_assignment
          (name
            (identifier))
          (signal_assignment)
          (waveform
            (waveform_element
              (simple_expression
                (unary_operator)
                (simple_expression
                  (parenthesis_expression
                    (element_association_list
                      (element_association
                        (conditional_expression
                          (simple_expression
                            (name
                              (identifier)))))))))))))
      (end_architecture
        architecture: (identifier)))))

