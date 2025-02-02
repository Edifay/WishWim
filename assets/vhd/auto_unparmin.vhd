library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity auto is
    port ( clk:     in  std_logic;
           reset:   in  std_logic;
           e:       in  std_logic;
           s:       out std_logic);
end auto;

architecture structural of auto is

    signal Etat_courant, Etat_futur: unsigned(4 downto 0);

begin
	-- Pour varier les plaisirs, le registre d'état sera implanté avec un process
	process (clk) 
	begin
		if rising_edge(clk) then
			if reset = '1' then
				Etat_courant <= "00001";
			else 
				Etat_courant <= Etat_futur;
			end if;
		end if;
	end process;
	
	Etat_futur(0) <= Etat_courant(0) and not e;
	Etat_futur(1) <= (Etat_courant(0) or Etat_courant(1) or Etat_courant(4)) and e;
	Etat_futur(2) <= (Etat_courant(1) or Etat_courant(2) or Etat_courant(3) or Etat_courant(4)) and not e;
	Etat_futur(3) <= Etat_courant(2) and e;
	Etat_futur(4) <= Etat_courant(3) and e;
	
	s <= Etat_courant(4);

	
	
	
	
	
	
	
end structural;
