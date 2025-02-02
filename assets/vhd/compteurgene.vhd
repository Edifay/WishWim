library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;

entity compteurgene is
  generic(
      n: positive :=8  -- 8 est la valeur par défaut si on ne spécifie rien
  );
  port (
    clk :    in  std_logic;
    reset :  in  std_logic;
    cpt:     out unsigned (n-1 downto 0)
  );
end compteurgene ;

architecture mixte of compteurgene is

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
  signal  dd : unsigned(n-1 downto 0);
  signal curval : unsigned(n-1 downto 0);
begin

  -- Registre n bits
  CPTgene: reggene
    -- completer pour choisir une taille pour cette instance de reggene :
    generic map (
				n => n
      )
    -- completer pour decrire le branchement des ports :
    port map (
				clk => clk,
				d => dd,
				q => curval,
				rst => reset 
      );

-- Equations :
	cpt <= curval;
	dd <= curval+1;
end mixte;
