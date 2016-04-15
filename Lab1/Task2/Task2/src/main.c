/*
Class: Programming of Embedded Systems
Author: Petter Haugen
Excercise: Lab #1, task #2
 */


#include <asf.h>
#include <gpio.h>
#include <evk1100.h>
#include <pm.h>

int main (void)
{
	//Initialize and start clock to osc0
	//OSC0 is set to 12 Mhz in the evk11.h file
	pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);		
	
	//Initialize board
	board_init();
	
	//Enable PB0 as input-pin, active low											
	gpio_enable_gpio_pin(GPIO_PUSH_BUTTON_0);			
	
	//Enable LED6 as GPIO-pin, active low
	gpio_enable_gpio_pin(LED6_GPIO);
	
	//Enable LED& as output, active low
	gpio_configure_pin(LED6_GPIO, GPIO_DIR_OUTPUT);	
	
	while(1){
	
	//If PB0 is pressed, e.g pin is low
	if (gpio_pin_is_low(GPIO_PUSH_BUTTON_0))	
	{
		//Activate LED6 by setting gpio-pin low
		gpio_set_pin_low(LED6_GPIO);								
	} 
	else
	{
		//Else, deactivate LED6 by setting gpio-pin high
		gpio_set_pin_high(LED6_GPIO);	
	}	
	}
}
