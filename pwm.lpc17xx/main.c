#include <stdio.h>
#include <lpc17xx/lpc17xx.h>
#include <lpc17xx/openlpc.h>

/*		LPC1769 Servo Demo
*		William Breidenthal 3/2012
*
*		This is a servo demo that has been customized to use the HiTec HS-9605MG servo with
*		the custom servo power/buffer board. If a different board or servo combo is used it
*		will be necessary to make adjustments. The current resolution exceeds the servo's,
*		it is not necessary to increase it any more.												
*
*		servo_setup() will setup a 58Hz PWM output on pin 2.3 (PWM1.4) with the servo centered.
*		servo_steer() takes a float in the range 0.0->1.0 and returns 0 with success, and -1 on failure. 
*/

int servo_steer(float value){
	int servo_set = 540;
	
	if((value < 0.0) || (value > 1.0)){
		return -1; 	//Don't update and return an error.
	} else {
		servo_set = (int)(540 * value); //Get an integer value from our turn percentage.
	} 
	
	LPC_PWM1->MR4 = 4310 + servo_set;	//Add to the minimum servo value.
	LPC_PWM1->LER |= (1<<4);			//Load enable Match register 4 (MR4), sets the new value. 
	return 0;	
}

void servo_setup(void){
	LPC_SC->PCONP |= 1<<6; 				//Turn on PWM
	LPC_SC->PCLKSEL0 |= (1<<12)|(1<<13);// PWM pclock/8
	LPC_PINCON->PINSEL4 |= (1<<6); 		//2.3 is PWM 1.4

	LPC_PWM1->PCR |= (1<<12);			//Turn on PWM 1.4 output
	LPC_PWM1->PR = 40;					//Prescale register value to get ~60Hz
	LPC_PWM1->MR0 = 5000;				//Match0 register
	LPC_PWM1->MR4 = 4580;				//Match4 register, opposite because of hardware inverter.
	LPC_PWM1->TCR |= (1<<0)|(1<<3); 	//Enable counter and PWM.
}

int main()
{
	// initialize the LED pin as an output
	LPC_GPIO1->FIODIR = 1 << 18 | 1 << 27;
	LPC_GPIO0->FIODIR = 1 << 4 | 1 << 5 | 1 << 6;
	
	servo_setup();	 //Initialize servo
	servo_steer(.5); //Steer to center	

	/*LPC1769 Servo Demo*/

	int polarity = 0;
	float a = 0;
	
	while(1){
		servo_steer(0.0);
		delay_ms(500);
		servo_steer(.333);
		delay_ms(500);
	}
	
	while(1){
		if(polarity == 0){
			a += .005;
		}
		if(polarity == 1){
			a -= .005;
		}
		if(a > 1.0){
			polarity = 1;
			delay_ms(1000);
		}
		if(a < 0.0){
			polarity = 0;
			delay_ms(1000);
		}
		servo_steer(a);
		delay_ms(2);
	}

	return 0;
}

