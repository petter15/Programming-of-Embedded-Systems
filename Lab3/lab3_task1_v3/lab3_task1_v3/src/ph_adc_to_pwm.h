/*
 * ph_adc_to_pwm.h
 *
 * Created: 12.03.2016 19:40:51
 *  Author: Petter
 */ 


#ifndef PH_ADC_TO_PWM_H_
#define PH_ADC_TO_PWM_H_

uint32_t adc_2_pwm (uint32_t adc_input, uint32_t pba_clk, uint16_t freq_or_duty);

float period_temp;
float cycle_glob_val=50;

uint32_t adc_2_pwm (uint32_t adc_input, uint32_t pba_clk, uint16_t freq_or_duty)
{
	//print_dbg("This is adc2pwm function\n");
		uint32_t adc_temp=0;
		uint32_t period=0;
		uint32_t cycle_temp=0;
		uint32_t cycle_cast_temp; 
		uint32_t cycle=0;
		uint32_t adc_cast =0; 
		
		char string_adc_pwm[9];
		char string_period_pwm[9];
		char string_freq_pwm[9];
		char string_cycle_pwm[9];
		char temp_string_pwm[9];
		
		
		
		
		switch(freq_or_duty)
			{
			 		case 0:
						//print_dbg("This is case 0 in adc2pwm\n");
			 			/*Frequency calculation:
			 			Set frequency= ADC reading*100 (1*100= 100 Hz, 1000*100 = 100kHz)
			 			*/
			 			if (adc_input > 1000)
			 			{
							 adc_temp = 100000;
			 			} 
						 else
			 			{
							adc_temp = (adc_input*100);	 
			 			}
						 
				
						dip204_set_cursor_position(12,1);
						dip204_write_string ("      ");
						dip204_set_cursor_position(12,1);
						sprintf (string_freq_pwm, "%lu", adc_temp);
						dip204_write_string (string_freq_pwm);
						dip204_set_cursor_position(18,1);
						dip204_write_string ("hz");
				
			
						//Calculate period to get according frequency output at the pwm output
						// 	//(MCK/prescaler)/period
						// 	periodic value=((float)PBA_freq/(float)set frequency);
 						period_temp = ((float)pba_clk) / ((float)adc_temp);
 						period = (uint32_t) period_temp;
					
						dip204_set_cursor_position(1,4);
						dip204_write_string ("Period: ");
						dip204_set_cursor_position(9,4);
						dip204_write_string ("      ");
						dip204_set_cursor_position(9,4);
						sprintf (string_period_pwm, "%lu", period);
						dip204_write_string (string_period_pwm);
						
						//Calculate duty cycle according to period and wanted duty cycled percent
						cycle_temp = ( period_temp / ( 100.0 / (100.0 - cycle_glob_val ) ));
					
						cycle = (uint32_t) cycle_temp;
						cycle_cast_temp = ((uint32_t)cycle_glob_val);
						
						dip204_set_cursor_position(16,4);
						dip204_write_string ("     ");
						dip204_set_cursor_position(16,4);
						sprintf (temp_string_pwm, "%lu", cycle_temp);
						dip204_write_string (temp_string_pwm);
					
						dip204_set_cursor_position(12,2);
						dip204_write_string ("    ");
						dip204_set_cursor_position(12,2);
						sprintf (string_cycle_pwm, "%lu", cycle_cast_temp);
						dip204_write_string (string_cycle_pwm);
					
						pwm_channel.cdty = cycle;	/* Channel duty cycle, should be < CPRD. */
						pwm_channel.cprd = period;	/* Channel period. */
				 	
						 break;
			
			 		case 1:
						// print_dbg("This is case 1 in adc2pwm\n");
			 			/* Duty cycle calculation:
				 		Duty cycle according to adc input
				 		Value between
				 		*/
					 
				 		//adc_temp = ( ((float) adc_input)/11.0);
						//adc_cast = ( (uint32_t) adc_temp);
						
						
						//adc_temp = ( ((float) adc_input)/10.0);
						if (adc_input > 1000)
						{
							adc_temp = 100.0;
						} 
						
						
						else
						{
							adc_temp = ( ((float) adc_input)/10.0);
						}
						 
						adc_cast = ( (uint32_t) adc_temp);
						 
						 dip204_set_cursor_position(12,4);
						 dip204_write_string ("  ");
						 dip204_set_cursor_position(12,4);
						 sprintf (string_adc_pwm, "%lu", cycle);
						 dip204_write_string (string_adc_pwm);
					 
					 
						 /*
						//Calculate period to get according frequency output at the pwm output
						// 	//(MCK/prescaler)/period
						// 	periodic value=((float)PBA_freq/(float)set frequency);
						temp = ((float)pba_clk) / ((float)freq);
						period = (uint32_t) temp;
						*/
				
					
						//Calculate duty cycle according to period and wanted duty cycled percent
					
						cycle_temp = ( period_temp / ( 100.0 / (100.0 - (adc_temp)) ));
						cycle = (uint32_t) cycle_temp;
						cycle_glob_val = adc_temp;
					
						 dip204_set_cursor_position(12,2);
						 dip204_write_string ("         ");
						 dip204_set_cursor_position(12,2);
						 sprintf (string_cycle_pwm, "%lu", adc_temp);
						dip204_write_string (string_cycle_pwm);
						dip204_set_cursor_position(16,2);
						dip204_write_string ("%");
					
						pwm_channel.cdty = cycle;	/* Channel duty cycle, should be < CPRD. */
					
				 		break;
				 		
					 default:
				 		break;
						 
		 	}	
	//Return ADC reading from case 1 to provide DT value for case 0
	return cycle_glob_val;
}


#endif /* PH_ADC_TO_PWM_H_ */