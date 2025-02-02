library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library xpm;
use xpm.vcomponents.all;


-- RAM 24 bits
entity RAM_Video is
    port (
        -- Clock/Reset
        clk  : in  std_logic ;

        -- Memory slave interface
        addr : in  unsigned (14 downto 0) ;
        do   : out unsigned (31 downto 0) ;
        we   : in  std_logic
    );
end entity;


architecture structural of RAM_Video is

    -- Constants
    constant MEMORY_SIZE : positive := 320*240*8 ; -- bit size
    constant FILE_NAME   : string   := "img.mem" ;
    constant ZERO        : std_logic_vector ( do'range ) := (others => '0') ;

    -- Signals
    signal s_do : std_logic_vector ( do'range ) ;
    signal s_we : std_logic_vector ( 0 downto 0 ) ;
begin

    -- No byte strobes
    s_we <= (others => we);

    -- Cast std_logic_sector to unsigned
    --do <= "000000000000000" & unsigned(s_do);
    do <= unsigned(s_do);

    -- Memory XPM Single Port RAM
    XPM_RAM : XPM_MEMORY_SPRAM
--    XPM_RAM : XPM_MEMORY_SPROM
        generic map (
            -- Memory configuration
            MEMORY_SIZE         => MEMORY_SIZE,
            MEMORY_PRIMITIVE    => "auto",
            MEMORY_OPTIMIZATION => "true",
            MEMORY_INIT_FILE    => FILE_NAME,
            USE_MEM_INIT        => 1,
            ECC_MODE            => "no_ecc",

            -- Port A: Port configuration
            ADDR_WIDTH_A        => 15,
            --BYTE_WRITE_WIDTH_A  => 9,
            --WRITE_DATA_WIDTH_A  => 9,
            --READ_DATA_WIDTH_A   => 9,
            WRITE_DATA_WIDTH_A  => 32,
            READ_DATA_WIDTH_A   => 32,
            WRITE_MODE_A        => "read_first",
            READ_LATENCY_A      => 1
        )
        port map (
            -- Clock/Reset
            clkA   => clk,
            rstA   => '0',
            sleep  => '0',

            -- Port A: Memory slave interface
            addrA  => std_logic_vector( addr ),
            enA    => '1',
            weA    => s_we,
            dinA   => ZERO,
            doutA  => s_do,
            regceA => '0',

            -- Error injection : off
            sbiterrA       => open,
            dbiterrA       => open,
            injectsbiterrA => '0',
            injectdbiterrA => '0'
        );


end architecture;
