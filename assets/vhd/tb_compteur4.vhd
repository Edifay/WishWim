library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;

entity tb_compteur4 is
end tb_compteur4;

architecture behavioral of tb_compteur4 is
    component compteur4 is
    port (
    clk :    in  std_logic;
    reset :  in  std_logic;
    cpt:     out unsigned (3 downto 0)
    );
    end component;

    signal clk:     std_logic := '0';
    constant clk_period : time := 8 ns;
    signal reset:   std_logic;

    signal cpt :  unsigned (3 downto 0);


begin

    C_compteur4: compteur4
    port map (  clk     => clk,
                reset   => reset,
                cpt     => cpt);

    process
    begin
        wait for clk_period/2 ;
        clk <= not clk;
        wait for clk_period/2 ;
        clk <= not clk;
    end process;

    process
    begin
        reset <= '1';
        wait for clk_period ;
        reset <= '0';
        wait for 10*clk_period ;
        reset <= '1';
        wait for clk_period ;
        reset <= '0';
        wait;
    end process;
end behavioral;

