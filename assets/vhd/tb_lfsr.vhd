---------------------------------------
-- tb_lfsr.vhd
-- Testbench pour le composant ``lfsr''
---------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Le testbench est une entité (``tb_lfsr''), qui va instancier le composant
-- testé (``lfsr'') et lui fournir des stimuli.
entity tb_lfsr is
end tb_lfsr;

architecture behavioral of tb_lfsr is
    -- On déclare le composant que l'on souhaite tester pour pouvoir
    -- l'instancier dans le testbench
    component lfsr is
      port ( clk:     in  std_logic;
             reset:   in  std_logic;
             s:  out std_logic);
    end component;

    -- Les signaux internes du testbench
    signal clk:     std_logic := '0';
    signal reset:   std_logic;
    signal s:  std_logic;

begin

    -- Le composant testé est instancié. Ses entrées/sorties sont connectées
    -- aux signaux du testbench
    C_lfsr: lfsr
    port map (  clk     => clk,
                reset   => reset,
                s       => s);

    -- Cette partie est un processus (``process'') qui génère un stimulus pour
    -- simuler l'horloge du circuit. Ce processus agit comme une boucle qui
    -- inverse la valeur de signal ``clk'', puis attend 4 ns. On obtient
    -- ainsi une horloge de période 8 ns (125MHz).
    process
    begin
        clk <= not clk;
        wait for 4 ns;
    end process;

    -- Ce processus génère le stimulus pour le signal de reset du circuit. Ce
    -- dernier doit être mis à '1' brièvement au début de la simulation pour
    -- initialiser les bascules D. Ce processus le met à '1' pendant 10 ns
    -- puis le met à '0' et attend infiniment.
    process
    begin
      reset <= '1';
      wait for 10 ns;
      reset <= '0';
      wait;
    end process;
end behavioral;

