library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

ENTITY tb_VGA IS
END tb_VGA;

ARCHITECTURE behavior OF tb_VGA IS 

	-- Component Declaration for the Unit Under Test (UUT)
	COMPONENT VGA
	PORT(
		CLK : IN std_logic;          
		HS : OUT std_logic;
		VS : OUT std_logic;
        R : OUT unsigned(4 downto 0);
        G : OUT unsigned(5 downto 0);
        B : OUT unsigned(4 downto 0) 
		);
	END COMPONENT;
	--Inputs
    SIGNAL Clk :  std_logic := '0';
    constant clk_period : time := 8 ns;

	--Outputs
	SIGNAL HSYNC :  std_logic;
	SIGNAL VSYNC :  std_logic;
	SIGNAL R :  unsigned(4 downto 0);
	SIGNAL G :  unsigned(5 downto 0);
	SIGNAL B :  unsigned(4 downto 0);
BEGIN

	-- Instantiate the Unit Under Test (UUT)
	uut: VGA PORT MAP(
		HS => HSYNC,
		VS => VSYNC,
		R => R,
		G => G,
		B => B,
		CLK => CLK);

	horloge : process
	begin
        CLK<= not CLK;
        wait for clk_period/2 ;
	end process horloge;
END;
