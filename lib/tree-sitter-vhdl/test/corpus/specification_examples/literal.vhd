================================================================================
Decimal literals
================================================================================

-- Integer literals
D <= 12;
D <= 0;
D <= 1E6;
D <= 123_456;

-- Real literals
D <= 12.0;
D <= 0.0;
D <= 0.456;

-- Real literals with exponents
D <= 3.14159_26;
D <= 1.34E-12;
D <= 1.0E+6;
D <= 6.023E+24;

--------------------------------------------------------------------------------

(design_file
  (line_comment
    (comment_content))
  (design_unit
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (decimal_integer)))))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (decimal_integer)))))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (decimal_integer)))))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (decimal_integer)))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (decimal_float)))))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (decimal_float)))))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (decimal_float)))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (decimal_float)))))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (decimal_float)))))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (decimal_float)))))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (decimal_float)))))))

================================================================================
Based literals
================================================================================

-- Integer literals of value 255
B <= 2#1111_1111#;
B <= 16#FF#;
B <= 016#0FF#;

-- Integer literals of value 224
B <= 16#E#E1;
B <= 2#1110_0000#;

-- Real literals of value 4095.0
B <= 16#F.FF#E+2;
B <= 2#1.1111_1111_111#E11;

--------------------------------------------------------------------------------

(design_file
  (line_comment
    (comment_content))
  (design_unit
  (concurrent_simple_signal_assignment
    (name
      (identifier))
    (signal_assignment)
    (waveform
      (waveform_element
        (simple_expression
          (based_literal
            (based_base)
            (based_integer))))))
  (concurrent_simple_signal_assignment
    (name
      (identifier))
    (signal_assignment)
    (waveform
      (waveform_element
        (simple_expression
          (based_literal
            (based_base)
            (based_integer))))))
  (concurrent_simple_signal_assignment
    (name
      (identifier))
    (signal_assignment)
    (waveform
      (waveform_element
        (simple_expression
          (based_literal
            (based_base)
            (based_integer))))))
  (line_comment
    (comment_content))
  (concurrent_simple_signal_assignment
    (name
      (identifier))
    (signal_assignment)
    (waveform
      (waveform_element
        (simple_expression
          (based_literal
            (based_base)
            (based_integer))))))
  (concurrent_simple_signal_assignment
    (name
      (identifier))
    (signal_assignment)
    (waveform
      (waveform_element
        (simple_expression
          (based_literal
            (based_base)
            (based_integer))))))
  (line_comment
    (comment_content))
  (concurrent_simple_signal_assignment
    (name
      (identifier))
    (signal_assignment)
    (waveform
      (waveform_element
        (simple_expression
          (based_literal
            (based_base)
            (based_float))))))
  (concurrent_simple_signal_assignment
    (name
      (identifier))
    (signal_assignment)
    (waveform
      (waveform_element
        (simple_expression
          (based_literal
            (based_base)
            (based_float))))))))

================================================================================
Characters
================================================================================

C <= 'A';
C <= '*';
C <= ''';
C <= ' ';

--------------------------------------------------------------------------------

(design_file
  (design_unit
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (name
              (character_literal))))))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (name
              (character_literal))))))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (name
              (character_literal))))))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (name
              (character_literal))))))))

================================================================================
Strings
================================================================================

S <= "Setup time is too short"; -- An error message.
S <= "";                        -- An empty string literal.
S <= " " & "A" & """";          -- Three string literals of length 1.
S <= "Characters such as $, %, and } are allowed in string literals.";
S <= "FIRST PART OF A SEQUENCE OF CHARACTERS "
   & "THAT CONTINUES ON THE NEXT LINE"
   & "Sequence that includes the" & ACK & "control character";

--------------------------------------------------------------------------------

(design_file
  (design_unit
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
            (simple_expression
              (simple_expression
                (string_literal))
              (adding_operator)
              (simple_expression
                (string_literal)))
            (adding_operator)
            (simple_expression
              (string_literal))))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (string_literal)))))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (simple_expression
              (simple_expression
                (simple_expression
                  (simple_expression
                    (string_literal))
                  (adding_operator)
                  (simple_expression
                    (string_literal)))
                (adding_operator)
                (simple_expression
                  (string_literal)))
              (adding_operator)
              (simple_expression
                (library_constant_character)))
            (adding_operator)
            (simple_expression
              (string_literal))))))))

================================================================================
Bit-strings
================================================================================

I <= B"1111_1111_1111"; -- Equivalent to the string literal "111111111111".
I <= X"FFF";            -- Equivalent to B"1111_1111_1111".
I <= O"777";            -- Equivalent to B"111_111_111".
I <= X"777";            -- Equivalent to B"0111_0111_0111".
I <= B"XXXX_01LH";      -- Equivalent to the string literal "XXXX01LH"
I <= UO"27";            -- Equivalent to B"010_111"
I <= SX"3W";            -- Equivalent to B"0011_WWWW"
I <= D"35";             -- Equivalent to B"100011"
I <= 12UB"X1";          -- Equivalent to B"0000_0000_00X1"
I <= 12SB"X1";          -- Equivalent to B"XXXX_XXXX_XXX1"
I <= 12UX"F-";          -- Equivalent to B"0000_1111_----"
I <= 12SX"F-";          -- Equivalent to B"1111_1111_----"
I <= 12D"13";           -- Equivalent to B"0000_0000_1101"
I <= 12UX"000WWW";      -- Equivalent to B"WWWW_WWWW_WWWW"
I <= 12SX"FFFC00";      -- Equivalent to B"1100_0000_0000"
I <= 12SX"XXXX00";      -- Equivalent to B"XXXX_0000_0000"

--------------------------------------------------------------------------------

(design_file
  (design_unit
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (bit_string_literal
              (bit_string_base)
              (bit_string_value))))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (bit_string_literal
              (bit_string_base)
              (bit_string_value))))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (bit_string_literal
              (bit_string_base)
              (bit_string_value))))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (bit_string_literal
              (bit_string_base)
              (bit_string_value))))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (bit_string_literal
              (bit_string_base)
              (bit_string_value))))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (bit_string_literal
              (bit_string_base)
              (bit_string_value))))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (bit_string_literal
              (bit_string_base)
              (bit_string_value))))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (bit_string_literal
              (bit_string_base)
              (bit_string_value))))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (bit_string_literal
              (bit_string_length)
              (bit_string_base)
              (bit_string_value))))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (bit_string_literal
              (bit_string_length)
              (bit_string_base)
              (bit_string_value))))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (bit_string_literal
              (bit_string_length)
              (bit_string_base)
              (bit_string_value))))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (bit_string_literal
              (bit_string_length)
              (bit_string_base)
              (bit_string_value))))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (bit_string_literal
              (bit_string_length)
              (bit_string_base)
              (bit_string_value))))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (bit_string_literal
              (bit_string_length)
              (bit_string_base)
              (bit_string_value))))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (bit_string_literal
              (bit_string_length)
              (bit_string_base)
              (bit_string_value))))))
    (line_comment
      (comment_content))
    (concurrent_simple_signal_assignment
      (name
        (identifier))
      (signal_assignment)
      (waveform
        (waveform_element
          (simple_expression
            (bit_string_literal
              (bit_string_length)
              (bit_string_base)
              (bit_string_value)))))))
  (line_comment
    (comment_content)))

