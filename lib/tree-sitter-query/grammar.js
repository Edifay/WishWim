/// <reference types="tree-sitter-cli/dsl" />

const PREC = {
  IMMEDIATE_CHILD: 1,

  // Prefer a string over a comment
  COMMENT: 1,
  STRING: 2,

  WILDCARD_NODE: 1,
};

const IDENTIFIER = /[a-zA-Z0-9.\-_\$]+/;

module.exports = grammar({
  name: "query",

  extras: $ => [
    $.comment,
    /\s+/,
  ],

  supertypes: $ => [
    $.definition,
  ],

  rules: {
    program: $ => repeat($.definition),

    definition: $ => choice(
      $.named_node,
      $.anonymous_node,
      $.grouping,
      $.predicate,
      $.list,
      $.field_definition,
    ),

    // Expressions that are valid inside a group.
    _group_expression: $ => choice(
      $.definition,
      immediate_child($._group_expression),
    ),

    // Expressions that are valid inside a named node.
    _named_node_expression: $ => choice(
      $.definition,
      $.negated_field,
      immediate_child($._named_node_expression),
    ),

    // Taken from https://github.com/tree-sitter/tree-sitter-javascript/blob/3f8b62f9befd3cb3b4cb0de22f6595a0aadf76ca/grammar.js#L827
    escape_sequence: _ => token.immediate(seq(
      '\\',
      choice(
        /[^xu0-7]/,
        /[0-7]{1,3}/,
        /x[0-9a-fA-F]{2}/,
        /u[0-9a-fA-F]{4}/,
        /u\{[0-9a-fA-F]+\}/
      )
    )),

    quantifier: _ => choice("*", "+", "?"),

    identifier: _ => IDENTIFIER,
    _immediate_identifier: $ => alias(token.immediate(IDENTIFIER), $.identifier),
    _node_identifier: $ => choice($.identifier, prec(PREC.WILDCARD_NODE, "_")),
    capture: $ => seq("@", field("name", $._immediate_identifier)),
    string: $ => seq(
      '"',
      optional($.string_content),
      '"',
    ),
    string_content: $ => repeat1(choice(token.immediate(prec(PREC.STRING, /[^"\\]+/)), $.escape_sequence)),
    parameters: $ => repeat1(choice($.capture, $.string, $._node_identifier)),
    comment: _ => token(prec(PREC.COMMENT, seq(";", /.*/))),
    list: $ => seq("[", repeat($.definition), "]", quantifier($), captures($)),

    grouping: $ => seq(
      "(",
      repeat(seq($._group_expression, optional("."))),
      ")",
      quantifier($),
      captures($),
    ),

    anonymous_node: $ => seq(
      field("name", choice($.string, "_")),
      quantifier($),
      captures($),
    ),

    named_node: $ => seq(
      "(",
      choice(
        field("name", $._node_identifier),
        seq(field("supertype", $.identifier), token.immediate('/'), field("name", $._immediate_identifier)),
      ),
      optional(
        seq(
          optional("."),
          choice(
            repeat1($._named_node_expression),
            seq(
              repeat($._named_node_expression),
              seq($._named_node_expression, "."),
            )
          ),
        ),
      ),
      ")",
      quantifier($),
      captures($),
    ),
    _field_name: $ => seq($.identifier, ":"),
    field_definition: $ => seq(
      field("name", $._field_name),
      $.definition,
    ),

    negated_field: $ => seq("!", $.identifier),

    predicate: $ =>
      seq(
        "(",
        field("name", seq("#", $._immediate_identifier, field("type", $.predicate_type))),
        field("parameters", $.parameters),
        ")"
      ),
    predicate_type: _ => token.immediate(choice("?", "!")),
  }
});

function captures($) {
  return repeat($.capture);
}

function quantifier($) {
  return optional(field("quantifier", $.quantifier));
}

function immediate_child(expression) {
  return prec.left(
    PREC.IMMEDIATE_CHILD,
    seq(expression, ".", expression),
  );
}
