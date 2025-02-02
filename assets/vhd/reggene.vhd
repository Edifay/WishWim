library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity reggene is
    generic( -- le mot-clé generic permet d'indiquer une liste de paramètres statiques qui seront appliqués à la description VHDL pour construire le circuit réel
      n: positive :=8  -- le paramètre générique n prendra par défaut la valeur 8 si aucune valeur n'est spécifiée à l'instanciation.
      );
    port (d   : in  unsigned(n-1 downto 0); -- expression d'un vecteur de n bits
          clk : in  std_logic;
          rst : in  std_logic;
          q   : out unsigned(n-1 downto 0));
end reggene;

architecture Behavioral of reggene is
begin
   process (clk)
   begin
     if rising_edge(clk) then
        if rst='1' then
          q <= (others => '0'); 
	-- n n'étant pas connu. On ne peut plus utiliser l'initialisation avec une constante. 
	-- Le mot clé "others" permet de définir tous les valeurs d'un vecteur qui n'ont pas encore été définis.
	  -- Ici, chaque bit du vecteur prendra la valeur '0'
        else
         q <= d;
        end if;
      end if;
   end process;
end Behavioral;
