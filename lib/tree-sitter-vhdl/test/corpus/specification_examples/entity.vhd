================================================================================
An entity declaration with port declarations only:
================================================================================

entity Full_Adder is
  port (X, Y, Cin: in Bit; Cout, Sum: out Bit);
end Full_Adder;

--------------------------------------------------------------------------------

(design_file
  (design_unit
    (entity_declaration
      entity: (identifier)
      (entity_head
        (port_clause
          (interface_list
            (interface_declaration
              (identifier_list
                (identifier)
                (identifier)
                (identifier))
              (simple_mode_indication
                (mode)
                (subtype_indication
                  type: (name
                    (library_type)))))
            (interface_declaration
              (identifier_list
                (identifier)
                (identifier))
              (simple_mode_indication
                (mode)
                (subtype_indication
                  type: (name
                    (library_type))))))))
      (end_entity
        entity: (identifier)))))

================================================================================
An entity declaration with generic declarations also:
================================================================================

entity AndGate is
  generic (N: Natural := 2);
  port( inputs: in Bit_Vector (1 to N);
        result: out Bit);
end entity AndGate;

--------------------------------------------------------------------------------

(design_file
  (design_unit
    (entity_declaration
      entity: (identifier)
      (entity_head
        (generic_clause
          (interface_list
            (interface_declaration
              (identifier_list
                generic: (identifier))
              (simple_mode_indication
                (subtype_indication
                  type: (name
                    (library_type)))
                (initialiser
                  (variable_assignment)
                  (conditional_expression
                    (simple_expression
                      (decimal_integer))))))))
        (port_clause
          (interface_list
            (interface_declaration
              (identifier_list
                (identifier))
              (simple_mode_indication
                (mode)
                (subtype_indication
                  type: (name
                    (library_type)
                    (parenthesis_group
                      (association_or_range_list
                        (simple_range
                          (simple_expression
                            (decimal_integer))
                          (simple_expression
                            (name
                              (identifier))))))))))
            (interface_declaration
              (identifier_list
                (identifier))
              (simple_mode_indication
                (mode)
                (subtype_indication
                  type: (name
                    (library_type))))))))
      (end_entity
        entity: (identifier)))))

================================================================================
An entity declaration with neither:
================================================================================

entity TestBench is
end TestBench;

--------------------------------------------------------------------------------

(design_file
  (design_unit
    (entity_declaration
      entity: (identifier)
      (entity_head)
      (end_entity
        entity: (identifier)))))

================================================================================
An entity declaration with entity declarative items:
================================================================================

entity ROM is
  port ( Addr: in Word;
         Data: out Word;
         Sel: in Bit);
  type Instruction is array (1 to 5) of Natural;
  type Program is array (Natural range <>) of Instruction;
  use Work.OpCodes.all, Work.RegisterNames.all;
  constant ROM_Code: Program :=
    (
      (STM, R14, R12, 12, R13),
      (LD, R7, 32, 0, R1 ),
      (BAL, R14, 0, 0, R7 )
      -- etc.
    ) ;
end ROM;

--------------------------------------------------------------------------------

(design_file
  (design_unit
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
                    (identifier)))))
            (interface_declaration
              (identifier_list
                (identifier))
              (simple_mode_indication
                (mode)
                (subtype_indication
                  type: (name
                    (identifier)))))
            (interface_declaration
              (identifier_list
                (identifier))
              (simple_mode_indication
                (mode)
                (subtype_indication
                  type: (name
                    (library_type)))))))
        (type_declaration
          type: (identifier)
          (array_type_definition
            (array_index_incomplete_type_list
              (index_constraint
                (simple_range
                  (simple_expression
                    (decimal_integer))
                  (simple_expression
                    (decimal_integer)))))
            (subtype_indication
              type: (name
                (library_type)))))
        (type_declaration
          type: (identifier)
          (array_type_definition
            (array_index_incomplete_type_list
              (index_subtype_definition
                type: (name
                  (library_type))))
            (subtype_indication
              type: (name
                (identifier)))))
        (use_clause
          (selected_name_list
            (selected_name
              library: (library_namespace)
              package: (identifier)
              (ALL))
            (selected_name
              library: (library_namespace)
              package: (identifier)
              (ALL))))
        (constant_declaration
          (identifier_list
            constant: (identifier))
          (subtype_indication
            type: (name
              (identifier)))
          (initialiser
            (variable_assignment)
            (conditional_expression
              (simple_expression
                (parenthesis_expression
                  (element_association_list
                    (element_association
                      (conditional_expression
                        (simple_expression
                          (parenthesis_expression
                            (element_association_list
                              (element_association
                                (conditional_expression
                                  (simple_expression
                                    (name
                                      (identifier)))))
                              (element_association
                                (conditional_expression
                                  (simple_expression
                                    (name
                                      (identifier)))))
                              (element_association
                                (conditional_expression
                                  (simple_expression
                                    (name
                                      (identifier)))))
                              (element_association
                                (conditional_expression
                                  (simple_expression
                                    (decimal_integer))))
                              (element_association
                                (conditional_expression
                                  (simple_expression
                                    (name
                                      (identifier))))))))))
                    (element_association
                      (conditional_expression
                        (simple_expression
                          (parenthesis_expression
                            (element_association_list
                              (element_association
                                (conditional_expression
                                  (simple_expression
                                    (name
                                      (identifier)))))
                              (element_association
                                (conditional_expression
                                  (simple_expression
                                    (name
                                      (identifier)))))
                              (element_association
                                (conditional_expression
                                  (simple_expression
                                    (decimal_integer))))
                              (element_association
                                (conditional_expression
                                  (simple_expression
                                    (decimal_integer))))
                              (element_association
                                (conditional_expression
                                  (simple_expression
                                    (name
                                      (identifier))))))))))
                    (element_association
                      (conditional_expression
                        (simple_expression
                          (parenthesis_expression
                            (element_association_list
                              (element_association
                                (conditional_expression
                                  (simple_expression
                                    (name
                                      (identifier)))))
                              (element_association
                                (conditional_expression
                                  (simple_expression
                                    (name
                                      (identifier)))))
                              (element_association
                                (conditional_expression
                                  (simple_expression
                                    (decimal_integer))))
                              (element_association
                                (conditional_expression
                                  (simple_expression
                                    (decimal_integer))))
                              (element_association
                                (conditional_expression
                                  (simple_expression
                                    (name
                                      (identifier)))))))))))
                  (line_comment
                    (comment_content))))))))
      (end_entity
        entity: (identifier)))))

================================================================================
An entity declaration with statements:
================================================================================

entity Latch is
  port ( Din: in Word;
         Dout: out Word;
         Load: in Bit;
         Clk: in Bit );
  constant Setup: Time := 12 ns;
  constant PulseWidth: Time := 50 ns;
  use Work.TimingMonitors.all;
begin
  assert Clk='1' or Clk'Delayed'Stable (PulseWidth);
  CheckTiming (Setup, Din, Load, Clk);
end;

--------------------------------------------------------------------------------

(design_file
  (design_unit
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
                    (identifier)))))
            (interface_declaration
              (identifier_list
                (identifier))
              (simple_mode_indication
                (mode)
                (subtype_indication
                  type: (name
                    (identifier)))))
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
                    (library_type)))))))
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
                (decimal_integer)
                unit: (library_constant_unit)))))
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
                (decimal_integer)
                unit: (library_constant_unit)))))
        (use_clause
          (selected_name_list
            (selected_name
              library: (library_namespace)
              package: (identifier)
              (ALL)))))
      (entity_body
        (concurrent_assertion_statement
          (assertion
            (logical_expression
              (relational_expression
                (simple_expression
                  (name
                    (identifier)))
                (relational_operator)
                (simple_expression
                  (name
                    (library_constant_std_logic))))
              (logical_operator)
              (simple_expression
                (name
                  (identifier)
                  (attribute
                    attribute: (attribute_signal))
                  (attribute
                    attribute: (attribute_signal))
                  (parenthesis_group
                    (association_or_range_list
                      (association_element
                        (conditional_expression
                          (simple_expression
                            (name
                              (identifier))))))))))))
        (concurrent_procedure_call_statement
          (name
            (identifier)
            (parenthesis_group
              (association_or_range_list
                (association_element
                  (conditional_expression
                    (simple_expression
                      (name
                        (identifier)))))
                (association_element
                  (conditional_expression
                    (simple_expression
                      (name
                        (identifier)))))
                (association_element
                  (conditional_expression
                    (simple_expression
                      (name
                        (identifier)))))
                (association_element
                  (conditional_expression
                    (simple_expression
                      (name
                        (identifier))))))))))
      (end_entity))))
