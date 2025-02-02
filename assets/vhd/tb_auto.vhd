library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity tb_auto is
end tb_auto;

architecture behavioral of tb_auto is
    component auto is
        port ( clk:     in  std_logic;
               Reset:   in  std_logic;
               e:       in  std_logic;
               s:       out std_logic);
    end component;

    signal clk:     std_logic := '0';
    constant clk_period : time := 8 ns;
    signal reset:   std_logic;
    signal e:       std_logic := '0';
    signal s:       std_logic;

    type bits_tab is array (0 to 31) of std_logic;
    constant stimuli_auto: bits_tab := (
        '0', '0', '0', '1', '1', '1', '0', '0',
        '0', '1', '0', '0', '1', '1', '0', '0',
        '0', '1', '0', '1', '1', '1', '1', '1',
        '0', '0', '0', '1', '0', '0', '1', '1');

    constant sorties_attendues: bits_tab := (
        '0', '0', '0', '0', '0', '0', '0', '0',
        '0', '0', '0', '0', '0', '1', '0', '0',
        '0', '0', '0', '0', '1', '0', '0', '0',
        '0', '0', '0', '0', '0', '0', '0', '1');
begin

    C_auto: auto
    port map (  clk     => clk,
                reset   => reset,
                e       => e,
                s       => s);

    process
        variable i: integer range 0 to 32 := 0;
    begin
        e   <= stimuli_auto(i);
        wait for clk_period/2 ;
        clk <= not clk;
        wait for clk_period/2 ;
        clk <= not clk;

        assert (s = sorties_attendues(i)) report "==== ERREUR: Sortie de l'automate inattendue ====" severity error;
        i   := i + 1;
        assert (i < 32) report "Simulation terminée" severity failure;
    end process;

    process
    begin
        reset <= '1';
        wait for clk_period ;
        reset <= '0';
        wait;
    end process;
end behavioral;

