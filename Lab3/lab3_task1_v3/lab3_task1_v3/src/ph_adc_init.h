/*
 * ph_adc_init.h
 *
 * Created: 12.03.2016 18:53:05
 *  Author: Petter
 */ 


#ifndef PH_ADC_INIT_H_
#define PH_ADC_INIT_H_

//ADC defines
#define EXAMPLE_ADC_POTENTIOMETER_CHANNEL   1
#define EXAMPLE_ADC_POTENTIOMETER_PIN       AVR32_ADC_AD_1_PIN
#define EXAMPLE_ADC_POTENTIOMETER_FUNCTION  AVR32_ADC_AD_1_FUNCTION



static void adc_init_pot (void)
{	


/** GPIO pin/adc-function map. */
const gpio_map_t ADC_GPIO_MAP = {
	{EXAMPLE_ADC_POTENTIOMETER_PIN, EXAMPLE_ADC_POTENTIOMETER_FUNCTION}
};	
	
/* Assign and enable GPIO pins to the ADC function. */
	gpio_enable_module(ADC_GPIO_MAP, sizeof(ADC_GPIO_MAP) /
	sizeof(ADC_GPIO_MAP[0]));
	
	/* Configure the ADC peripheral module.
	 * Lower the ADC clock to match the ADC characteristics (because we
	 * configured the CPU clock to 12MHz, and the ADC clock characteristics are
	 *  usually lower; cf. the ADC Characteristic section in the datasheet). */
	AVR32_ADC.mr |= 0x1 << AVR32_ADC_MR_PRESCAL_OFFSET;
	adc_configure(&AVR32_ADC);

	/* Enable the ADC channels. */
	adc_enable(&AVR32_ADC, EXAMPLE_ADC_POTENTIOMETER_CHANNEL);
}



#endif /* PH_ADC_INIT_H_ */