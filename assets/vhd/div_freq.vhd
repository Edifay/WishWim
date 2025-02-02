---------------------------------------
-- div_freq.vhd
-- Description du diviseur de frequence du TP1 en VHDL
---------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity div_freq is
    port ( clk:     in  std_logic;
           s:  out std_logic;
           reset:  in  std_logic);
end div_freq;

-- Avec le mot-clé "architecture", on décrit le fonctionnement du circuit.
-- Ce fonctionnement peut être décrit de manière comportementale (description
-- effective de la fonctionnalité) ou structurelle (Instanciation de composants
-- définis par ailleurs, càd soit dans une bibliothèque déclarée, soit dans le
-- projet en cours).
architecture structurelle of div_freq is

    -- Pour pouvoir instancier un composant (approche structurelle), il faut
    -- au préalable le déclarer.
    -- Ici, nous déclarons le composant "basc_D" (inclus dans le projet).
    -- Consultez sa définition dans le fichier "vhd/basc_d.vhd".
    component basc_D is
        port (  clk:    in  std_logic;
                d:      in  std_logic;
                q:      out std_logic;
                qb:      out std_logic;
                reset:  in  std_logic);
    end component;

    signal fil: std_logic;

begin
    -- D'un point de vue structurel, on peut instancier des composants avec
    -- le mot-clé "port map"
    -- Par exemple, le code suivant permet d'instancier une bascule D, nommée
    -- "FD0".
    -- Cette bascule est connectée aux signaux du circuit : l'horloge, le fil et la sortie
    FD0: basc_D
    port map (  clk     => clk,         -- Connexion du port "clk" du
                                        -- composant "basc_D" au signal
                                        -- "clk" (entrée du composant div_freq)
                d       => fil,         -- Connexion du port "d" du
                                        -- composant "basc_D" au signal "fil"
                                        -- (qui lui même est connecté à "qb")
                q       => s,           -- Connexion du port "q" du
                                        -- composant "basc_D" au signal
                                        -- "s" sortie du composant "div_freq"
                qb      => fil,         -- Connexion du port "d" du
                                        -- composant "basc_D" au signal "qb"
                reset   => reset);      -- Connexion du port "reset" du
                                        -- composant "basc_D" au signal
                                        -- "reset" (entrée de div_freq)

end structurelle ;

