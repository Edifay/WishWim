module.exports = grammar({
    name: 'vhdl',

    /* The external scanner tokenises all lexical elements (i.e. section 15 of
     * the VHDL-2019 standard), which makes handling case insensitivity and
     * multiple look-ahead easier.
     *
     * In addition to the lexical elements, it also tokenises often-used
     * library identifiers and built-in attributes.
     *
     * It also makes all searches binary, in contrast to parser.c that mostly
     * use linear searches (probably because it has to preserve priority).
     */
    externals: $ => [
        $.identifier,

        $.ABS,
        $.ACCESS,
        $.AFTER,
        $.ALIAS,
        $.ALL,
        $.AND,
        $.ARCHITECTURE,
        $.ARRAY,
        $.ASSERT,
        $.ASSUME, // Not used -- PSL keyword
        $.ATTRIBUTE,
        $.BEGIN,
        $.BLOCK,
        $.BODY,
        $.BUFFER,
        $.BUS,
        $.CASE,
        $.COMPONENT,
        $.CONFIGURATION,
        $.CONSTANT,
        $.CONTEXT,
        $.COVER, // Not used -- PSL keyword
        $.DEFAULT,
        $.DISCONNECT,
        $.DOWNTO,
        $.ELSE,
        $.ELSIF,
        $.END,
        $.ENTITY,
        $.EXIT,
        $.FAIRNESS, // Not used -- PSL keyword
        $.FILE,
        $.FOR,
        $.FORCE,
        $.FUNCTION,
        $.GENERATE,
        $.GENERIC,
        $.GROUP,
        $.GUARDED,
        $.IF,
        $.IMPURE,
        $.IN,
        $.INERTIAL,
        $.INOUT,
        $.IS,
        $.LABEL,
        $.LIBRARY,
        $.LINKAGE,
        $.LITERAL,
        $.LOOP,
        $.MAP,
        $.MOD,
        $.NAND,
        $.NEW,
        $.NEXT,
        $.NOR,
        $.NOT,
        $.NULL,
        $.OF,
        $.ON,
        $.OPEN,
        $.OR,
        $.OTHERS,
        $.OUT,
        $.PACKAGE,
        $.PARAMETER,
        $.PORT,
        $.POSTPONED,
        $.PROCEDURE,
        $.PROCESS,
        $.PROPERTY,
        $.PROTECTED,
        $.PRIVATE,
        $.PURE,
        $.RANGE,
        $.RECORD,
        $.REGISTER,
        $.REJECT,
        $.RELEASE,
        $.REM,
        $.REPORT,
        $.RESTRICT, // Not used -- PSL keyword
        $.RETURN,
        $.ROL,
        $.ROR,
        $.SELECT,
        $.SEQUENCE,
        $.SEVERITY,
        $.SIGNAL,
        $.SHARED,
        $.SLA,
        $.SLL,
        $.SRA,
        $.SRL,
        $.STRONG, // Not used -- PSL keyword
        $.SUBTYPE,
        $.THEN,
        $.TO,
        $.TRANSPORT,
        $.TYPE,
        $.UNAFFECTED,
        $.UNITS,
        $.UNTIL,
        $.USE,
        $.VARIABLE,
        $.VIEW,
        $.VMODE, // Not used -- PSL keyword
        $.VPKG,  // Not used -- PSL keyword
        $.VPROP, // Not used -- PSL keyword
        $.VUNIT,
        $.WAIT,
        $.WHEN,
        $.WHILE,
        $.WITH,
        $.XNOR,
        $.XOR,

        $.reserved_end_marker, // Scanner internal use only

        $.directive_body,
        $.directive_constant_builtin,
        $.directive_error,
        $.directive_protect,
        $.directive_warning,
        $._directive_newline,

        $._grave_accent,
        $.box,

        $.delimiter_end_marker, // Scanner internal use only

        $.decimal_integer,
        $.decimal_float,
        $.based_base,
        $.based_integer,
        $.based_float,
        $.character_literal,
        $.string_literal,
        $.string_literal_std_logic,
        $.bit_string_length,
        $.bit_string_base,
        $.bit_string_value,
        $.operator_symbol,
        $._line_comment_start,
        $._block_comment_start,
        $._block_comment_end,
        $.comment_content,

        $.token_end_marker, // Scanner internal use only

        $.attribute_function,
        $.attribute_impure_function,
        $.attribute_mode_view,
        $.attribute_pure_function,
        $.attribute_range,
        $.attribute_signal,
        $.attribute_subtype,
        $.attribute_type,
        $.attribute_value,

        $.library_attribute,
        $.library_constant,
        $.library_constant_boolean,
        $.library_constant_character,
        $.library_constant_debug,
        $.library_constant_env,
        $.library_constant_standard,
        $.library_constant_std_logic,
        $.library_constant_unit,
        $.library_function,
        $.library_namespace,
        $.library_type,

        $._end_of_file,

        $.error_sentinel,
    ],

    extras: $ => [
        $.line_comment,
        $.block_comment,
        $._tool_directive,
        /\s+/
    ],

    conflicts: $ => [
        [ $.generate_body ],
        [ $.case_generate_body ]
    ],

    rules: {
        // Design File
            design_file: $ => seq(
                repeat($.design_unit), $._end_of_file
            ),

            design_unit: $ => prec.right(choice(
                seq(repeat1($._context_item), optional($._library_unit)),
                $._library_unit,

                // These are useful for language injection, e.g. Markdown
                repeat1($._block_declarative_item),
                repeat1($._concurrent_statement)
            )),

        // Context Items
            _context_item: $ => choice(
                $.library_clause,
                $.use_clause,
                $.context_reference
            ),

            library_clause: $ => seq(
                alias($.LIBRARY, "library"), $.logical_name_list, ";"
            ),

            use_clause: $ => seq(
                alias($.USE, "use"), $.selected_name_list, ";"
            ),

            context_reference: $ => seq(
                alias($.CONTEXT, "context"), $.selected_name_list, ";"
            ),

            logical_name_list: $ => seq(
                $._logical_name, repeat(seq(",", $._logical_name))
            ),

            _logical_name: $ => choice(
                field("library", $.library_namespace),
                field("library", $._identifier)
            ),

            selected_name_list: $ => seq(
                $.selected_name, repeat(seq(",", $.selected_name))
            ),

            selected_name: $ => seq(
                $._logical_name, optional(seq(".", choice(field("package", $.identifier), $.ALL), repeat(seq(".", choice($._identifier, $.ALL))))),
            ),

        // Library Units
            _library_unit: $ => choice(
                $.entity_declaration,
                $.configuration_declaration,
                $.package_declaration,
                $.package_instantiation_declaration,
                $.context_declaration,

                $.architecture_definition,
                $.package_definition
            ),

            entity_declaration: $ => seq(
                alias($.ENTITY, "entity"), field("entity", $._identifier), $.entity_head, optional($.entity_body), $.end_entity, ";"
            ),

            entity_head: $ => seq(
                alias($.IS, "is"), optional($.generic_clause), optional($.port_clause), repeat($._entity_declarative_item)
            ),

            port_clause: $ => seq(
                alias($.PORT, "port"), "(", optional($.interface_list), ")", ";"
            ),

            entity_body: $ => seq(
                alias($.BEGIN, "begin"), repeat($._entity_statement)
            ),

            end_entity: $ => seq(
                alias($.END, "end"), optional(alias($.ENTITY, "entity")), optional(field("entity", $._identifier))
            ),

            configuration_declaration: $ => seq(
                alias($.CONFIGURATION, "configuration"), field("configuration", $._identifier), alias($.OF, "of"), field("entity", $.name), $.configuration_head, $.block_configuration, $.end_configuration, ";"
            ),

            configuration_head: $ => seq(
                alias($.IS, "is"), repeat($._configuration_declarative_item), repeat(seq($.verification_unit_binding_indication, ";"))
            ),

            end_configuration: $ => seq(
                alias($.END, "end"), optional(alias($.CONFIGURATION, "configuration")), optional(field("configuration", $._identifier))
            ),

            package_declaration: $ => seq(
                alias($.PACKAGE, "package"), field("package", $._identifier), $.package_declaration_body, $.end_package, ";"
            ),

            package_declaration_body: $ => seq(
                alias($.IS, "is"), optional($.package_header), repeat($._package_declarative_item)
            ),

            end_package: $ => seq(
                alias($.END, "end"), optional(alias($.PACKAGE, "package")), optional(field("package", $._identifier))
            ),

            package_instantiation_declaration: $ => seq(
                alias($.PACKAGE, "package"), field("package", $._identifier), alias($.IS, "is"), alias($.NEW, "new"), $.name, optional($.generic_map_aspect), ";"
            ),

            interface_package_declaration: $ => seq(
                alias($.PACKAGE, "package"), field("package", $._identifier), alias($.IS, "is"), alias($.NEW, "new"), $.name, $._interface_package_generic_map_aspect
            ),

            context_declaration: $ => seq(
                alias($.CONTEXT, "context"), $._identifier, $.context_declaration_body, $.end_context, ";"
            ),

            context_declaration_body: $ => seq(
                alias($.IS, "is"), repeat($._context_item)
            ),

            end_context: $ => seq(
                alias($.END, "end"), optional(alias($.CONTEXT, "context")), optional($._identifier)
            ),

            architecture_definition: $ => seq(
                alias($.ARCHITECTURE, "architecture"), field("architecture", $._identifier), alias($.OF, "of"), field("entity", $.name), $.architecture_head, $.concurrent_block, $.end_architecture, ";"
            ),

            architecture_head: $ => seq(
                alias($.IS, "is"), repeat($._block_declarative_item)
            ),

            end_architecture: $ => seq(
                alias($.END, "end"), optional(alias($.ARCHITECTURE, "architecture")), optional(field("architecture", $._identifier))
            ),

            package_definition: $ => seq(
                alias($.PACKAGE, "package"), alias($.BODY, "body"), field("package", $._identifier), $.package_definition_body, $.end_package_body, ";"
            ),

            package_definition_body: $ => seq(
                alias($.IS, "is"), repeat($._package_body_declarative_item)
            ),

            end_package_body: $ => seq(
                alias($.END, "end"), optional(seq(alias($.PACKAGE, "package"), alias($.BODY, "body"))), optional(field("package", $._identifier))
            ),

        // Declaration Item Lists
            _entity_declarative_item: $ => choice(
                $.subprogram_declaration,
                $.subprogram_definition,
                $.subprogram_instantiation_declaration,
                $.package_declaration,
                $.package_definition,
                $.package_instantiation_declaration,
                $.type_declaration,
                $.subtype_declaration,
                $.mode_view_declaration,
                $.constant_declaration,
                $.signal_declaration,
                $.variable_declaration,
                $.file_declaration,
                $.alias_declaration,
                $.attribute_declaration,
                $.attribute_specification,
                $.disconnection_specification,
                $.use_clause,
                $.group_template_declaration,
                $.group_declaration
            ),

            _package_declarative_item: $ => choice(
                $.subprogram_declaration,
                $.subprogram_instantiation_declaration,
                $.package_declaration,
                $.package_instantiation_declaration,
                $.type_declaration,
                $.subtype_declaration,
                $.mode_view_declaration,
                $.constant_declaration,
                $.signal_declaration,
                $.variable_declaration,
                $.file_declaration,
                $.alias_declaration,
                $.component_declaration,
                $.attribute_declaration,
                $.attribute_specification,
                $.disconnection_specification,
                $.use_clause,
                $.group_template_declaration,
                $.group_declaration
            ),

            _package_body_declarative_item: $ => choice(
                $.subprogram_declaration,
                $.subprogram_definition,
                $.subprogram_instantiation_declaration,
                $.package_declaration,
                $.package_definition,
                $.package_instantiation_declaration,
                $.type_declaration,
                $.subtype_declaration,
                $.constant_declaration,
                $.variable_declaration,
                $.file_declaration,
                $.alias_declaration,
                $.attribute_declaration,
                $.attribute_specification,
                $.use_clause,
                $.group_template_declaration,
                $.group_declaration
            ),

            _protected_type_declarative_item: $ => choice(
                $.subprogram_declaration,
                $.subprogram_instantiation_declaration,
                $.attribute_specification,
                $.use_clause,
                $.private_variable_declaration,
                $.alias_declaration
            ),

            _protected_type_body_declarative_item: $ => choice(
                $.subprogram_declaration,
                $.subprogram_definition,
                $.subprogram_instantiation_declaration,
                $.package_declaration,
                $.package_definition,
                $.package_instantiation_declaration,
                $.type_declaration,
                $.subtype_declaration,
                $.constant_declaration,
                $.variable_declaration,
                $.file_declaration,
                $.alias_declaration,
                $.attribute_declaration,
                $.attribute_specification,
                $.use_clause,
                $.group_template_declaration,
                $.group_declaration
            ),

            _process_declarative_item: $ => choice(
                $.subprogram_declaration,
                $.subprogram_definition,
                $.subprogram_instantiation_declaration,
                $.package_declaration,
                $.package_definition,
                $.package_instantiation_declaration,
                $.type_declaration,
                $.subtype_declaration,
                $.constant_declaration,
                $.variable_declaration,
                $.file_declaration,
                $.alias_declaration,
                $.attribute_declaration,
                $.attribute_specification,
                $.use_clause,
                $.group_template_declaration,
                $.group_declaration
            ),

            _subprogram_declarative_item: $ => choice(
                $.subprogram_declaration,
                $.subprogram_definition,
                $.subprogram_instantiation_declaration,
                $.package_declaration,
                $.package_definition,
                $.package_instantiation_declaration,
                $.type_declaration,
                $.subtype_declaration,
                $.constant_declaration,
                $.variable_declaration,
                $.file_declaration,
                $.alias_declaration,
                $.attribute_declaration,
                $.attribute_specification,
                $.use_clause,
                $.group_template_declaration,
                $.group_declaration
            ),

            _block_declarative_item: $ => prec(1, choice(
                $.subprogram_declaration,
                $.subprogram_definition,
                $.subprogram_instantiation_declaration,
                $.package_declaration,
                $.package_definition,
                $.package_instantiation_declaration,
                $.type_declaration,
                $.subtype_declaration,
                $.mode_view_declaration,
                $.constant_declaration,
                $.signal_declaration,
                $.variable_declaration,
                $.file_declaration,
                $.alias_declaration,
                $.component_declaration,
                $.attribute_declaration,
                $.attribute_specification,
                $.configuration_specification,
                $.disconnection_specification,
                $.use_clause,
                $.group_template_declaration,
                $.group_declaration
            )),

        // Declarations
            subprogram_declaration: $ => seq(
                $._subprogram_specification, ";"
            ),

            subprogram_definition: $ => seq(
                $._subprogram_specification, $.subprogram_head, $.sequential_block, $.subprogram_end, ";"
            ),

            subprogram_head: $ => seq(
                alias($.IS, "is"), repeat($._subprogram_declarative_item)
            ),

            subprogram_end: $ => seq(
                alias($.END, "end"), optional(choice(alias($.PROCEDURE, "procedure"), alias($.FUNCTION, "function"))), optional(field("function", $._designator))
            ),

            subprogram_instantiation_declaration: $ => seq(
                choice(
                    seq(alias($.PROCEDURE, "procedure"), field("procedure", $._designator)),
                    seq(alias($.FUNCTION, "function"), field("function", $._designator))
                ), alias($.IS, "is"), alias($.NEW, "new"), $.name, optional($.signature), optional($.generic_map_aspect), ";"
            ),

            type_declaration: $ => seq(
                alias($.TYPE, "type"), field("type", $._identifier), optional(seq(alias($.IS, "is"), $._type_definition)), ";"
            ),

            subtype_declaration: $ => seq(
                alias($.SUBTYPE, "subtype"), field("type", $._identifier), alias($.IS, "is"), $.subtype_indication, ";"
            ),

            mode_view_declaration: $ => seq(
                alias($.VIEW, "view"), field("view", $._identifier), alias($.OF, "of"), $.subtype_indication, $.mode_view_body, $.end_view, ";"
            ),

            mode_view_body: $ => seq(
                alias($.IS, "is"), repeat($.mode_view_element_definition)
            ),

            end_view: $ => seq(
                alias($.END, "end"), alias($.VIEW, "view"), optional(field("view", $._identifier))
            ),

            constant_declaration: $ => seq(
                alias($.CONSTANT, "constant"), alias($.constant_identifier_list, $.identifier_list), ":", $.subtype_indication, optional($.initialiser), ";"
            ),

            signal_declaration: $ => seq(
                alias($.SIGNAL, "signal"), $.identifier_list, ":", $.subtype_indication, optional($.signal_kind), optional($.initialiser), ";"
            ),

            signal_kind: $ => choice(
                alias($.REGISTER, "register"),
                alias($.BUS, "bus")
            ),

            variable_declaration: $ => seq(
                optional(alias($.SHARED, "shared")), alias($.VARIABLE, "variable"), $.identifier_list, ":", $.subtype_indication, optional($.generic_map_aspect), optional($.initialiser), ";"
            ),

            initialiser: $ => seq(
                $.variable_assignment, $.conditional_expression
            ),

            file_declaration: $ => seq(
                alias($.FILE, "file"), $.identifier_list, ":", $.subtype_indication, optional($.file_open_information), ";"
            ),

            interface_file_declaration: $ => seq(
                alias($.FILE, "file"), $.identifier_list, ":", $.subtype_indication
            ),

            alias_declaration: $ => seq(
                alias($.ALIAS, "alias"), $._alias_designator, optional(seq(":", $.subtype_indication)), alias($.IS, "is"), $.name, ";"
            ),

            component_declaration: $ => seq(
                alias($.COMPONENT, "component"), field("component", $._identifier), optional($.component_body), $.end_component, ";"
            ),

            component_body: $ => choice(
                seq(alias($.IS, "is"), optional($.generic_clause), optional($.port_clause)),
                seq($.generic_clause, optional($.port_clause)),
                seq($.port_clause)
            ),

            end_component: $ => seq(
                alias($.END, "end"), optional(alias($.COMPONENT, "component")), optional(field("component", $._identifier))
            ),

            attribute_declaration: $ => seq(
                alias($.ATTRIBUTE, "attribute"), field("attribute", $._identifier), ":", field("type", $.name), ";"
            ),

            attribute_specification: $ => seq(
                alias($.ATTRIBUTE, "attribute"), $._attribute_designator, alias($.OF, "of"), $.entity_specification, alias($.IS, "is"), $.conditional_expression, ";"
            ),

            disconnection_specification: $ => seq(
                alias($.DISCONNECT, "disconnect"), $.guarded_signal_specification, alias($.AFTER, "after"), $._expression, ";"
            ),

            group_template_declaration: $ => seq(
                alias($.GROUP, "group"), $._identifier, alias($.IS, "is"), "(", $.entity_class_entry_list, ")", ";"
            ),

            group_declaration: $ => seq(
                alias($.GROUP, "group"), $._identifier, ":", $.name, "(", $.group_constituent_list, ")", ";"
            ),

            private_variable_declaration: $ => seq(
                alias($.PRIVATE, "private"), $.variable_declaration
            ),

        // Type Definitions
            _interface_type_indication: $ => choice(
                $.subtype_indication,
                $.unspecified_type_indication
            ),

            unspecified_type_indication: $ => seq(
                alias($.TYPE, "type"), alias($.IS, "is"), $._incomplete_type_definition
            ),

            interface_type_declaration: $ => seq(
                alias($.TYPE, "type"), field("type", $._identifier), optional(seq(alias($.IS, "is"), $._incomplete_type_definition))
            ),

            _incomplete_type_definition: $ => choice(
                $.private_incomplete_type_definition,
                $.scalar_incomplete_type_definition,
                $.discrete_incomplete_type_definition,
                $.integer_incomplete_type_definition,
                $.physical_incomplete_type_definition,
                $.floating_incomplete_type_definition,
                $.array_type_definition,
                $.access_incomplete_type_definition,
                $.file_incomplete_type_definition
            ),

            private_incomplete_type_definition: $ => alias($.PRIVATE, "private"),

            scalar_incomplete_type_definition: $ => alias($.box, "<>"),

            discrete_incomplete_type_definition: $ => seq(
                "(", alias($.box, "<>"), ")"
            ),

            integer_incomplete_type_definition: $ => seq(
                alias($.RANGE, "range"), alias($.box, "<>")
            ),

            physical_incomplete_type_definition: $ => seq(
                alias($.UNITS, "units"), alias($.box, "<>")
            ),

            floating_incomplete_type_definition: $ => seq(
                alias($.RANGE, "range"), alias($.box, "<>"), ".", alias($.box, "<>")
            ),

            array_type_definition: $ => choice(
                seq(alias($.ARRAY, "array"), "(", $.array_index_incomplete_type_list, ")", alias($.OF, "of"), $._incomplete_subtype_indication),
                seq(alias($.ARRAY, "array"), $.index_constraint, alias($.OF, "of"), $.subtype_indication)
            ),

            array_index_incomplete_type_list: $ => seq(
                $._array_index_incomplete_type, repeat(seq(",", $._array_index_incomplete_type))
            ),

            _array_index_incomplete_type: $ => choice(
                $.index_subtype_definition,
                $.index_constraint,
                $.unspecified_type_indication
            ),

            index_constraint: $ => prec.left(seq(
                $._range, repeat(seq(",", $._range))
            )),

            index_subtype_definition: $ => seq(
                field("type", $.name), alias($.RANGE, "range"), alias($.box, "<>")
            ),

            access_type_definition: $ => seq(
                alias($.ACCESS, "access"), $.subtype_indication, optional($.generic_map_aspect)
            ),

            access_incomplete_type_definition: $ => seq(
                alias($.ACCESS, "access"), $._incomplete_subtype_indication
            ),

            file_type_definition: $ => seq(
                alias($.FILE, "file"), alias($.OF, "of"), field("type", $.name)
            ),

            file_incomplete_type_definition: $ => seq(
                alias($.FILE, "file"), alias($.OF, "of"), $._incomplete_type_mark
            ),

            subtype_indication: $ => seq(
                optional($.resolution_indication), field("type", $.name), optional($.range_constraint)
            ),

            resolution_indication: $ => choice(
                $.name,
                seq("(", $._element_resolution, ")")
            ),

            _element_resolution: $ => choice(
                $.resolution_indication,
                $.record_resolution
            ),

            record_resolution: $ => seq(
                $.record_element_resolution, repeat(seq(",", $.record_element_resolution))
            ),

            record_element_resolution: $ => seq(
                $._identifier, $.resolution_indication
            ),

            _incomplete_subtype_indication: $ => choice(
                $.subtype_indication,
                $.unspecified_type_indication
            ),

            _incomplete_type_mark: $ => choice(
                field("type", $.name),
                $.unspecified_type_indication
            ),

        // Statement Blocks
            concurrent_block: $ => seq(
                alias($.BEGIN, "begin"), repeat($._concurrent_statement)
            ),

            sequential_block: $ => seq(
                alias($.BEGIN, "begin"), repeat($._sequential_statement)
            ),

            generate_body: $ => choice(
                seq(alias($.GENERATE, "generate"), optional($.generate_block)),
                seq(alias($.GENERATE, "generate"), optional($.generate_head), alias($.BEGIN, "begin"), optional($.generate_block), optional($.generate_block_end))
            ),

            generate_head: $ => seq(
                repeat1($._block_declarative_item)
            ),

            case_generate_alternative: $ => prec.left(seq(
                alias($.WHEN, "when"), optional($.label_declaration), $._element, $.case_generate_body
            )),

            case_generate_body: $ => choice(
                seq("=>", optional($.generate_block)),
                seq("=>", optional($.generate_head), alias($.BEGIN, "begin"), optional($.generate_block), optional($.generate_block_end))
            ),

            generate_block: $ => seq(
                repeat1($._concurrent_statement)
            ),

            generate_block_end: $ => seq(
                alias($.END, "end"), optional($._label), ";"
            ),

        // Statements
            _entity_statement: $ => choice(
                $.concurrent_assertion_statement,
                $.concurrent_procedure_call_statement,
                $.process_statement
            ),

            _concurrent_statement: $ => choice(
                $.component_instantiation_statement,

                $.for_generate_statement,
                $.if_generate_statement,
                $.case_generate_statement,

                $.concurrent_assertion_statement,
                $.concurrent_procedure_call_statement,
                $.concurrent_simple_signal_assignment,
                $.concurrent_conditional_signal_assignment,
                $.concurrent_selected_signal_assignment,

                $.block_statement,
                $.process_statement
            ),

            _sequential_statement: $ => choice(
                $.assertion_statement,
                $.case_statement,
                $.exit_statement,
                $.if_statement_block,
                $.loop_statement,
                $.next_statement,
                $.null_statement,
                $.procedure_call_statement,
                $.report_statement,
                $.return_statement,
                $.sequential_block_statement,
                $.simple_waveform_assignment,
                $.simple_force_assignment,
                $.simple_release_assignment,
                $.conditional_signal_assignment,
                $.selected_waveform_assignment,
                $.selected_force_assignment,
                $.simple_variable_assignment,
                $.selected_variable_assignment,
                $.wait_statement
            ),

            block_statement: $ => seq(
                $.label_declaration, alias($.BLOCK, "block"), optional($.guard_condition), optional($.block_head), $.concurrent_block, $.end_block, ";"
            ),

            guard_condition: $ => seq(
                "(", $._expression, ")"
            ),

            block_head: $ => choice(
                seq(alias($.IS, "is"), optional(seq($.generic_clause, optional(seq($.generic_map_aspect, ";")))), optional(seq($.port_clause, optional(seq($.port_map_aspect, ";")))), repeat($._block_declarative_item)),
                seq($.generic_clause, optional(seq($.generic_map_aspect, ";")), optional(seq($.port_clause, optional(seq($.port_map_aspect, ";")))), repeat($._block_declarative_item)),
                seq($.port_clause, optional(seq($.port_map_aspect, ";")), repeat($._block_declarative_item)),
                seq(repeat1($._block_declarative_item))
            ),

            end_block: $ => seq(
                alias($.END, "end"), alias($.BLOCK, "block"), optional($._label)
            ),

            sequential_block_statement: $ => seq(
                optional($.label_declaration), alias($.BLOCK, "block"), optional($.sequential_block_head), $.sequential_block, $.end_block, ";"
            ),

            sequential_block_head: $ => choice(
                seq(alias($.IS, "is"), repeat($._process_declarative_item)),
                seq(repeat1($._process_declarative_item))
            ),

            component_instantiation_statement: $ => choice(
                seq($.label_declaration, $.instantiated_unit,        optional($.generic_map_aspect), optional($.port_map_aspect), ";"),
                seq($.label_declaration, field("component", $.name), optional($.generic_map_aspect),          $.port_map_aspect,  ";")
            ),

            instantiated_unit: $ => choice(
                seq(alias($.COMPONENT, "component"), field("component", $.name)), // Optional "component" covered by procedure call
                seq(alias($.ENTITY, "entity"), optional(seq(field("library", $.library_namespace), ".")), field("entity", $.name), optional(seq("(", field("architecture", $._identifier), ")"))),
                seq(alias($.CONFIGURATION, "configuration"), field("configuration", $.name))
            ),

            process_statement: $ => seq(
                optional($.label_declaration), optional(alias($.POSTPONED, "postponed")), alias($.PROCESS, "process"), optional($.sensitivity_specification), optional($.process_head), $.sequential_block, $.end_process, ";"
            ),

            sensitivity_specification: $ => seq(
                "(", $._process_sensitivity_list, ")"
            ),

            process_head: $ => choice(
                seq(alias($.IS, "is"), repeat($._process_declarative_item)),
                seq(     repeat1($._process_declarative_item)),
            ),

            end_process: $ => seq(
                alias($.END, "end"), optional(alias($.POSTPONED, "postponed")), alias($.PROCESS, "process"), optional($._label)
            ),

            case_generate_statement: $ => seq(
                $.label_declaration, alias($.CASE, "case"), $._expression, alias($.GENERATE, "generate"), $.case_generate_block, $.end_generate, ";"
            ),

            case_generate_block: $ => seq(
                repeat1($.case_generate_alternative)
            ),

            for_generate_statement: $ => seq(
                $.label_declaration, $.for_loop, $.generate_body, $.end_generate, ";"
            ),

            if_generate_statement: $ => seq(
                $.label_declaration, $.if_generate, $.end_generate, ";"
            ),

            if_generate: $ => seq(
                alias($.IF, "if"), optional($.label_declaration), $._expression, $.generate_body, optional($._elsif_else_generate)
            ),

            _elsif_else_generate: $ => choice(
                $.elsif_generate,
                $.else_generate
            ),

            elsif_generate: $ => seq(
                alias($.ELSIF, "elsif"), optional($.label_declaration), $._expression, $.generate_body, optional($._elsif_else_generate)
            ),

            else_generate: $ => seq(
                alias($.ELSE, "else"), optional($.label_declaration), $.generate_body
            ),

            end_generate: $ => seq(
                alias($.END, "end"), alias($.GENERATE, "generate"), optional($._label)
            ),

            assertion_statement: $ => seq(
                optional($.label_declaration), $.assertion, ";"
            ),

            concurrent_assertion_statement: $ => seq(
                optional($.label_declaration), optional(alias($.POSTPONED, "postponed")), $.assertion, ";"
            ),

            assertion: $ => seq(
                alias($.ASSERT, "assert"), $._expression, optional($.report_expression), optional($.severity_expression)
            ),

            report_expression: $ => seq(
                alias($.REPORT, "report"), $._expression
            ),

            severity_expression: $ => seq(
                alias($.SEVERITY, "severity"), $._expression
            ),

            case_statement: $ => seq(
                optional($.label_declaration), $.case_expression, $.case_body, $.end_case, ";"
            ),

            case_expression: $ => seq(
                alias($.CASE, "case"), optional("?"), $._expression
            ),

            case_body: $ => seq(
                alias($.IS, "is"), repeat1($.case_statement_alternative)
            ),

            end_case: $ => seq(
                alias($.END, "end"), alias($.CASE, "case"), optional("?"), optional($._label)
            ),

            exit_statement: $ => seq(
                optional($.label_declaration), alias($.EXIT, "exit"), optional($._label), optional($.when_expression), ";"
            ),

            when_expression: $ => seq(
                alias($.WHEN, "when"), $._expression
            ),

            if_statement_block: $ => seq(
                optional($.label_declaration), $.if_statement, $.end_if, ";"
            ),

            if_statement: $ => seq(
                alias($.IF, "if"), $._expression, alias($.THEN, "then"), optional($.if_statement_body), optional($._elsif_else_statement)
            ),

            _elsif_else_statement: $ => choice(
                $.elsif_statement,
                $.else_statement
            ),

            elsif_statement: $ => seq(
                alias($.ELSIF, "elsif"), $._expression, alias($.THEN, "then"), optional($.if_statement_body), optional($._elsif_else_statement)
            ),

            else_statement: $ => seq(
                alias($.ELSE, "else"), optional($.if_statement_body)
            ),

            if_statement_body: $ => seq(
                repeat1($._sequential_statement)
            ),

            end_if: $ => seq(
                alias($.END, "end"), alias($.IF, "if"), optional($._label)
            ),

            loop_statement: $ => seq(
                optional($.label_declaration), optional($._iteration_scheme), $.loop_body, $.end_loop, ";"
            ),

            _iteration_scheme: $ => choice(
                $.while_loop,
                $.for_loop
            ),

            loop_body: $ => seq(
                alias($.LOOP, "loop"), repeat($._sequential_statement)
            ),

            end_loop: $ => seq(
                alias($.END, "end"), alias($.LOOP, "loop"), optional($._label)
            ),

            next_statement: $ => seq(
                optional($.label_declaration), alias($.NEXT, "next"), optional($._label), optional($.when_expression), ";"
            ),

            null_statement: $ => seq(
                optional($.label_declaration), alias($.NULL, "null"), ";"
            ),

            procedure_call_statement: $ => seq(
                optional($.label_declaration), $.name, ";"
            ),

            concurrent_procedure_call_statement: $ => seq(
                optional($.label_declaration), optional(alias($.POSTPONED, "postponed")), $.name, ";"
            ),

            report_statement: $ => seq(
                optional($.label_declaration), $.report_expression, optional($.severity_expression), ";"
            ),

            return_statement: $ => choice(
                seq(optional($.label_declaration), alias($.RETURN, "return"), optional($.when_expression), ";"),
                seq(optional($.label_declaration), alias($.RETURN, "return"), $.conditional_or_unaffected_expression, ";")
            ),

            signal_assignment: $ => "<=",

            simple_waveform_assignment: $ => seq(
                optional($.label_declaration), $._target, $.signal_assignment, optional($.delay_mechanism), $.waveform, ";"
            ),

            variable_assignment: $ => ":=",

            simple_variable_assignment: $ => seq(
                optional($.label_declaration), $._target, $.variable_assignment, $.conditional_or_unaffected_expression, ";"
            ),

            concurrent_simple_signal_assignment: $ => seq(
                optional($.label_declaration), optional(alias($.POSTPONED, "postponed")), $._target, $.signal_assignment, optional(alias($.GUARDED, "guarded")), optional($.delay_mechanism), $.waveform, ";"
            ),

            simple_force_assignment: $ => seq(
                optional($.label_declaration), $._target, $.signal_assignment, alias($.FORCE, "force"), optional($.force_mode), $.conditional_or_unaffected_expression, ";"
            ),

            simple_release_assignment: $ => seq(
                optional($.label_declaration), $._target, $.signal_assignment, alias($.RELEASE, "release"), optional($.force_mode), ";"
            ),

            conditional_signal_assignment: $ => seq(
                optional($.label_declaration), $._target, $.signal_assignment, optional($.delay_mechanism), $.conditional_waveforms, ";"
            ),

            concurrent_conditional_signal_assignment: $ => seq(
                optional($.label_declaration), optional(alias($.POSTPONED, "postponed")), $._target, $.signal_assignment, optional(alias($.GUARDED, "guarded")), optional($.delay_mechanism), $.conditional_waveforms, ";"
            ),

            selected_waveform_assignment: $ => seq(
                optional($.label_declaration), $.with_expression, $.select_target, $.signal_assignment, optional($.delay_mechanism), $.selected_waveforms, ";"
            ),

            concurrent_selected_signal_assignment: $ => seq(
                optional($.label_declaration), optional(alias($.POSTPONED, "postponed")), $.with_expression, $.select_target, $.signal_assignment, optional(alias($.GUARDED, "guarded")), optional($.delay_mechanism), $.selected_waveforms, ";"
            ),

            selected_force_assignment: $ => seq(
                optional($.label_declaration), $.with_expression, $.select_target, $.signal_assignment, alias($.FORCE, "force"), optional($.force_mode), $.selected_expressions, ";"
            ),

            selected_variable_assignment: $ => seq(
                optional($.label_declaration), $.with_expression, $.select_target, $.variable_assignment, $.selected_expressions, ";"
            ),

            with_expression: $ => seq(
                alias($.WITH, "with"), $._expression
            ),

            select_target: $ => seq(
                alias($.SELECT, "select"), optional("?"), $._target
            ),

            wait_statement: $ => seq(
                optional($.label_declaration), alias($.WAIT, "wait"), optional($.sensitivity_clause), optional($.condition_clause), optional($.timeout_clause), ";"
            ),

        // Expressions
            conditional_expression: $ => prec(9, seq(
                $._expression, repeat(seq($.when_expression, $.else_expression))
            )),

            else_expression: $ => seq(
                alias($.ELSE, "else"), $._expression
            ),

            _expression: $ => prec(10, choice(
                $.condition_expression,
                $.logical_expression,
                $.relational_expression,
                $.shift_expression,
                $.simple_expression
            )),

            condition_expression: $ => prec(11, seq(
                $.condition_conversion, $._expression,
            )),

            condition_conversion: $ => "??",

            logical_expression: $ => prec.left(12, seq(
                $._expression, $.logical_operator, $._expression,
            )),

            relational_expression: $ => prec.left(13, seq(
                $._expression, $.relational_operator, $._expression,
            )),

            shift_expression: $ => prec.left(14, seq(
                $._expression, $.shift_operator, $._expression,
            )),

            simple_expression: $ => choice(
                prec.left(15, seq($._expression, $.adding_operator,      $._expression)),
                prec     (16, seq(               $.sign,                 $._expression)),
                prec.left(17, seq($._expression, $.multiplying_operator, $._expression)),
                prec.left(18, seq($._expression, $.exponentiate,         $._expression)),
                prec     (19, seq(               $.unary_operator,       $._expression)),
                prec     (20, $._primary)
            ),

            unary_operator: $ => choice(
                alias($.ABS,  "abs"),
                alias($.NOT,  "not"),
                alias($.AND,  "and"),
                alias($.OR,   "or"),
                alias($.NAND, "nand"),
                alias($.NOR,  "nor"),
                alias($.XOR,  "xor"),
                alias($.XNOR, "xnor")
            ),

            logical_operator: $ => choice(
                alias($.AND,  "and"),
                alias($.OR,   "or"),
                alias($.NAND, "nand"),
                alias($.NOR,  "nor"),
                alias($.XOR,  "xor"),
                alias($.XNOR, "xnor"),
            ),

            relational_operator: $ => choice(
                "=",
                "/=",
                "<",
                "<=",
                ">",
                ">=",
                "?=",
                "?/=",
                "?<",
                "?<=",
                "?>",
                "?>="
            ),

            shift_operator: $ => choice(
                alias($.SLL, "sll"),
                alias($.SRL, "srl"),
                alias($.SLA, "sla"),
                alias($.SRA, "sra"),
                alias($.ROL, "rol"),
                alias($.ROR, "ror")
            ),

            sign: $ => choice(
                "+",
                "-"
            ),

            adding_operator: $ => choice(
                "+",
                "-",
                "&"
            ),

            multiplying_operator: $ => choice(
                "*",
                "/",
                alias($.MOD, "mod"),
                alias($.REM, "rem")
            ),

            exponentiate: $ => "**",

            _primary: $ => choice(
                $.name,
                $._literal,
                $.allocator,
                $.parenthesis_expression
            ),

            name: $ => prec.left(21, seq(
                seq($._direct_name, repeat($._name_selector)),
            )),

            _direct_name: $ => prec.left(choice(
                $._identifier,
                $.operator_symbol,
                $.character_literal,
                $.library_constant_std_logic,
                $._external_name,
            )),

            _label: $ => choice(
                alias($.identifier,       $.label),
                alias($.library_constant, $.label),
                alias($.library_function, $.label),
                alias($.library_type,     $.label),
            ),

            _attribute: $ => choice(
                alias($.identifier,       $.attribute_identifier),
                alias($.library_constant, $.attribute_identifier),
                alias($.library_function, $.attribute_identifier),
                alias($.library_type,     $.attribute_identifier),
            ),

            _unit: $ => choice(
                alias($.identifier,       $.unit),
                alias($.library_constant, $.unit),
                alias($.library_function, $.unit),
                alias($.library_type,     $.unit),
                $.library_constant_unit
            ),

            _identifier: $ => choice(
                $.identifier,
                $.library_constant,
                $.library_function,
                $.library_type,
            ),

            _name_selector: $ => choice(
                $.function_call,
                $.parenthesis_group,
                $.attribute,
                $.signature,
                $.selection
            ),

            _external_name: $ => choice(
                $.external_constant_name,
                $.external_signal_name,
                $.external_variable_name
            ),

            external_constant_name: $ => seq(
                "<<", alias($.CONSTANT, "constant"), $._external_pathname, ":", $._interface_type_indication, ">>"
            ),

            external_signal_name: $ => seq(
                "<<", alias($.SIGNAL, "signal"), $._external_pathname, ":", $._interface_type_indication, ">>"
            ),

            external_variable_name: $ => seq(
                "<<", alias($.VARIABLE, "variable"), $._external_pathname, ":", $._interface_type_indication, ">>"
            ),

            _external_pathname: $ => choice(
                $.package_pathname,
                $.absolute_pathname,
                $.relative_pathname
            ),

            package_pathname: $ => seq(
                "@", $._identifier, ".", $._identifier, ".", repeat(seq($._identifier, ".")), $._identifier
            ),

            absolute_pathname: $ => seq(
                ".", $.partial_pathname
            ),

            relative_pathname: $ => seq(
                repeat(seq("^", ".")), $.partial_pathname
            ),

            partial_pathname: $ => seq(
                repeat(seq($.pathname_element, ".")), $._identifier
            ),

            pathname_element: $ => seq(
              $._identifier, optional(seq("(", $._expression, ")"))
            ),

            function_call: $ => prec.right(choice(
                seq(optional($.generic_map_aspect), alias($.PARAMETER, "parameter"), alias($.MAP, "map"), $.parenthesis_group),
                seq($.generic_map_aspect, optional(seq(optional(seq(alias($.PARAMETER, "parameter"), alias($.MAP, "map"))), $.parenthesis_group)))
            )),

            parenthesis_group: $ => seq(
                "(",
                optional($.association_or_range_list),
                ")",
            ),

            association_or_range_list: $ => seq(
                $._association_or_range, repeat(seq(",", $._association_or_range))
            ),

            _association_or_range: $ => choice(
                $.association_element,
                $._range
            ),

            association_element: $ => choice(
                $._actual_part,
                prec.left(6, seq($.name, "=>", $._actual_part))
            ),

            _actual_part: $ => prec(21, choice(
                seq(optional(alias($.INERTIAL, "inertial")), $.conditional_expression),
                $.OPEN
            )),

            attribute: $ => seq(
                "'",
                choice(
                    $._attribute_designator,
                    $.parenthesis_expression // qualified_expression
                ),
            ),

            _attribute_designator: $ => choice(
                field("attribute", $._attribute),
                field("attribute", $.attribute_function),
                field("attribute", $.attribute_impure_function),
                field("attribute", $.attribute_mode_view),
                field("attribute", $.attribute_pure_function),
                field("attribute", $.attribute_range),
                field("attribute", $.attribute_signal),
                field("attribute", $.attribute_subtype),
                field("attribute", $.attribute_type),
                field("attribute", $.attribute_value),
                field("attribute", $.library_attribute)
            ),

            signature: $ => seq(
                "[", optional(seq(field("type", $.name), repeat(seq(",", field("type", $.name))))), optional(seq(alias($.RETURN, "return"), field("type", $.name))), "]"
            ),

            selection: $ => seq(
                ".", $._suffix
            ),

            _suffix: $ => choice(
                $.identifier,
                $.character_literal,
                $.operator_symbol,
                $.ALL
            ),

            _literal: $ => choice(
                seq($._abstract_literal, optional(field("unit", $._unit))),
                $.bit_string_literal,
                $.string_literal,
                $.string_literal_std_logic,
                $.library_constant_boolean,
                $.library_constant_character,
                $.library_constant_debug,
                $.library_constant_env,
                $.library_constant_standard,
                alias($.NULL, "null")
            ),

            bit_string_literal: $ => seq(
                optional($.bit_string_length),
                $.bit_string_base,
                $.bit_string_value
            ),

            _abstract_literal: $ => choice(
                $.decimal_integer,
                $.decimal_float,
                $.based_literal,
            ),

            based_literal: $ => seq(
                $.based_base, choice($.based_integer, $.based_float)
            ),

            allocator: $ => seq(
                alias($.NEW, "new"), $.name
            ),

            parenthesis_expression: $ => seq(
                "(", $.element_association_list, ")",
            ),

            element_association_list: $ => seq(
                $.element_association, repeat(seq(",", $.element_association))
            ),

            element_association: $ => choice(
                $.conditional_expression,
                prec(6, seq($._element, "=>", $.conditional_expression))
            ),

            _element: $ => choice(
                $.simple_expression,
                $._range,
                $.OTHERS,
                $.choices
            ),

            choices: $ => prec.left(7, seq(
                $._element, "|", $._element
            )),

            _range: $ => choice(
                $._expression,
                $.simple_range
            ),

            simple_range: $ => prec.left(8, seq(
                $.simple_expression, $._direction, $.simple_expression
            )),

            _direction: $ => choice(
                alias($.TO, "to"),
                alias($.DOWNTO, "downto")
            ),

        // Tool Directives
            _tool_directive: $ => seq(
                $._grave_accent, $._directive, $._directive_newline
            ),

            _directive: $ => choice(
                $.user_directive,
                $.protect_directive,
                $.warning_directive,
                $.error_directive,

                $.if_conditional_analysis,
                $.elsif_conditional_analysis,
                $.else_conditional_analysis,
                $.end_conditional_analysis
            ),

            user_directive: $ => seq(
                $._identifier, repeat($.directive_body)
            ),

            protect_directive: $ => seq(
                $.directive_protect, repeat($.directive_body),
            ),

            warning_directive: $ => seq(
                $.directive_warning, $.string_literal,
            ),

            error_directive: $ => seq(
                $.directive_error, $.string_literal,
            ),

            if_conditional_analysis: $ => seq(
                alias($.IF, "if"), $.conditional_analysis_expression, alias($.THEN, "then")
            ),

            elsif_conditional_analysis: $ => seq(
                alias($.ELSIF, "elsif"), $.conditional_analysis_expression, alias($.THEN, "then")
            ),

            else_conditional_analysis: $ => seq(
                alias($.ELSE, "else")
            ),

            end_conditional_analysis: $ => seq(
                alias($.END, "end"), optional(alias($.IF, "if"))
            ),

            conditional_analysis_expression: $ => seq(
                $.conditional_analysis_relation, repeat(seq($.logical_operator, $.conditional_analysis_relation))
            ),

            conditional_analysis_relation: $ => choice(
                seq(optional(alias($.NOT, "not")), "(", $.conditional_analysis_expression, ")"),
                seq($._conditional_analysis_identifier, $._conditional_analysis_operator, $.string_literal)
            ),

            _conditional_analysis_operator: $ => choice(
                "=",
                "/=",
                "<",
                "<=",
                ">",
                ">="
            ),

            _conditional_analysis_identifier: $ => choice(
                $.directive_constant_builtin,
                $._identifier
            ),

        // Configuration
            _configuration_declarative_item: $ => choice(
                $.use_clause,
                $.attribute_specification,
                $.group_declaration
            ),

            _configuration_item: $ => choice(
                $.block_configuration,
                $.component_configuration
            ),

            block_configuration: $ => seq(
                alias($.FOR, "for"), $.name, repeat($.use_clause), repeat($._configuration_item), $.end_for, ";"
            ),

            component_configuration: $ => seq(
                alias($.FOR, "for"), $.component_specification, optional($.binding_indication), repeat(seq($.verification_unit_binding_indication, ";")), optional($.block_configuration), $.end_for, ";"
            ),

            end_for: $ => seq(
                alias($.END, "end"), alias($.FOR, "for")
            ),

        // Interface Declaration
            generic_interface_list: $ => seq(
                $._generic_interface_declaration, repeat(seq(";", $._generic_interface_declaration)), optional(";")
            ),

            _generic_interface_declaration: $ => choice(
                alias($.generic_interface_declaration, $.interface_declaration),
                $.interface_constant_declaration,
                $.interface_signal_declaration,
                $.interface_variable_declaration,
                $.interface_file_declaration,

                $.interface_type_declaration,
                $.interface_subprogram_declaration,
                $.interface_package_declaration
            ),

            generic_interface_declaration: $ => seq(
                alias($.generic_identifier_list, $.identifier_list), ":", $._mode_indication
            ),

            interface_list: $ => seq(
                $._interface_declaration, repeat(seq(";", $._interface_declaration)), optional(";")
            ),

            _interface_declaration: $ => choice(
                $.interface_declaration,
                $.interface_constant_declaration,
                $.interface_signal_declaration,
                $.interface_variable_declaration,
                $.interface_file_declaration,

                $.interface_type_declaration,
                $.interface_subprogram_declaration,
                $.interface_package_declaration
            ),

            interface_declaration: $ => seq(
                $.identifier_list, ":", $._mode_indication
            ),

            interface_constant_declaration: $ => seq(
                alias($.CONSTANT, "constant"), alias($.constant_identifier_list, $.identifier_list), ":", $._mode_indication
            ),

            interface_signal_declaration: $ => seq(
                alias($.SIGNAL, "signal"), $.identifier_list, ":", $._mode_indication
            ),

            interface_variable_declaration: $ => seq(
                alias($.VARIABLE, "variable"), $.identifier_list, ":", $._mode_indication
            ),

            interface_subprogram_declaration: $ => seq(
                $._interface_subprogram_specification, optional(seq(alias($.IS, "is"), $._interface_subprogram_default))
            ),

            _interface_subprogram_specification: $ => choice(
                $.interface_procedure_specification,
                $.interface_function_specification
            ),

            procedure_specification: $ => prec.left(seq(
                alias($.PROCEDURE, "procedure"), field("procedure", $._designator), optional($.subprogram_header), optional($.parameter_list_specification)
            )),

            interface_procedure_specification: $ => seq(
                alias($.PROCEDURE, "procedure"), field("procedure", $._designator), optional($.parameter_list_specification)
            ),

            function_specification: $ => seq(
                optional(choice(alias($.PURE, "pure"), alias($.IMPURE, "impure"))), alias($.FUNCTION, "function"), field("function", $._designator), optional($.subprogram_header), optional($.parameter_list_specification), alias($.RETURN, "return"), optional(seq($._identifier, alias($.OF, "of"))), field("type", $.name)
            ),

            interface_function_specification: $ => seq(
                optional(choice(alias($.PURE, "pure"), alias($.IMPURE, "impure"))), alias($.FUNCTION, "function"), field("function", $._designator), optional($.parameter_list_specification), alias($.RETURN, "return"), field("type", $.name)
            ),

            _interface_package_generic_map_aspect: $ => choice(
                $.generic_map_any,
                $.generic_map_default,
                $.generic_map_aspect
            ),

            generic_map_any: $ => seq(
                alias($.GENERIC, "generic"), alias($.MAP, "map"), "(", alias($.box, "<>"), ")"
            ),

            generic_map_default: $ => seq(
                alias($.GENERIC, "generic"), alias($.MAP, "map"), "(", $.DEFAULT, ")"
            ),

            generic_map_aspect: $ => seq(
                alias($.GENERIC, "generic"), alias($.MAP, "map"), $.association_list
            ),

            port_map_aspect: $ => seq(
                alias($.PORT, "port"), alias($.MAP, "map"), $.association_list
            ),

            association_list: $ => seq(
                "(", $.association_element, repeat(seq(",", $.association_element)), ")"
            ),

            _interface_subprogram_default: $ => choice(
                $.name,
                alias($.box, "<>")
            ),

            _designator: $ => choice(
                $._identifier,
                $.operator_symbol
            ),

            generic_identifier_list: $ => seq(
                field("generic", $._identifier), repeat(seq(",", field("generic", $._identifier)))
            ),

            constant_identifier_list: $ => seq(
                field("constant", $._identifier), repeat(seq(",", field("constant", $._identifier)))
            ),

            identifier_list: $ => seq(
                $._identifier, repeat(seq(",", $._identifier))
            ),

            _mode_indication: $ => choice(
                $.simple_mode_indication,
                $._mode_view_indication
            ),

            simple_mode_indication: $ => seq(
                optional($.mode), $._interface_type_indication, optional(alias($.BUS, "bus")), optional($.initialiser)
            ),

            _mode_view_indication: $ => choice(
                $.record_mode_view_indication,
                $.array_mode_view_indication
            ),

            record_mode_view_indication: $ => seq(
                alias($.VIEW, "view"), field("view", $.name), optional(seq(alias($.OF, "of"), $.subtype_indication))
            ),

            array_mode_view_indication: $ => seq(
                alias($.VIEW, "view"), "(", field("view", $.name), ")", optional(seq(alias($.OF, "of"), $.subtype_indication))
            ),

            mode_view_element_definition: $ => seq(
                $.record_element_list, ":", $._element_mode_indication, ";"
            ),

            record_element_list: $ => seq(
                $._identifier, repeat(seq(",", $._identifier))
            ),

            _element_mode_indication: $ => choice(
                $.mode,
                $._element_mode_view_indication
            ),

            _element_mode_view_indication: $ => choice(
                $.element_record_mode_view_indication,
                $.element_array_mode_view_indication
            ),

            element_record_mode_view_indication: $ => seq(
                alias($.VIEW, "view"), field("view", $.name)
            ),

            element_array_mode_view_indication: $ => seq(
                alias($.VIEW, "view"), "(", field("view", $.name), ")"
            ),

            mode: $ => choice(
                alias($.IN, "in"),
                alias($.OUT, "out"),
                alias($.INOUT, "inout"),
                alias($.BUFFER, "buffer"),
                alias($.LINKAGE, "linkage")
            ),

        // Others
            range_constraint: $ => seq(
                alias($.RANGE, "range"), $._range
            ),

            group_constituent_list: $ => seq(
                $.name, repeat(seq(",", $.name))
            ),

            entity_class_entry_list: $ => seq(
                $.entity_class_entry, repeat(seq(",", $.entity_class_entry))
            ),

            entity_class_entry: $ => seq(
                $._entity_class, optional(alias($.box, "<>"))
            ),

            guarded_signal_specification: $ => seq(
                $.signal_list, ":", field("type", $.name)
            ),

            signal_list: $ => choice(
                seq($.name, repeat(seq(",", $.name))),
                $.OTHERS,
                $.ALL
            ),

            entity_specification: $ => seq(
                $.entity_name_list, ":", $._entity_class
            ),

            _entity_class: $ => choice(
                alias($.ENTITY, "entity"),
                alias($.ARCHITECTURE, "architecture"),
                alias($.CONFIGURATION, "configuration"),
                alias($.PROCEDURE, "procedure"),
                alias($.FUNCTION, "function"),
                alias($.PACKAGE, "package"),
                alias($.TYPE, "type"),
                alias($.SUBTYPE, "subtype"),
                alias($.CONSTANT, "constant"),
                alias($.SIGNAL, "signal"),
                alias($.VARIABLE, "variable"),
                alias($.COMPONENT, "component"),
                alias($.LABEL, "label"),
                alias($.LITERAL, "literal"),
                alias($.UNITS, "units"),
                alias($.GROUP, "group"),
                alias($.FILE, "file"),
                alias($.PROPERTY, "property"),
                alias($.SEQUENCE, "sequence"),
                alias($.VIEW, "view")
            ),

            entity_name_list: $ => choice(
                seq($.entity_designator, repeat(seq(",", $.entity_designator))),
                $.OTHERS,
                $.ALL
            ),

            entity_designator: $ => seq(
                $._entity_tag, optional($.signature)
            ),

            _entity_tag: $ => choice(
                $._identifier,
                $.character_literal,
                $.operator_symbol
            ),

            _alias_designator: $ => choice(
                $._identifier,
                $.character_literal,
                $.operator_symbol
            ),

            file_open_information: $ => seq(
                optional(seq($.OPEN, $._expression)), alias($.IS, "is"), $.file_logical_name
            ),

            file_logical_name: $ => $._expression,

            package_header: $ => seq(
                $.generic_clause, optional(seq($.generic_map_aspect, ";"))
            ),

            generic_clause: $ => seq(
                alias($.GENERIC, "generic"), "(", optional(alias($.generic_interface_list, $.interface_list)), ")", ";"
            ),

            subprogram_header: $ => seq(
                alias($.GENERIC, "generic"), "(", optional(alias($.generic_interface_list, $.interface_list)), ")", optional($.generic_map_aspect)
            ),

            _subprogram_specification: $ => choice(
                $.procedure_specification,
                $.function_specification
            ),

            parameter_list_specification: $ => seq(
                optional(alias($.PARAMETER, "parameter")), "(", optional($.interface_list), ")"
            ),

            _type_definition: $ => choice(
                $.enumeration_type_definition,
                $.range_constraint,
                $.physical_type_definition,
                $.array_type_definition,
                $.record_type_definition,
                $.access_type_definition,
                $.file_type_definition,
                $.protected_type_declaration,
                $.protected_type_body,
                $.protected_type_instantiation_definition
            ),

            protected_type_instantiation_definition: $ => seq(
                alias($.NEW, "new"), $.subtype_indication, optional($.generic_map_aspect)
            ),

            protected_type_declaration: $ => seq(
                alias($.PROTECTED, "protected"), optional($.protected_type_header), repeat($._protected_type_declarative_item), $.protected_type_declaration_end
            ),

            protected_type_declaration_end: $ => seq(
                alias($.END, "end"), alias($.PROTECTED, "protected"), optional($._identifier)
            ),

            protected_type_header: $ => seq(
                $.generic_clause, optional(seq($.generic_map_aspect, ";"))
            ),

            protected_type_body: $ => seq(
                alias($.PROTECTED, "protected"), alias($.BODY, "body"), repeat($._protected_type_body_declarative_item), $.protected_type_body_end
            ),

            protected_type_body_end: $ => seq(
                alias($.END, "end"), alias($.PROTECTED, "protected"), alias($.BODY, "body"), optional($._identifier)
            ),

            while_loop: $ => seq(
                alias($.WHILE, "while"), $._expression
            ),

            for_loop: $ => seq(
                alias($.FOR, "for"), $.parameter_specification
            ),

            parameter_specification: $ => seq(
                $._identifier, alias($.IN, "in"), $._range
            ),

            case_statement_alternative: $ => seq(
                $.when_element, "=>", repeat($._sequential_statement)
            ),

            selected_waveforms: $ => seq(
                repeat(seq($.waveform, $.when_element, ",")), $.waveform, $.when_element
            ),

            waveform: $ => choice(
                seq($.waveform_element, repeat(seq(",", $.waveform_element))),
                alias($.UNAFFECTED, "unaffected")
            ),

            waveform_element: $ => seq(
                $._expression, optional(seq(alias($.AFTER, "after"), $._expression)),
            ),

            force_mode: $ => choice(
                alias($.IN, "in"),
                alias($.OUT, "out")
            ),

            selected_expressions: $ => seq(
                repeat(seq($._expression, $.when_element, ",")), $._expression, $.when_element
            ),

            when_element: $ => seq(
                alias($.WHEN, "when"), $._element
            ),

            conditional_waveforms: $ => seq(
                $.waveform, $.when_expression, repeat(seq($.else_waveform, $.when_expression)), optional($.else_waveform)
            ),

            else_waveform: $ => seq(
                alias($.ELSE, "else"), $.waveform
            ),

            delay_mechanism: $ => choice(
                alias($.TRANSPORT, "transport"),
                seq(optional(seq(alias($.REJECT, "reject"), $._expression)), alias($.INERTIAL, "inertial"))
            ),

            _target: $ => choice(
                $.name,
                $.aggregate
            ),

            aggregate: $ => seq(
                "(",
                $.element_association, repeat(seq(",", choice($.element_association))),
                ")"
            ),

            conditional_or_unaffected_expression: $ => seq(
                $._expression_or_unaffected, repeat(seq($.when_expression, $.else_expression_or_unaffected)), optional($.when_expression)
            ),

            else_expression_or_unaffected: $ => seq(
                alias($.ELSE, "else"), $._expression_or_unaffected
            ),

            _expression_or_unaffected: $ => choice(
                $._expression,
                alias($.UNAFFECTED, "unaffected")
            ),

            label_declaration: $ => seq(
                $._label, ":"
            ),

            sensitivity_clause: $ => seq(
                alias($.ON, "on"), $.sensitivity_list
            ),

            sensitivity_list: $ => seq(
                $.name, repeat(seq(",", $.name))
            ),

            condition_clause: $ => seq(
                alias($.UNTIL, "until"), $._expression
            ),

            timeout_clause: $ => seq(
                alias($.FOR, "for"), $._expression
            ),

            physical_type_definition: $ => seq(
                $.range_constraint, alias($.UNITS, "units"), $.primary_unit_declaration, repeat($.secondary_unit_declaration), $.end_units
            ),

            primary_unit_declaration: $ => seq(
                $._identifier, ";"
            ),

            secondary_unit_declaration: $ => seq(
                $._identifier, "=", seq(optional($._abstract_literal), choice($.name, $.library_constant_unit)), ";"
            ),

            end_units: $ => seq(
                alias($.END, "end"), alias($.UNITS, "units"), optional($._identifier)
            ),

            enumeration_type_definition: $ => seq(
                "(", $.enumeration_literal, repeat(seq(",", $.enumeration_literal)), ")"
            ),

            enumeration_literal: $ => choice(
                field("constant", $._identifier),
                $.character_literal
            ),

            record_type_definition: $ => seq(
                alias($.RECORD, "record"), repeat($.element_declaration), $.end_record
            ),

            end_record: $ => seq(
                alias($.END, "end"), alias($.RECORD, "record"), optional($._identifier)
            ),

            element_declaration: $ => seq(
                $.identifier_list, ":", $.subtype_indication, ";"
            ),

            _process_sensitivity_list: $ => choice(
                $.ALL,
                $.sensitivity_list
            ),

            configuration_specification: $ => prec.left(choice(
                seq(alias($.FOR, "for"), $.component_specification, $.binding_indication, optional(seq($.end_for, ";"))),
                seq(alias($.FOR, "for"), $.component_specification, $.binding_indication, $.verification_unit_binding_indication, ";", repeat(seq($.verification_unit_binding_indication, ";")), $.end_for, ";")
            )),

            verification_unit_binding_indication: $ => seq(
                alias($.USE, "use"), alias($.VUNIT, "vunit"), $.verification_unit_list
            ),

            verification_unit_list: $ => seq(
                $.name, repeat(seq(",", $.name))
            ),

            component_specification: $ => seq(
                $.instantiation_list, ":", $.name
            ),

            instantiation_list: $ => choice(
                seq($._label, repeat(seq(",", $._label))),
                $.OTHERS,
                $.ALL
            ),

            binding_indication: $ => seq(
                optional(seq(alias($.USE, "use"), $.entity_aspect)), optional($.generic_map_aspect), optional($.port_map_aspect), ";"
            ),

            entity_aspect: $ => choice(
                seq(alias($.ENTITY, "entity"), field("entity", $.name), optional(seq("(", field("architecture", $._identifier), ")"))),
                seq(alias($.CONFIGURATION, "configuration"), field("configuration", $.name)),
                $.OPEN
            ),

        // Comments
            line_comment: $ => seq(
                $._line_comment_start,
                $.comment_content,
            ),

            block_comment: $ => seq(
                $._block_comment_start,
                $.comment_content,
                $._block_comment_end,
            ),
    }
});

