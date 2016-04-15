/*
 * ph_spi_disp_init.h
 *
 * Created: 12.03.2016 18:39:59
 *  Author: Petter
 */ 


#ifndef PH_SPI_DISP_INIT_H_
#define PH_SPI_DISP_INIT_H_

//Initialize LCD display
static void init_disp (uint32_t pba_clk)
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
		.baudrate     = 384000,
		.bits         = 8,
		.spck_delay   = 0,
		.trans_delay  = 0,
		.stay_act     = 1,
		.spi_mode     = 0,
	.modfdis      = 1};
	
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
	spi_setupChipReg(DIP204_SPI, &spiOptions, pba_clk);
	
	// initialize LCD
	dip204_init(backlight_IO, true);
	// Hide cursor
	dip204_hide_cursor();

}


#endif /* PH_SPI_DISP_INIT_H_ */