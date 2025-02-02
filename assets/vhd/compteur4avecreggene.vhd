library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

use ieee.numeric_std.all;

entity compteur4avecreggene is
  port (
    clk :    in  std_logic;
    reset :  in  std_logic;
    cpt:     out unsigned (3 downto 0)
  );
end compteur4avecreggene ;

architecture mixte of compteur4avecreggene is

  -- Registre n bits
  component reggene
    generic (
      n: positive :=8
      );
    port (
      d   : in unsigned(n-1 downto 0);
      clk : in std_logic ;
      rst : in  std_logic;
      q   : out unsigned(n-1 downto 0)
      );
  end component;

  signal  dd :     unsigned(3 downto 0);
  signal  curval : unsigned(3 downto 0);

begin

  CPT4: reggene
    generic map ( -- Lors de l'instantiation, le generic map permet de fixer les valeurs voulues pour les paramètres génériques de l'instance
      n => 4 -- on veut ici un registre de taille 4
      )
    port map (clk => clk,
              rst => reset,
              d => dd,
              q => curval);

  cpt <= curval ;
  dd <= curval+1;

end mixte;
