================================================================================
Configuration
================================================================================

library TTL, Work;

configuration V4_27_87 of Processor is
  use Work.all;
  for Structure_View
    for A1: ALU
      use configuration TTL.SN74LS181;
    end for;
    for M1,M2,M3: MUX
      use entity Multiplex4 (Behavior);
    end for;
    for all: Latch
      -- use defaults
    end for;
  end for;
end configuration V4_27_87;

--------------------------------------------------------------------------------

(design_file
  (design_unit
    (library_clause
      (logical_name_list
        library: (identifier)
        library: (library_namespace)))
    (configuration_declaration
      configuration: (identifier)
      entity: (name
        (identifier))
      (configuration_head
        (use_clause
          (selected_name_list
            (selected_name
              library: (library_namespace)
              (ALL)))))
      (block_configuration
        (name
          (identifier))
        (component_configuration
          (component_specification
            (instantiation_list
              (label))
            (name
              (identifier)))
          (binding_indication
            (entity_aspect
              configuration: (name
                (identifier)
                (selection
                  (identifier)))))
          (end_for))
        (component_configuration
          (component_specification
            (instantiation_list
              (label)
              (label)
              (label))
            (name
              (identifier)))
          (binding_indication
            (entity_aspect
              entity: (name
                (identifier))
              architecture: (identifier)))
          (end_for))
        (component_configuration
          (component_specification
            (instantiation_list
              (ALL))
            (name
              (identifier)))
          (line_comment
            (comment_content))
          (end_for))
        (end_for))
      (end_configuration
        configuration: (identifier)))))

