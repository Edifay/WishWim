-- Ceci est une ligne de commentaires

---------------------------------------
-- AC.vhd
-- Ce fichier décrit le composant "AC" dans le langage VHDL.
---------------------------------------

-- Les mot-clés "library" et "use" permettent d'indiquer le nom des
-- bibliothèques que ce composant va utiliser.
-- La bibliothèque STD_LOGIC_1164 définit le type std_logic utilisé pour décrire
-- un bit
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- En VHDL, un composant se décrit en deux parties : une partie entité et une
-- partie architecture.
-- Avec le mot-clé "entity", on définit un composant et ses entrée/sorties.
entity AC is -- Notre composant est nommé AC
    port ( -- après le mot-clé port, on liste les entrées-sorties de notre composant.
           a:  in  std_logic; -- un port d'entrée (mot-clé in) "a" sur un bit (type std_logic)
           b:  in  std_logic; -- une deuxième entrée, "b"
           c:  in  std_logic;
           s:  out std_logic; -- une sortie (mot-clé out), nommée "s", sur un bit
           co: out std_logic
         ); -- Notez la position des points-virgules
end AC; -- Notez que la syntaxe reprend le nom donné au composant

-- Avec le mot-clé "architecture", on décrit le fonctionnement du circuit.
-- Ce fonctionnement peut être décrit de manière comportementale (description
-- effective de la fonctionnalité) ou structurelle (instanciation de composants
-- définis par ailleurs, càd soit dans une bibliothèque déclarée, soit dans le
-- projet en cours).
-- Ici, l'architecture du composant AC s'appelle "comportemental".
architecture comportemental of AC is
    -- Avec le mot-clé "signal", on déclare les signaux internes du circuit.
    -- Pour simplifier, ces signaux correspondent aux fils de votre schéma.
    signal bla: std_logic; -- bla est déclaré comme un signal interne contenant
                           -- un bit.
    signal a_and_b: std_logic;
    signal bla_and_c: std_logic;
begin
    -- Il existe de multiples façons de décrire le circuit d'un point de vue
    -- comportemental. L'affectation concurrente ("<=") est la plus simple.
    -- D'un point logique, il est possible d'utiliser les opérateurs booléens
    -- suivants : and, or, nand, nor, xor, xnor, not.

    bla <= a xor b; -- Affectation du xor des entrées a et b dans le signal bla
    s <= bla xor c; -- Affectation du résultat dans la sortie s
    a_and_b <= a and b;
    bla_and_c <= bla and c;
    co <= a_and_b or bla_and_c;
    -- Complétez maintenant pour décrire la sortie co
end comportemental; -- Ici aussi, on recopie le nom donné à l'architecture

