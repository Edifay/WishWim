library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

ENTITY VGA IS
    PORT( CLK : IN std_logic;
          HS : OUT std_logic;
          VS : OUT std_logic;
          R : OUT unsigned(4 downto 0);
          G : OUT unsigned(5 downto 0);
          B : OUT unsigned(4 downto 0)
      );
END VGA;

ARCHITECTURE Behavioral OF VGA IS

    COMPONENT GeneSync
        Port ( CLK : in std_logic;
               HSYNC : out std_logic;
               VSYNC : out std_logic;
               IMG : out std_logic;
               X : out unsigned(8 downto 0);
               Y : out unsigned(7 downto 0)
           );
    END COMPONENT;

    component GeneRGB
	    Port (
			 X: in unsigned(8 downto 0);
			 Y: in unsigned(7 downto 0);
			 DATA: in unsigned(31 downto 0);
			 R : OUT unsigned(4 downto 0);
			 G : OUT unsigned(5 downto 0);
			 B : OUT unsigned(4 downto 0);
			 ADDR: out unsigned(14 downto 0)
		 );
    end component;

    component RAM_VIDEO is
  	port (
	    clk  : in    std_logic;
	    addr : in    unsigned (14 downto 0);
	    do   : out   unsigned(31 downto 0);
            we   : in    std_logic
	    );
    end component;

    -- Signaux internes
    signal AD: unsigned(14 downto 0);
    signal DATA: unsigned(31 downto 0);
    signal Xi : unsigned(8 downto 0);
    signal Yi : unsigned(7 downto 0);
    signal IMGi: std_logic;
    signal Ri : unsigned(4 downto 0);
    signal Gi : unsigned(5 downto 0);
    signal Bi : unsigned(4 downto 0);
BEGIN
    cRAM_VIDEO: RAM_VIDEO port map(CLK => CLK, ADDR => AD, DO =>DATA, WE=> '0');
    cGeneSync: GeneSync PORT MAP(HSYNC => HS, VSYNC => VS, IMG => IMGi,
                                 X => Xi, Y => Yi, CLK => CLK);
    cGeneGRB: GeneRGB port map(X => Xi, Y => Yi, R => Ri, G => Gi, B => Bi, DATA => DATA, ADDR => AD);

    R <= Ri when IMGi ='1' else (others =>'0');
    G <= Gi when IMGi ='1' else (others =>'0');
    B <= Bi when IMGi ='1' else (others =>'0');

END Behavioral;
