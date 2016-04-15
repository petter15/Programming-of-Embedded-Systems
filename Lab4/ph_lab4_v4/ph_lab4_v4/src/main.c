/*
Class: Programming of Embedded Systems
Author: Petter Haugen
Excercise: Lab #4, task #1, version # 4
 */

#define CPU_HZ		sysclk_get_cpu_hz()
#define PBA_HZ		sysclk_get_pba_hz()
#define SAMPLE_TIME 20

#define adc_meas_start	1
#define adc_meas_stop	5

#include <asf.h>

#include "ph_adc_init.h"
#include "ph_tc_init.h"
#include "ph_sd_mmc.h"
#include "conf_explorer.h"
#include "conf_access.h"
#include "string.h"



//Global variables
volatile	uint8_t TC_INT = false;
uint32_t adc_value_pot = 0;
char adc_value_string [9];
char rec_char;			

uint32_t adc_loop_cnt=0;

//! command number
static uint8_t   cmd_type= adc_meas_stop;

//! string length
static uint8_t   i_str = 0;
//! string for command
static char cmd_str[10];



static void fat_parse_cmd(void)
{
	char adc_loop_string [6];
	
	printf("Got command: '%s' \n", cmd_str);
	//If command is start
	if			(strcmp(cmd_str, "start") ==0) {
	
		cmd_type = adc_meas_start;
		file_open(FOPEN_MODE_APPEND);
	
		printf("\nExecuting ADC start measurement\n");
	}
	//else if command is stop
	else if		(strcmp(cmd_str, "stop") ==0) {
		
		cmd_type = adc_meas_stop;
		file_close();
		printf("\nStopping ADC measurement\n");
	
		printf("Number of ADC measurements: ");
		sprintf(adc_loop_string, "%lu", adc_loop_cnt);
		printf (adc_loop_string);
		LED_Off(LED1);
		}
	
	else{
		printf("\nCommand not found, please retry \n ");
		}
}


//Build command line
static void fat_build_cmd(void)
{
	char rec_char;

	if (udi_cdc_is_rx_ready())
	{
		udi_cdc_read_buf(&rec_char, 1);
	
	
		if (rec_char)
		{	
			switch (rec_char)
			{	
				//Carriage return case
				case '\r':	
				// Add new line to indicate carriage return
				printf("%c", rec_char);
				printf(" \r\n");
				// Add NUL char to terminate command string.
				cmd_str[i_str] = '\0';
				// Decode the command.
				fat_parse_cmd();
				i_str = 0;
				break;
			
			
				// Backspace case
				case '\b':
				if (i_str > 0)
				{
				// Replace last char.
				printf("%c",rec_char);
				printf("\b \b");
	
				// Decraese command length.
				i_str--;
				}
				break;
			
				default:
				// Echo.
				printf("%c",rec_char);
				// Append to cmd line.
				cmd_str[i_str] = rec_char;
				i_str++;
				break;
				}
		}
	}
}

/**
 * \brief TC interrupt.
 * The ISR handles RC compare interrupt and sets the TC_INT to perform further actions.
 */

__attribute__((__interrupt__))
static void tc_irq(void)
{	
	LED_Toggle(LED0);
	
	//Enable the interrupt identifier
	TC_INT=true;
	
	// Clear the interrupt flag. This is a side effect of reading the TC SR.
	tc_read_sr(EXAMPLE_TC, EXAMPLE_TC_CHANNEL);	
}



int main (void)
{	
	
	volatile avr32_tc_t *tc = EXAMPLE_TC;	
	
	/* Initialize basic board support features.
	 * - Initialize system clock sources according to device-specific
	 *   configuration parameters supplied in a conf_clock.h file.
	 * - Set up GPIO and board-specific features using additional configuration
	 *   parameters, if any, specified in a conf_board.h file.
	 */
	sysclk_init();
	board_init();
	
	//Disable interrupts
	cpu_irq_disable();

	// Initialize interrupt vector table support.
	irq_initialize_vectors();
	
	//Initialize interrupt controller
	INTC_init_interrupts ();
		
	// Register the RTC interrupt handler to the interrupt controller.
	INTC_register_interrupt(&tc_irq, EXAMPLE_TC_IRQ, EXAMPLE_TC_IRQ_PRIORITY);	
	
	// Enable interrupts
	cpu_irq_enable();

	//Initialize ADC
	adc_init_pot();

	//Initialize timer counter module
	tc_init(tc, SAMPLE_TIME, PBA_HZ);

	/* Call a local utility routine to initialize C-Library Standard I/O over
	 * a USB CDC protocol. Tunable parameters in a conf_usb.h file must be
	 * supplied to configure the USB device correctly.
	 */
	stdio_usb_init();
	
	//Initialize the SPI access to the SD card
	sd_mmc_resources_init(PBA_HZ);
	
	delay_ms(1000);
	
	//print welcome message on terminal window
	printf("This is Embedded Systems, Lab #4 \n");
	printf("By Petter Haugen \n");
	
	if (!sd_mmc_spi_check_presence())
	{
		printf("No SD card detected, try re-inserting and re-setting\n");
		while(!sd_mmc_spi_check_presence())
		{
			//Do nothing, wait
			
		}
	}
	
	printf("SD card detected\n");
	//Select and mount FAT partition on SD card
	nav_reset();
	nav_drive_set(0);
	nav_partition_mount();
	
	
	//create file on root:
	nav_file_create((FS_STRING) "adc_read.txt");
	file_open(FOPEN_MODE_APPEND);
	
	if (!fat_check_open())
	{
		printf("Could not open file!\n");
		while(!fat_check_open())
		{
			//Do nothing, wait
		}
	}
	
	else if (fat_check_open())
	{
		printf("File created and opened\n");
		
	}
	
	//Instruction message
	printf("Type start or stop, followed by enter\n");
	
	while (1)
	{	
		//If new character(s) recognized on USART channel
		fat_build_cmd();

		//If timer/counter interrupt is true and adc meas start command received
		if (cmd_type == adc_meas_start && TC_INT == true)
		{	
			
			/* Get value for the potentiometer adc channel */
			adc_start(&AVR32_ADC);
			adc_value_pot = adc_get_value(&AVR32_ADC, EXAMPLE_ADC_POTENTIOMETER_CHANNEL);
			LED_Toggle(LED1);
			//Increase the ADC loop counter
			adc_loop_cnt++;
			
			//Convert adc value from unsigned int to string
			sprintf(adc_value_string, "%lu", adc_value_pot);
			
			//Folder Cursor is already set to correct folder and opened, start writing to file.
			file_write_buf((uint8_t*)adc_value_string, strlen(adc_value_string));
			file_putc('\r');
			file_putc('\n');
			//Set the timer counter interrupt to false
			TC_INT=false;	
		}
	
		else if (cmd_type == adc_meas_stop)
		{
			LED_Toggle(LED5);
			//Do nothing at all
		}
	}
}
