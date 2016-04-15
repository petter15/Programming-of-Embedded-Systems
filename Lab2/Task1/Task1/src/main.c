/*
Class: Programming of Embedded Systems
Author: Petter Haugen
Excercise: Lab #2, task #1
 */

#include <asf.h>


//Interrupt routine for switch PB0

__attribute__((__interrupt__))
static void int_handler (void)
{
	if(gpio_get_pin_interrupt_flag(GPIO_PUSH_BUTTON_0))
	{
		LED_On(LED1);
		LED_Off(LED0);
		usart_write_line(USART, "This is PB0 interrupt \n");
		gpio_clear_pin_interrupt_flag(GPIO_PUSH_BUTTON_0);
	}
}

int main (void)
{
	
	// Set OSC0 as main CPU clock (12 MHz)
	sysclk_init();
	
	// Initialize the EVK1100
	board_init();
	
	
	gpio_enable_gpio_pin(LED1_GPIO);
	gpio_configure_pin(LED1_GPIO, GPIO_DIR_OUTPUT);
	
	
	
	// Initialize interrupt vector table support.
	irq_initialize_vectors();

	//Enable pin interrupts
	gpio_enable_pin_interrupt(GPIO_PUSH_BUTTON_0, GPIO_RISING_EDGE);

	//Initialize interrupt controller
	INTC_init_interrupts ();

	//Set interrupt handler
	INTC_register_interrupt(&int_handler,(AVR32_GPIO_IRQ_0+88/8), AVR32_INTC_INT0);

	// Enable global interrupts
	cpu_irq_enable();
	
	
	// Define USART GPIO pin map
	static const gpio_map_t USART_GPIO_MAP =
	{
		{USART_RXD_PIN, USART_RXD_FUNCTION},
		{USART_TXD_PIN, USART_TXD_FUNCTION}
	};

	// Define USART options
	static usart_options_t usart_options =
	{
		.baudrate		= 9600,
		.charlength		= 8,
		.paritytype		= USART_NO_PARITY,
		.stopbits		= USART_1_STOPBIT,
		.channelmode	= USART_NORMAL_CHMODE
	};
	
	// Assign GPIO
	gpio_enable_module(	USART_GPIO_MAP,
	sizeof(USART_GPIO_MAP) / sizeof(USART_GPIO_MAP[0]) );
	
	// Initialize USART
	usart_init_rs232(USART, &usart_options, sysclk_get_pba_hz());
	gpio_set_pin_low(LED1_GPIO);
	
	
	//Initial state
	LED_On(LED0);
	LED_Off(LED1);
	
	while(true){
		if (LED_Test(LED1))
		usart_write_line(USART, "LED2 is on\n");
		delay_ms(500);
		if (LED_Test(LED0))
		usart_write_line(USART, "LED1 is on\n");
		delay_ms(500);
	}
}
