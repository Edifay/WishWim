library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity auto is
	port ( clk   :	in	std_logic;
				 reset :	in	std_logic;
				 e     :	in	std_logic;
				 s     :	out std_logic);
end auto;

architecture structural of auto is
	type Etat_type is (A, B, C, D, eE); -- On définit un nouveau type "Etat_type" comme étant une énumération de 5 symboles possibles.
		-- comme le langage est insensible à la casse on ne peut pas définir un symbole E (entrée e de l'entité déjà existante) on nomme donc cet état eE
		-- Ces symboles pourront être utilisés pour les signaux de ce type.
		signal Etat_courant, Etat_futur : Etat_type; -- on définit nos 2 signaux
begin
	-- On décrit le registre d'état avec un process (pas le choix car on ne connait pas la taille du registre)
	Registre : process (clk)
	begin
		if rising_edge(clk) then
			if reset = '1' then
				Etat_courant <= A;
			else 
				Etat_courant <= Etat_futur;
			end if;
		end if;
	end process;

		-- Pour décrire la fonction de transition, on peut aussi utiliser un process
	Transition : process (
		-- Liste de sensibilité à compléter avec tous les signaux entrants de la fonction de transition
			Etat_courant, e
		)
	begin
		case Etat_courant is	-- Dans un process, on peut utiliser une structure de type case (mots clés :	case et is)
			when A => 
				if e = '0' then 
					Etat_futur <= A;
				else
					Etat_futur <= B;
				end if;
			when B => 
				if e = '0' then 
					Etat_futur <= C;
				else
					Etat_futur <= B;
				end if;
			when C => 
				if e = '0' then 
					Etat_futur <= C;
				else
					Etat_futur <= D;
				end if;
			when D => 
				if e = '0' then 
					Etat_futur <= C;
				else
					Etat_futur <= eE;
				end if;
			when eE => 
				if e = '0' then 
					Etat_futur <= C;
				else
					Etat_futur <= B;
				end if;
		end case; -- fin de la structure case
	end process;

	-- De même, on décrit la fonction de sortie par un process
	-- Dans un case, pour englober tous les cas restants, on peut utiliser "when others =>".
	Sorties : process (
		Etat_courant
		-- Compléter la liste de sensibilité par les signaux entrants de la fonction de sortie
		)
	begin
		case Etat_courant is
			when eE => 
				s <= '1';
			when others => 
				s <= '0';
		end case; -- fin de la structure case
	end process;
end structural;
