library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

ENTITY tb_lecture_memoire IS
END tb_lecture_memoire;

ARCHITECTURE behavior OF tb_lecture_memoire IS

component lecture_memoire is
    port (data : out unsigned(31 downto 0);
          clk : in std_logic;
          rst : in std_logic);
end component;

  -- Signaux internes
	signal clk :  std_logic := '0';
  signal rst :  std_logic := '1';
  signal donnee_lue : unsigned(31 downto 0);
  -- Constantes
  constant clock_period : time := 8 ns;
BEGIN
	ulecture: lecture_memoire PORT MAP(
		clk => clk,
		rst => rst,
    data => donnee_lue
	);

	horloge : process
	begin
		CLK<= not CLK;
    wait for clock_period/2 ;
	end process horloge;

	process
	begin
    wait for 3*clock_period;
    rst <= '0';
    wait;
	end process;
end ;
