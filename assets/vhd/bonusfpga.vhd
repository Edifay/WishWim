library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity bonusfpga is
    port ( clk:     in  std_logic;
           reset:   in  std_logic;
           s0:  out std_logic;
           s1:  out std_logic;
           s2:  out std_logic;
           s3:  out std_logic);
end bonusfpga ;

architecture mixte of bonusfpga is

    component AC is
    port ( a:  in  std_logic;
           b:  in  std_logic;
           c:  in  std_logic;
           s:  out std_logic;
           co: out std_logic);
    end component;
    component div_freq is
    port ( clk:     in  std_logic;
           s:  out std_logic;
           reset:  in  std_logic);
    end component;
    component basc_D is
    port (  clk:    in  std_logic;
            d:      in  std_logic;
            q:      out std_logic;
            qb:     out std_logic;
            reset:  in  std_logic);
    end component;

    constant facteur : integer := 26;
    signal tmpH: std_logic_vector(facteur downto 0);

begin
    -- On génére une horloge à l'Hertz comme pour le LFSR
	tmpH(0) <= clk;

    Horloge:
    for i in 0 to facteur -1 generate 
            DivX: div_freq port map (
                 clk => tmpH(i),
                 s => tmpH(i+1),
                 reset => reset);
    end generate Horloge;

    -- tmpH(facteur) est l'horloge à utiliser pour le reste du circuit
    -- A compléter en fonction du circuit mystère fourni par l'enseignant


end mixte;

