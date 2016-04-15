/*
 * ph_pwm_init.h
 *
 * Created: 12.03.2016 18:56:40
 *  Author: Petter
 */ 


#ifndef PH_PWM_INIT_H_
#define PH_PWM_INIT_H_


//PWM defines
#define EXAMPLE_PWM_PIN             AVR32_PWM_3_PIN
#define EXAMPLE_PWM_FUNCTION        AVR32_PWM_3_FUNCTION
#define EXAMPLE_PWM_CHANNEL_ID      3
//Corresponding pin: PB22

avr32_pwm_channel_t		pwm_channel;
	
static void ph_init_pwm (void)
{
	/* PWM controller configuration. */
	pwm_opt_t pwm_opt =
		{
			.diva = AVR32_PWM_DIVA_CLK_OFF,
			.divb = AVR32_PWM_DIVB_CLK_OFF,
			.prea = AVR32_PWM_PREA_MCK,
			.preb = AVR32_PWM_PREB_MCK
		};

	//Initial settings: 
	//cdty=100
	//cprd=1000
	//Meaning a duty cycle of 90 % and a frequency of 12 Khz
	//Not really important, cause we will provide new values for cdty and cprd immediately when we enter the case structure in while loop.
	
	/* With these settings, the output waveform period will be:
	 * (PBA_HZ/1)/1000 ==12kHz == (MCK/prescaler)/period, with
	 * MCK == 12000000Hz, prescaler == 1, period == 20. */
	pwm_channel.cdty = 100; /* Channel duty cycle, should be < CPRD. */
	pwm_channel.cprd = 1000; /* Channel period. */
	pwm_channel.cupd = 0; /* Channel update is not used here. */
	pwm_channel.CMR.calg = PWM_MODE_LEFT_ALIGNED; /* Channel mode. */
	pwm_channel.CMR.cpol = PWM_POLARITY_LOW;      /* Channel polarity. */
	pwm_channel.CMR.cpd = PWM_UPDATE_DUTY;        /* Not used the first time. */
	pwm_channel.CMR.cpre = AVR32_PWM_CPRE_MCK; /* Channel prescaler. */	//No prescaler = 1

	/* Enable the alternative mode of the output pin to connect it to the PWM
	 * module within the device. */
	gpio_enable_module_pin(EXAMPLE_PWM_PIN, EXAMPLE_PWM_FUNCTION);

	/* Initialize the PWM module. */
	pwm_init(&pwm_opt);

	/* Set channel configuration to channel 0. */
	pwm_channel_init(EXAMPLE_PWM_CHANNEL_ID, &pwm_channel);

	/* Start channel 0. */
	pwm_start_channels(1 << EXAMPLE_PWM_CHANNEL_ID);
	
}




#endif /* PH_PWM_INIT_H_ */