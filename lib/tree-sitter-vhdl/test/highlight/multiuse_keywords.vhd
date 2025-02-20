entity MyModule is
  port(
    signal Clk   : in std_logic;
--                 ^ keyword.modifier
    signal Reset : in std_logic
--                 ^ keyword.modifier
  );
end MyModule;

architecture Behaviour of MyModule is
begin
  process(Clk) begin
    if(rising_edge(Clk)) then
      State <= Idle;

    else
--  ^ keyword.conditional
      for n in 1 to 7 loop
--    ^ keyword.repeat
--          ^ keyword
        A <= B + C;
      end loop;

      case(State) is
--    ^ keyword.conditional
        when Idle =>
--      ^ keyword.conditional
          -- Do stuff

        when Running =>
--      ^ keyword.conditional
          -- Do stuff

        when others =>
--      ^ keyword.conditional
          -- Do stuff
      end case;
    end if;

    wait for 3 ns;
--       ^ keyword.coroutine
  end process;

  A <= 3 when B = C else 5;
--       ^ keyword.conditional.ternary
--                  ^ keyword.conditional.ternary

  A <= (others => 3 when B = C else 5);
--                  ^ keyword.conditional.ternary
--                             ^ keyword.conditional.ternary

  Gen: case(PARAM) generate
    when Gen5: 5 => begin
--  ^ keyword.conditional
      -- Instance 5
    end Gen5;

    when 6 =>
--  ^ keyword.conditional
      -- Instance 6

    when others =>
--  ^ keyword.conditional
      -- Default instance
  end generate;
end Behaviour;

