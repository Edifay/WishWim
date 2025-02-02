library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity reg4b is
	port (d   : in  unsigned(3 downto 0);
	      clk : in  std_logic;
	      rst : in  std_logic;
	      q   : out unsigned(3 downto 0));
end reg4b;

architecture Behavioral of reg4b is
begin
   -- Comme pour la bascule D vu au dernier TP, on retrouve une description via un processus.
	process (clk) -- La liste de sensibilité ne contient que l'horloge.
		      -- Contrairement à la bascule D, un changement sur rst ne sera pris en compte qu'au prochain front montant de l'horloge. On parle de reset asynchrone.
	begin
		if rising_edge(clk) then
			if rst='1' then
				q <= "0000"; -- Ici, il convient de noter que les constantes de vecteurs de bits s'expriment entre double quote, contrairement aux constantes std_logic qui s'expriment entre simple quote.
			else
				q <= d; -- affectation de vecteurs. Rien de neuf
			end if;
		end if;
	end process;
end Behavioral;

