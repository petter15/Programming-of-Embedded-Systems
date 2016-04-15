/*
Class: Programming of Embedded Systems
Author: Petter Haugen
Excercise: Lab #1, task #3
 */

#include <asf.h>
#include <pm.h>
#include <spi.h>
#include <evk1100.h>
#include <board.h>
#include <compiler.h>
#include <dip204.h>
#include <gpio.h>
#include <delay.h>
#include <spi.h>
#include <avr32/io.h>
#include <stdio.h>

void init_disp (void);

//Initialize LCD display
void init_disp (void)
{
	static const gpio_map_t DIP204_SPI_GPIO_MAP =
	{
		{DIP204_SPI_SCK_PIN,  DIP204_SPI_SCK_FUNCTION },  // SPI Clock.
		{DIP204_SPI_MISO_PIN, DIP204_SPI_MISO_FUNCTION},  // MISO.
		{DIP204_SPI_MOSI_PIN, DIP204_SPI_MOSI_FUNCTION},  // MOSI.
		{DIP204_SPI_NPCS_PIN, DIP204_SPI_NPCS_FUNCTION}   // Chip Select NPCS.
	};
	

	// add the spi options driver structure for the LCD DIP204
	spi_options_t spiOptions =
	{
		.reg          = DIP204_SPI_NPCS,
		.baudrate     = 1000000,
		.bits         = 8,
		.spck_delay   = 0,
		.trans_delay  = 0,
		.stay_act     = 1,
		.spi_mode     = 0,
		.modfdis      = 1
	};

	// Assign I/Os to SPI
	gpio_enable_module(DIP204_SPI_GPIO_MAP,
	sizeof(DIP204_SPI_GPIO_MAP) / sizeof(DIP204_SPI_GPIO_MAP[0]));

	// Initialize as master
	spi_initMaster(DIP204_SPI, &spiOptions);

	// Set selection mode: variable_ps, pcs_decode, delay
	spi_selectionMode(DIP204_SPI, 0, 0, 0);

	// Enable SPI
	spi_enable(DIP204_SPI);

	// setup chip registers
	spi_setupChipReg(DIP204_SPI, &spiOptions, FOSC0);
	
	// initialize LCD
	dip204_init(backlight_IO, true);
	
	dip204_hide_cursor();
}


int main (void)
{		
	// Switch the CPU main clock to oscillator 0
	pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);	
	
	board_init();
	init_disp(); 


	U32 x = 12345678;
	U32 y = 87654321;
	U64 z =0;
	
	F32 a = 1234.5678;
	F32 b = 8765.4321;
	F32 c = 0;
	
	U32 calc_time_z = 0;
	U32 calc_time_c = 0;
	
	U32 cnt_1 = 0;
	U32 cnt_2 = 0;
	U32 cnt_3 = 0;
	U32 cnt_res_z = 0; 
	U32 cnt_res_c = 0; 
	
	char cycl_str_z[9];
	char cycl_str_c[9];
	char result[19];	
	char cycl_c[9];
	char cycl_z[9];
	
	//Calculation:
	
	//Cycle count 1
	cnt_1 = Get_sys_count();
	
	//Calculation part 1:
	z = x*y;
	
	//Cycle count 2
	cnt_2 = Get_sys_count();
	
	//Calculation part 2:
	c = a*b;
	
	//Cycle count 3
	cnt_3 = Get_sys_count();
	
	//Cycle count result
	cnt_res_z = cnt_2 - cnt_1 ;
	cnt_res_c = cnt_3 - cnt_2 ;
	
	//Use cycle count result to find calculation time
	calc_time_c = (cnt_res_c * 1000000 + FOSC0 - 1) / FOSC0;
	calc_time_z = (cnt_res_z * 1000000 + FOSC0 - 1) / FOSC0;
	
	//Compose strings for display output
	sprintf(result, "%f", c);
	sprintf(cycl_str_z, "%lu", calc_time_z);
	sprintf(cycl_str_c, "%lu", calc_time_c);
	sprintf(cycl_c, "%lu", cnt_res_c);
	sprintf(cycl_z, "%lu", cnt_res_z);
	
	//Display calculation time, cycles and multiplication result on monitor
	dip204_clear_display();

	dip204_set_cursor_position(1,1);
	dip204_write_string("x*y=z");
	dip204_set_cursor_position(1,2);
	dip204_write_string("Time:");
	dip204_set_cursor_position(7,2);
	dip204_write_string(cycl_str_z);
	dip204_set_cursor_position(1,3);
	dip204_write_string("Cycles:");
	dip204_set_cursor_position(9,3);
	dip204_write_string(cycl_z);

	dip204_set_cursor_position(11,1);
	dip204_write_string("a*b=c");
	dip204_set_cursor_position(11,2);
	dip204_write_string("Time:");
	dip204_set_cursor_position(17,2);
	dip204_write_string(cycl_str_c);
	dip204_set_cursor_position(11,3);
	dip204_write_string("Cycles:");
	dip204_set_cursor_position(19,3);
	dip204_write_string(cycl_c);

	while (1)
	{
		
	}
} 