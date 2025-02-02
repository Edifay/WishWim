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

    signal Etat_courant, Etat_futur: unsigned(2 downto 0);
    
    signal d0: unsigned(0 downto 0);
    signal d1: unsigned(0 downto 0);
    signal d2: unsigned(0 downto 0);
    
    
    signal q0: unsigned(0 downto 0);
    signal q1: unsigned(0 downto 0);
    signal q2: unsigned(0 downto 0);
begin
   
   
	d2(0) <= q1(0) and q0(0) and e;
	d1(0) <= (q1(0) and not q0(0)) or ((not e) and (q2(0) or q0(0)));
	d0(0) <= e and ((not q0(0)) or (not q1(0)));

	
	reg0 : reggene
	generic map(
		n => 1
	)
	port map (
		d => d0,
		clk => clk,
		rst => reset,
		q => q0
	);
	
	reg1 : reggene
	generic map (
		n => 1
	)
	port map (
		d => d1,
		clk => clk,
		rst => reset,
		q => q1
	);
	
	reg2 : reggene
	generic map (
		n => 1
	)
	port map (
		d => d2,
		clk => clk,
		rst => reset,
		q => q2
	);
	
	s <= q2(0);

end structural;

