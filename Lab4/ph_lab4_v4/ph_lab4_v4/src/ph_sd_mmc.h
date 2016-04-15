/*
 * ph_sd_mmc.h
 *
 * Created: 03.04.2016 22:00:12
 *  Author: Petter
 */ 


#ifndef PH_SD_MMC_H_
#define PH_SD_MMC_H_

#include "conf_sd_mmc_spi.h"

static void sd_mmc_resources_init(uint32_t pba_clk)
{
	// GPIO pins used for SD/MMC interface
	static const gpio_map_t SD_MMC_SPI_GPIO_MAP =
	{
		{SD_MMC_SPI_SCK_PIN,  SD_MMC_SPI_SCK_FUNCTION },  // SPI Clock.
		{SD_MMC_SPI_MISO_PIN, SD_MMC_SPI_MISO_FUNCTION},  // MISO.
		{SD_MMC_SPI_MOSI_PIN, SD_MMC_SPI_MOSI_FUNCTION},  // MOSI.
		{SD_MMC_SPI_NPCS_PIN, SD_MMC_SPI_NPCS_FUNCTION}   // Chip Select NPCS.
	};

	// SPI options.
	spi_options_t spiOptions =
	{
		.reg          = SD_MMC_SPI_NPCS,
		.baudrate     = SD_MMC_SPI_MASTER_SPEED,  // Defined in conf_sd_mmc_spi.h.
		.bits         = SD_MMC_SPI_BITS,          // Defined in conf_sd_mmc_spi.h.
		.spck_delay   = 0,
		.trans_delay  = 0,
		.stay_act     = 1,
		.spi_mode     = 0,
		.modfdis      = 1
	};

	// Assign I/Os to SPI.
	gpio_enable_module(SD_MMC_SPI_GPIO_MAP,
	sizeof(SD_MMC_SPI_GPIO_MAP) / sizeof(SD_MMC_SPI_GPIO_MAP[0]));

	// Initialize as master.
	spi_initMaster(SD_MMC_SPI, &spiOptions);

	// Set SPI selection mode: variable_ps, pcs_decode, delay.
	spi_selectionMode(SD_MMC_SPI, 0, 0, 0);

	// Enable SPI module.
	spi_enable(SD_MMC_SPI);

	// Initialize SD/MMC driver with SPI clock (PBA).
	sd_mmc_spi_init(spiOptions, pba_clk);
}




#endif /* PH_SD_MMC_H_ */