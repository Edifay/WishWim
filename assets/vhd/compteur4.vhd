library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Le paquetage numeric_std définit les types signed et unsigned, et les opérations arithmétiques associés.
use ieee.numeric_std.all;

entity compteur4 is
  port (
    clk :    in  std_logic; -- entrée d'horloge de ce circuit
    reset :  in  std_logic; -- entrée de remise à zéro du circuit
    cpt:     out unsigned (3 downto 0) -- la sortie cpt est un vecteur de 4 bits interprétable comme un entier non signé.
    -- cpt prend donc des valeurs entre 0 et 15 et on peut lui appliquer des opérateurs arithmétiques +, -, *, etc. 
    -- ainsi que des comparaisons <, >, <=, >=, =, /=
  );
end compteur4 ;

architecture mixte of compteur4 is

  -- Déclaration des composants reg4 (registre 4 bits)
  -- Allez voir la définition dans "vhd/reg4b.vhd" avant d'attaquer la suite
  component reg4b
    port (
      d   : in unsigned(3 downto 0);
      clk : in std_logic ;
      rst : in  std_logic;
      q   : out unsigned(3 downto 0)
      );
  end component;

  signal  dd :     unsigned(3 downto 0); -- déclaration d'un vecteur de bits non-signé pour contenir la valeur future du compteur
  signal  curval : unsigned(3 downto 0); -- déclaration d'un vecteur de bits non-signé pour contenir la valeur courante du compteur

begin

	CT4: reg4b
		port map (clk => clk,
							rst => reset,
							d => dd,
							q => curval);
	
	cpt <= curval;
	dd <= curval+1; 

  -- Instanciez un registre 4 bits pour alimenter le signal curval et affectez une valeur à dd et à cpt
end mixte;
