---------------------------------------
-- lfsr.vhd
---------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity lfsr is
    port ( clk:     in  std_logic;
           reset:   in  std_logic;
           s:  out std_logic);
end lfsr;

architecture mixte of lfsr is

    component basc_D is
        port (  clk:    in  std_logic;
                d:      in  std_logic;
                q:      out std_logic;
                qb:      out std_logic;
                reset:  in  std_logic);
    end component;

    signal d3, d2, d1, d0, q0: std_logic;

begin

    d3 <= d0 xnor q0;
    s <= q0;

    FD0: basc_D
    port map (  clk     => clk,
                d       => d0,
                q       => q0,
                qb      => open, -- "open" est un mot-clé indiquant qu'une
                                  -- sortie de l'affectation est inutilisée.
                reset   => reset);

    
    FD3: basc_D
    port map (  clk     => clk,
                d       => d3,
                q       => open,
                qb      => d2, -- "open" est un mot-clé indiquant qu'une
                                  -- sortie de l'affectation est inutilisée.
                reset   => reset);


    FD2: basc_D
    port map (  clk     => clk,
                d       => d2,
                q       => d1,
                qb      => open, -- "open" est un mot-clé indiquant qu'une
                                  -- sortie de l'affectation est inutilisée.
                reset   => reset);
                
    FD1: basc_D
    port map (  clk     => clk,
                d       => d1,
                q       => d0,
                qb      => open, -- "open" est un mot-clé indiquant qu'une
                                  -- sortie de l'affectation est inutilisée.
                reset   => reset);

    -- A compléter
end mixte;

