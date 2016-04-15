/*
Class: Programming of Embedded Systems
Author: Petter Haugen
Excercise: Lab #3, task #1, version # 3
 */

#define SAMPLE_TIME		20				//Sample time in ms
#define CPU_HZ			48000000		//Define desired main clock speed as 12 Mhz
#define PBA_HZ			12000000		//Define maximum PBA frequency as 12 Mhz
#define IS_FREQ			0				//Obtain ADC value for frequency			
#define IS_DUTY			1				//Obtain ADC value for duty cycle


#include "asf.h"
#include "stdio.h"
#include "print_funcs.h"


//Personal includes
#include "ph_spi_disp_init.h"
#include "ph_adc_init.h"
#include "ph_pwm_init.h"
#include "ph_tc_init.h"
#include "ph_adc_to_pwm.h"




//Global variables

uint8_t TC_INT= false;
uint8_t switch_identifier = IS_FREQ;
uint32_t adc_value_pot;
char adc_value_string [9];



static void write_welcome_strings (void)
{
	
	//Display calculation time, cycles and multiplication result on monitor
	dip204_clear_display();

	dip204_set_cursor_position(1,1);
	dip204_write_string("Frequency: ");
		
	dip204_set_cursor_position(1,2);
	dip204_write_string("Dt cycle: ");
	
	dip204_set_cursor_position(16,2);
	dip204_write_string ("%");

	dip204_set_cursor_position(16,3);
	dip204_write_string("    ");
	dip204_set_cursor_position(1,3);
	dip204_write_string("Pot ADC value: ");
	
}


/**
 * \brief TC interrupt.
 * The ISR handles RC compare interrupt and sets the update_timer flag to
 * update the timer value.
 */

__attribute__((__interrupt__))
static void tc_irq(void)
{	

	LED_Toggle(LED0);
	//Enable the interrupt identifier
	TC_INT = true;
	
	// Clear the interrupt flag. This is a side effect of reading the TC SR.
	tc_read_sr(EXAMPLE_TC, EXAMPLE_TC_CHANNEL);
	
}


//Interrupt routine for switch PB0

__attribute__((__interrupt__))
static void int__push_button_handler (void)
{
	if(gpio_get_pin_interrupt_flag(GPIO_PUSH_BUTTON_0))
	{
		//Push button interrupt : Switch between frequency setting and duty cycle
		//Sets the switch identifier to a given value 
		
		LED_Toggle(LED1);
		
		if (switch_identifier == IS_FREQ)
		{
			LED_On(LED4);
			LED_Off(LED5);
			switch_identifier = IS_DUTY;
		}
		
		else if (switch_identifier == IS_DUTY)
		{	
			LED_On(LED5);
			LED_Off(LED4);
			switch_identifier = IS_FREQ;
		}

		gpio_clear_pin_interrupt_flag(GPIO_PUSH_BUTTON_0);
	}
	
}


int main (void){
	
	volatile avr32_tc_t *tc = EXAMPLE_TC;
	
	// Configure main CPU clock and peripheral bus speed
	pm_freq_param_t System_Clock = {
		.cpu_f = CPU_HZ,
		.pba_f = PBA_HZ,
		.osc0_f = FOSC0,
		.osc0_startup = OSC0_STARTUP
	};
	pm_configure_clocks(&System_Clock);
	
	
	//Initialize board
	board_init();
	
	//Initialize LCD display
	init_disp(PBA_HZ);
	
	//Write welcome strings
	write_welcome_strings();
	
	//Initialize debug strings
	init_dbg_rs232(PBA_HZ);
	
	
	//Interrupts init
	// Initialize interrupt vector table support.
	irq_initialize_vectors();

	//Enable pin interrupts
	gpio_enable_pin_interrupt(GPIO_PUSH_BUTTON_0, GPIO_RISING_EDGE);

	//Initialize interrupt controller
	INTC_init_interrupts ();

	//Set interrupt handler //Highest priority level
	INTC_register_interrupt(&int__push_button_handler,(AVR32_GPIO_IRQ_0+88/8), AVR32_INTC_INT0);

	// Register the RTC interrupt handler to the interrupt controller.
	INTC_register_interrupt(&tc_irq, EXAMPLE_TC_IRQ, EXAMPLE_TC_IRQ_PRIORITY);

	// Enable global interrupts
	cpu_irq_enable();
	
	//Initialize ADC
	adc_init_pot();
	
	//Initialize PWM
	ph_init_pwm();
	
	
	//Initialize TimerCounter module
	tc_init(tc, SAMPLE_TIME, PBA_HZ);
	
	//Debug string
	print_dbg("Initialized!\n");
	
	//Start ADC module
	adc_start(&AVR32_ADC);
	
	while(1)
	{
		//print_dbg("This is while loop \n");
		if (TC_INT == true)
		{
			//print_dbg("This is TC interrupt\n");
		//Toggle LED4 to show presence in if statement
		LED_Toggle(LED3);
		
		/* Get value for the potentiometer adc channel */
		adc_value_pot = adc_get_value(&AVR32_ADC, EXAMPLE_ADC_POTENTIOMETER_CHANNEL);
		adc_start(&AVR32_ADC);
			
			
			dip204_set_cursor_position(16,3);
			dip204_write_string("    ");
			dip204_set_cursor_position(16,3);
			sprintf(adc_value_string, "%lu", adc_value_pot);
			dip204_write_string(adc_value_string);
			
			
			
			switch(switch_identifier)	
			{
			case IS_FREQ:
			//print_dbg("This is freq case\n");
			adc_2_pwm(adc_value_pot, PBA_HZ, 0);
			
			/* Set channel configuration to channel 0. */
			pwm_channel_init(EXAMPLE_PWM_CHANNEL_ID, &pwm_channel);
			
			break;
			
			
			case IS_DUTY:
			//print_dbg("This is duty case\n");
			adc_2_pwm(adc_value_pot, PBA_HZ, 1);
			
			/* Set channel configuration to channel 0. */
			pwm_channel_init(EXAMPLE_PWM_CHANNEL_ID, &pwm_channel);
					
			break;
		
				
			default:
			break;
		
			
		
		//End of switch
		}
			
		//Set TC_int to false to disable false triggering	
		TC_INT=false;	
		//End of if statement	
		}
	
	//End of while loop	
	}
	
//End of main
}