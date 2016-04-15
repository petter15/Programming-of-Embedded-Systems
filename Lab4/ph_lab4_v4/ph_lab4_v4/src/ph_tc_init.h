/*
 * ph_tc_init.h
 *
 * Created: 03.04.2016 15:44:18
 *  Author: Petter
 */ 


#ifndef PH_TC_INIT_H_
#define PH_TC_INIT_H_

//TC defines
//! \note TC module is used in this example.
#define EXAMPLE_TC                 (&AVR32_TC)
//! \note TC Channel 0 is used.
#define EXAMPLE_TC_CHANNEL         0
//! \note IRQ0 line of channel 0 is used.
#define EXAMPLE_TC_IRQ             AVR32_TC_IRQ0
//! \note IRQ Group of TC module
#define EXAMPLE_TC_IRQ_GROUP       AVR32_TC_IRQ_GROUP
//! \note Interrupt priority 0 is used for TC in this example.
#define EXAMPLE_TC_IRQ_PRIORITY    AVR32_INTC_INT0


//Initialize TC3
/**
 * \brief TC Initialization
 *
 * Initializes and start the TC module with the following:
 * - Counter in Up mode with automatic reset on RC compare match.
 * - fPBA/8 is used as clock source for TC
 * - Enables RC compare match interrupt
 * \param tc Base address of the TC module
 */
static void tc_init(volatile avr32_tc_t *tc, uint32_t time_tick, uint32_t PBA_freq)
{
	float RC_calc;
	uint16_t RC;
	
	// Options for waveform generation.
	static const tc_waveform_opt_t waveform_opt = {
		// Channel selection.
		.channel  = EXAMPLE_TC_CHANNEL,
		// Software trigger effect on TIOB.
		.bswtrg   = TC_EVT_EFFECT_NOOP,
		// External event effect on TIOB.
		.beevt    = TC_EVT_EFFECT_NOOP,
		// RC compare effect on TIOB.
		.bcpc     = TC_EVT_EFFECT_NOOP,
		// RB compare effect on TIOB.
		.bcpb     = TC_EVT_EFFECT_NOOP,
		// Software trigger effect on TIOA.
		.aswtrg   = TC_EVT_EFFECT_NOOP,
		// External event effect on TIOA.
		.aeevt    = TC_EVT_EFFECT_NOOP,
		// RC compare effect on TIOA.
		.acpc     = TC_EVT_EFFECT_NOOP,
		/*
		 * RA compare effect on TIOA.
		 * (other possibilities are none, set and clear).
		 */
		.acpa     = TC_EVT_EFFECT_NOOP,
		/*
		 * Waveform selection: Up mode with automatic trigger(reset)
		 * on RC compare.
		 */
		.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,
		// External event trigger enable.
		.enetrg   = false,
		// External event selection.
		.eevt     = 0,
		// External event edge selection.
		.eevtedg  = TC_SEL_NO_EDGE,
		// Counter disable when RC compare.
		.cpcdis   = false,
		// Counter clock stopped with RC compare.
		.cpcstop  = false,
		// Burst signal selection.
		.burst    = false,
		// Clock inversion.
		.clki     = false,
		// Internal source clock 3, connected to fPBA / 8.
		.tcclks   = TC_CLOCK_SOURCE_TC3
	};

	// Options for enabling TC interrupts
	static const tc_interrupt_t tc_interrupt = {
		.etrgs = 0,
		.ldrbs = 0,
		.ldras = 0,
		.cpcs  = 1, // Enable interrupt on RC compare alone
		.cpbs  = 0,
		.cpas  = 0,
		.lovrs = 0,
		.covfs = 0
	};
	// Initialize the timer/counter.
	tc_init_waveform(tc, &waveform_opt);

	/*
	 * Set the compare triggers.
	
	*RC calculation:
	The interrupt shall occur every 20 ms, and the time is set according to datasheet of AT32UC3A0512. 
	RC_calc = (((float)PBA_freq) *(float) time_tick )/divider * 1000.0
	Then casted from float to int value
	*/
	RC_calc = (((float)PBA_freq) *(float) time_tick )/8000.0;
	RC = (uint16_t) RC_calc;
	
	
	tc_write_rc(tc, EXAMPLE_TC_CHANNEL, RC);
	// configure the timer interrupt
	tc_configure_interrupts(tc, EXAMPLE_TC_CHANNEL, &tc_interrupt);
	// Start the timer/counter.
	tc_start(tc, EXAMPLE_TC_CHANNEL);
}


#endif /* PH_TC_INIT_H_ */