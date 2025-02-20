/*------------------------------------------------------------------------------

Header block comment
------------------------------------------------------------------------------*/

library IEEE;
    use IEEE.std_logic_1164.all;
    use IEEE.numeric_std.all;

library work;
    use work.MyConstants.all;
    use work.MyRegisters.all;
--------------------------------------------------------------------------------

`if VHDL_VERSION > "2000" then
  `warning "This version is cool"
`else
  `error "This version is bad"
`end if
--------------------------------------------------------------------------------

package interfaces is
  type streaming_bus is record               -- the record definition
    data  : std_logic_vector(7 downto 0);
    valid : std_logic;
    ready : std_logic;
  end record;

  view streaming_master of streaming_bus is  -- the mode view of the record
    valid, data : out;
    ready       : in;
  end view;

  alias streaming_slave is streaming_master'converse;

  function MyFunc (x: integer) return real;
end interfaces;
--------------------------------------------------------------------------------

package body interfaces is
  function MyFunc (x: integer) return real is
    variable a, b, c: integer;
  begin
    return real(x + a) / real(b*c);
  end function MyFunc;
end package body interfaces;
--------------------------------------------------------------------------------

entity Processor is
  generic(CLK_FREQ : integer);
  port(
    clk, rst : in   std_logic;
    adc_data : view streaming_slave;
    ddc_data : view streaming_master
  );
end entity Processor;
--------------------------------------------------------------------------------

architecture Behaviour of Processor is
  signal clk, rst        : std_logic;
  signal bus_in, bus_out : streaming_bus;

  type IntArray is array 1 to 3 of integer;
  constant integers    : IntArray   := (123, 7#1645#, 13#AC83#);
  variable reals       : RealArray  := (123.456, 7#16.45#e7, 13#AC.83#);
  signal   logic       : LogicArray := ("1010", b"01101", sx"123ABC", d"123456");
  signal   sized_logic : LogicArray := (5b"01101", 24sx"123ABC", 20d"123456");
  constant strings     : String     := "Hello World!!!";
begin
  producer : entity work.source port map(clk, rst, bus_in);
  consumer : entity work.sink   port map(clk, rst, bus_out);

  bus_in.ready <= bus_out.ready;

  digital_downconverter: process(clk) is
    variable temp : integer;
  begin
    if(rising_edge(clk)) then
      if(reset) then
        bus_out <= (data => (others => 'X'), valid => '0');
      else
        if(bus_out.ready) then
          -- TODO: Implement the DDC
          bus_out.data  <= bus_in.data;
          bus_out.valid <= bus_in.valid;
        else
          bus_out.valid <= '0';
        end if;
      end if;
    end if;
  end process digital_downconverter;
end Behaviour;
--------------------------------------------------------------------------------

