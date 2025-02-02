---------------------------------------
-- tb_AC.vhd
-- Testbench pour le composant "AC"
---------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Un testbench est une entité ("tb_lfsr"), qui va instancier le composant
-- testé et lui fournir des stimuli. Il n'a donc aucune entrée-sortie.
entity tb_AC is
end tb_AC;

architecture banc of tb_AC is
    -- Pour pouvoir instancier un composant (description structurelle), il faut
    -- au préalable le déclarer.
    -- Ici, nous déclarons le composant "AC" que nous venons de décrire dans le fichier
    --  "vhd/AC.vhd", et que nous souhaitons tester.
        component AC is
                port ( a:  in  std_logic;
                       b:  in  std_logic;
                       c:  in  std_logic;
                       s:  out std_logic;
                       co: out std_logic);
        end component; -- Remarquez la similitude entre la définition d'une entité et la
                       -- déclaration d'un composant.

        signal tb_a,tb_b,tb_c, tb_s, tb_co : std_logic;
begin
    -- Par exemple, le code suivant permet d'instancier un composant AC, nommé
    -- "Mon_AC".

    -- Le composant testé est instancié grace au mot-clé "port map" et ses
    -- entrées/sorties sont connectées aux signaux du testbench.
        Mon_AC: AC -- l'instance, nommée Mon_AC, est un composant AC.
        port map (
                         a  => tb_a, -- Connexion du port "a" de l'instance au signal "tb_a"
                         b  => tb_b, -- Attention à ne pas confondre l'opérateur d'association (=>) avec celui
                                     -- d'affectation concurrente
                         c  => tb_c,
                         s  => tb_s,
                         co => tb_co);

    -- Cette partie décrit les stimuli à appliquer à l'entrée du composant à tester.
    -- On utilise un processus ("process") qui agit comme une boucle infinie
    -- répétant à l'infini le contenu entre le begin et le end.
        process
        begin
                tb_a <= '0'; -- La valeur d'un std_logic est entouré de simple quote.
                tb_b <= '0';
                tb_c <= '0';
                wait for 5 ns; -- On indique au simulateur de laisser passer un peu de temps.
                -- Le mot-clé "wait" ne peut être utilisé que dans un testbench
                -- Avec cette conbinaison d'entrée, le circuit devrait donner des valeurs
                -- nulles pour s et co. La syntaxe suivant permet de le vérifier et levera une
                -- erreur en cas de soucis.
                assert ( tb_s = '0') report "==== ERREUR : Sortie S fausse" severity error;
                assert ( tb_co = '0') report "==== ERREUR : Sortie CO fausse" severity error;

                tb_a <= '1'; -- On teste maintenant la combinaison d'entrée "001"
                wait for 5 ns;
                assert ( tb_s = '1') report "==== ERREUR : Sortie S fausse" severity error;
                assert ( tb_co = '0') report "==== ERREUR : Sortie CO fausse" severity error;

                tb_b <= '1'; -- On teste maintenant la combinaison d'entrée "011"
                wait for 5 ns;
                assert ( tb_s = '0') report "==== ERREUR : Sortie S fausse" severity error;
                assert ( tb_co = '1') report "==== ERREUR : Sortie CO fausse" severity error;

                tb_c <= '1'; -- On teste maintenant la combinaison d'entrée "111"
                wait for 5 ns;
                assert ( tb_s = '1') report "==== ERREUR : Sortie S fausse" severity error;
                assert ( tb_co = '1') report "==== ERREUR : Sortie CO fausse" severity error;

                tb_b <= '0'; -- On teste maintenant la combinaison d'entrée "101"
                wait for 5 ns;
                assert ( tb_s = '0') report "==== ERREUR : Sortie S fausse" severity error;
                assert ( tb_co = '1') report "==== ERREUR : Sortie CO fausse" severity error;

                tb_a <= '0'; -- On teste maintenant la combinaison d'entrée "100"
                wait for 5 ns;
                assert ( tb_s = '1') report "==== ERREUR : Sortie S fausse" severity error;
                assert ( tb_co = '0') report "==== ERREUR : Sortie CO fausse" severity error;

                tb_b <= '1'; -- On teste maintenant la combinaison d'entrée "110"
                wait for 5 ns;
                assert ( tb_s = '0') report "==== ERREUR : Sortie S fausse" severity error;
                assert ( tb_co = '1') report "==== ERREUR : Sortie CO fausse" severity error;

                tb_c <= '0'; -- On teste maintenant la combinaison d'entrée "010"
                wait for 5 ns;
                assert ( tb_s = '1') report "==== ERREUR : Sortie S fausse" severity error;
                assert ( tb_co = '0') report "==== ERREUR : Sortie CO fausse" severity error;

                -- Avez-vous remarqué que les combinaisons testées suivaient un code de Gray ?
                -- Quel avantage par rapport à un codage dans l'ordre naturel ?

                -- A ce stade, on a testé exhaustivement les entrées.
                -- On peut arrêter le simulateur avec la commande suivante.
                -- Sans cette commande, le processus rebouclerait à l'infini et le simulateur aussi.
                -- Vous pourrez le vérifier facilement en commentant la ligne.
                report "Simulation terminée" severity failure;
        end process;
end banc;

