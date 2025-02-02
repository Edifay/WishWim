library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity Compteur is
	generic (n: positive := 17);
	port (reveil : out std_logic; -- passe à 1 au bout de 2^n cycles après le rst
	      clk : in std_logic;
	      rst : in std_logic);
end Compteur;

architecture mixte of Compteur is
	signal cpt: unsigned(n downto 0);
begin
   -- processus de description du registre 	
	process (clk)
	begin
		if rising_edge(clk) then
			if rst='1' then
				cpt <= (others => '0');
			else
				cpt <= cpt + 1;
			end if;
		end if;
	end process;
	reveil <= cpt(n);
end mixte;
