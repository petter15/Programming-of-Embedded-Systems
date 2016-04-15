/*
Class: Programming of Embedded Systems
Author: Petter Haugen
Excercise: Lab #2, task #2
*/

#include "asf.h"
#include "stdio.h"
#include "conf_access.h"
#include "conf_sd_mmc_spi.h"

//SDRAM status LEDs for read/write
#define LED_SDRAM_WRITE     LED_BI0_RED
#define LED_SDRAM_READ      LED_BI0_GREEN

//! \Clock frequencies (hz)
#define CPU_HZ				  60000000		// CPU clock setting
#define PBA_HZ                30000000		//Peripheral bus A clock setting
#define UINT32_MAX_VALUE	  4294967295	//Numbers of register entries

//PDCA channel settings
#define AVR32_PDCA_CHANNEL_USED_RX AVR32_PDCA_PID_SPI1_RX
#define AVR32_PDCA_CHANNEL_USED_TX AVR32_PDCA_PID_SPI1_TX

#define AVR32_PDCA_CHANNEL_SPI_RX 0 // PDCA channel 0 for reception
#define AVR32_PDCA_CHANNEL_SPI_TX 1 // PDCA channel 1 for transmission

// PDCA Channel pointer
volatile avr32_pdca_channel_t* pdca_channelrx ;
volatile avr32_pdca_channel_t* pdca_channeltx ;

// Used to indicate the end of PDCA transfer
volatile bool end_of_transfer;

// Local RAM buffer for the example to store data received from the SD/MMC card
volatile char ram_buffer[1000];

//Pointer to SDRAM start address
volatile unsigned long *sdram = SDRAM;


/*! Initializes SD/MMC resources: GPIO, SPI and SD/MMC.
 */
static void sd_mmc_resources_init(void)
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
    .baudrate     = PBA_HZ,					  // SPI speed configured in system clock settings
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
  sd_mmc_spi_init(spiOptions, PBA_HZ);
}

/*! Initialize PDCA (Peripheral DMA Controller A) resources for the SPI transfer and start a dummy transfer
 */
static void local_pdca_init(void)
{
//   this PDCA channel is used for data reception from the SPI
//     pdca_channel_options_t pdca_options_SPI_RX ={ // pdca channel options
//   
//       .addr = ram_buffer,						// memory address.
//       .size = 512,                              // transfer counter: here the size of the string
//       .r_addr = NULL,                           // next memory address after 1st transfer complete
//       .r_size = 0,                              // next transfer counter not used here
//       .pid = AVR32_PDCA_CHANNEL_USED_RX,        // select peripheral ID - data are on reception from SPI1 RX line
//       .transfer_size = PDCA_TRANSFER_SIZE_BYTE  // select size of the transfer: 8,16,32 bits
//     };

  // this channel is used to activate the clock of the SPI by sending a dummy variables
  pdca_channel_options_t pdca_options_SPI_TX ={ // pdca channel options

    .addr = sdram,							  // memory address.                                      
    .size = 512,                              // transfer counter: here the size of the string
    .r_addr = NULL,                           // next memory address after 1st transfer complete
    .r_size = 0,                              // next transfer counter not used here
    .pid = AVR32_PDCA_CHANNEL_USED_TX,        // select peripheral ID - data are on reception from SPI1 TX line
    .transfer_size = PDCA_TRANSFER_SIZE_WORD  // select size of the transfer: 8,16,32 bits
  };

  // Init PDCA transmission channel
  pdca_init_channel(AVR32_PDCA_CHANNEL_SPI_TX, &pdca_options_SPI_TX);

  // Init PDCA Reception channel
//  pdca_init_channel(AVR32_PDCA_CHANNEL_SPI_RX, &pdca_options_SPI_RX);

}

//! Writing using PDCA
//! @brief This function opens a SD_MMC memory in write mode at a given sector
//! address.
//!
//! NOTE: If page buffer > 512 bytes, page content is first loaded in buffer to
//! be partially updated by write_byte or write64 functions.
//!
//! @param  pos   Sector address
//!
//! @return bit
//!   The open succeeded      -> True
//!/
static Bool sd_mmc_spi_write_open_PDCA (uint32_t pos)
{
	uint32_t	gl_ptr_mem;
	uint8_t		r1;
	
	
	// Set the global memory ptr at a Byte address.
	gl_ptr_mem = pos << 9;                    // gl_ptr_mem = pos * 512

	// wait for MMC not busy
	if (false == sd_mmc_spi_wait_not_busy())
	return false;


	spi_selectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);          // select SD_MMC_SPI
	// issue command
	r1 = sd_mmc_spi_command(MMC_WRITE_BLOCK, gl_ptr_mem);

	// check for valid response
	if (r1 != 0x00)
	{
		spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);  // unselect SD_MMC_SPI
		// print_dbg("\n****** could not open SD_MMC_SPI for single block read\n");
		// Trace("\n****** could not open SD_MMC_SPI for single block read\n");
		return false;
	}

	// send dummy
	spi_write(SD_MMC_SPI,0xFF);   // give clock again to end transaction

	// send data start token
	spi_write(SD_MMC_SPI,MMC_STARTBLOCK_WRITE);
	//Toggle SDRAM read LED
	//LED_Toggle(LED_SDRAM_READ);

	return true;   // Read done.
}


//! Stop PDCA transfer
//! @brief This function closes a PDCA write transfer
//! page programming.
//!
static Bool sd_mmc_spi_write_close_PDCA (void)
{
	uint16_t	i;
	uint8_t		r1;

	// load 16-bit CRC (ignored)
	spi_write(SD_MMC_SPI,0xFF);
	spi_write(SD_MMC_SPI,0xFF);

	// read data response token
	r1 = sd_mmc_spi_send_and_read(0xFF);
	if( (r1 & MMC_DR_MASK) != MMC_DR_ACCEPT)
	{
		spi_write(SD_MMC_SPI,0xFF);    // send dummy bytes
		spi_write(SD_MMC_SPI,0xFF);
		spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
		return false;
		//     return r1;             // return ERROR byte
	}

	spi_write(SD_MMC_SPI,0xFF);    // send dummy bytes
	spi_write(SD_MMC_SPI,0xFF);

	// release chip select
	spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);  // unselect SD_MMC_SPI

	// wait card not busy after last programming operation
	i=0;
	while (false == sd_mmc_spi_wait_not_busy())
	{
		i++;
		if (i == 10)
		return false;
	}

	return true;                  // Write done
}




/*! \ Main function.
 */
int main(void)
{
	//Local variables
	 unsigned long sdram_size, i, j, k, progress_inc, num_sec_sd; //32 bit unsigned integer
	
	
	// Configure main CPU clock and peripheral bus speed
	pm_freq_param_t System_Clock = {
		.cpu_f = CPU_HZ,
		.pba_f = PBA_HZ,
		.osc0_f = FOSC0,
		.osc0_startup = OSC0_STARTUP
	};
	pm_configure_clocks(&System_Clock);
	
 
	// Initialize debug RS232 with PBA clock
	init_dbg_rs232(PBA_HZ);

	// Initialize SD/MMC driver resources: GPIO, SPI and SD/MMC.
	sd_mmc_resources_init();

	//Write start message to USART
	print_dbg("\nThis is lab #2, task #2. By Petter Haugen\n");

	// The SDRAM is 32 MB in total, meaning:
	// Number of bits = 268,435,456
	// Number of bytes = 33,554,432
	// Number of 4 byte words = 8,388,608
	// Defining SDRAM size in 32-bit words and write to USART:
	sdram_size = SDRAM_SIZE >> 2;
	print_dbg("\r\nSDRAM size: ");
	print_dbg_ulong(SDRAM_SIZE >> 20);
	print_dbg(" MB\r\n");
	
	
	// Initialize the external SDRAM chip with CPU clock speed.
	sdramc_init(CPU_HZ);
	print_dbg("SDRAM initialized\r\n");

	//Connect to SDRAM via External Bus Interface
	// Setting EBI slave to have fixed default master
	AVR32_HMATRIX.SCFG[AVR32_HMATRIX_SLAVE_EBI].defmstr_type	= AVR32_HMATRIX_DEFMSTR_TYPE_FIXED_DEFAULT;
	
	//Setting EBI slave to have PDCA as a master
	AVR32_HMATRIX.SCFG[AVR32_HMATRIX_SLAVE_EBI].fixed_defmstr	= AVR32_HMATRIX_MASTER_PDCA;
	
	// Determine the increment of SDRAM word address requiring an update of the
	// printed progression status.
	progress_inc = (sdram_size + 50) / 100;
	
	//Write test pattern 0x00 - 0xFF to SDRAM
	
	for (i =0, j=0, k=0; i<sdram_size; i++)
	{
		//Status update for write progress
		if (i== k*progress_inc)
		{
			
			print_dbg("\rFilling SDRAM with test pattern: ");
			print_dbg_ulong(k++);
		}
	
	 //Fill SDRAM position i with value j, and then increment j.
	 sdram[i]=j;
	 j++;
	// LED_Toggle(LED_SDRAM_WRITE);
		//Debugger to ensure test pattern is within limits 0x00 - 0xFF
		if (j>0xFF)
		{
			j=0x00;
		}
	}
 //LED_Off(LED_SDRAM_WRITE);
 print_dbg("\rSDRAM filled with test pattern       \r\n");
 LED_On(LED1);
 //wait(2000);
 
 print_dbg("\rInsert SDcard to transfer test pattern from SDRAM to SDcard \r\n");
 
 // Ask user to insert SD/MMC
 print_dbg("\r\nInsert SD/MMC...");
 
 // Wait for a card to be inserted
 while (!sd_mmc_spi_mem_check());
 print_dbg("\r\nCard detected!");
 
 // Read Card capacity
 sd_mmc_spi_get_capacity();
 print_dbg("\r\n SD card capacity = ");
 print_dbg_ulong(capacity >> 20);
 print_dbg(" MBytes\r\n\n");

 // Initialize PDCA controller before starting a transfer
 local_pdca_init();
 
 //Calculation of the amount SD/MMC sectors needed to fit the entire SDRAM content over to the SD card
 num_sec_sd = sdram_size / 512;
 
 // Determine the increment of steps for progress indicator
 progress_inc = num_sec_sd / 100;
 
  // Loop for sector based transfer of SDRAM content to SD card
  
  for(j = 0, i=0; j < num_sec_sd; j++)
  {		
	  //Opening PDCA write session for sector j
	  if(sd_mmc_spi_write_open_PDCA(j))
	  {
		//Load 512 bytes of the SDRAM content to the SPI_TX channel
		pdca_load_channel(AVR32_PDCA_CHANNEL_SPI_TX, sdram,512);
		
		//Enable PDCA transmission channel
		pdca_enable(AVR32_PDCA_CHANNEL_SPI_TX);
		
		// Wait for transmission to end
		while (!(pdca_get_transfer_status(AVR32_PDCA_CHANNEL_SPI_TX)&2));
	
		
		// Disable PDCA
		pdca_disable(AVR32_PDCA_CHANNEL_SPI_TX);
		
		// Close PCDA write session
		sd_mmc_spi_write_close_PDCA();
		}

    
    // Writing progress to terminal
	
	if (j ==i * progress_inc)
	{
		print_dbg("\nTranfer of SDRAM content to SD card");
		print_dbg_ulong(i++);
		print_dbg_char('%');	
	}
     
  }
  //Turn off SDRAM read LED
  //LED_Off(LED_SDRAM_READ);
  
 //Transmission complete message
 print_dbg("\nTransfer complete");

  while (1);
}
