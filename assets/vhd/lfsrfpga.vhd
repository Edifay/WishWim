library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity lfsrfpga is
    port ( clk:     in  std_logic;
           reset:   in  std_logic;
           s:  out std_logic);
end lfsrfpga;

architecture mixte of lfsrfpga is

    component lfsr is
    port ( clk:     in  std_logic;
           reset:   in  std_logic;
           s:  out std_logic);
    end component;
    component div_freq is
    port ( clk:     in  std_logic;
           s:  out std_logic;
           reset:  in  std_logic);
    end component;

    constant facteur : integer := 26;
    signal tmpH: std_logic_vector(facteur downto 0);

begin
	tmpH(0) <= clk;

    -- On utilise la structure generate pour cascader les diviseurs de fréquences.
    -- Cette structure qui n'est pas à connaitre (pour le module) permet
    -- de réaliser de manière itérative plusieurs instanciations.
    Horloge:
    for i in 0 to facteur -1 generate
            DivX: div_freq port map (
                 clk => tmpH(i),
                 s => tmpH(i+1),
                 reset => reset);
    end generate Horloge;


-- On genere une horloge permettant de suivre à l'oeil nu
-- A quelle fréquence travaille t'on maintenant ?
    I_lfsr: lfsr
    port map (  clk     => tmpH(facteur),
                reset   => reset,
                s       => s);

end mixte;

