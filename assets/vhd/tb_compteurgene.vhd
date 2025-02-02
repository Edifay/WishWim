library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;

entity tb_compteurgene is
end tb_compteurgene;

architecture behavioral of tb_compteurgene is
  component compteurgene is
    generic (n : positive := 12);
    port (
    clk :    in  std_logic;
    reset :  in  std_logic;
    cpt:     out unsigned (n-1 downto 0)
    );
    end component;

    signal clk:     std_logic := '0';
    constant clk_period : time := 8 ns;
    signal reset:   std_logic;

  -- on va fabriquer une instance du composant générique
  signal cpt6 :  unsigned (5 downto 0);

begin
  C_compteur6: compteurgene
    generic map (n => 6)
    port map (  clk     => clk,
                reset   => reset,
                cpt     => cpt6);

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
        wait;
    end process;
end behavioral;

