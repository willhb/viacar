#include <stdio.h>
#include <lpc17xx/lpc17xx.h>
#include <lpc17xx/openlpc.h>
#include <lpc17xx/lpc17xx_ssp.h>

/*		ViaCar Main file
*/

void ext_adc_setup(void){
	LPC_SC->PCONP |= (1<<21);

	LPC_SC->PCLKSEL1 |= (1<<10);
	LPC_PINCON->PINSEL3 |= (3 << 10)|(3 << 8)|(3 << 14);
	LPC_PINCON->PINMODE3 |= (3<<8)|(3<<14)|(3<<16)|(3<<10);
	LPC_GPIO1->FIODIR |= 1 << 18 | 1 << 27;
	
	LPC_SSP0->CR0 |= 0xF|(1<<6);
	LPC_SSP0->CR1 |= (1<<1);
	LPC_SSP0->CPSR = 2;
}

void ui_setup(void){
	//SW1 = 4.28
	//SW2 = 0.21
	//LED4-7 = 4.29, 0.5, 0.7,0.9
	LPC_PINCON->PINMODE0 |= (1<<11)|(1<<15)|(1<<19); //pin 5,7,9 pullups off;
	LPC_GPIO0->FIODIR |= (1<<5)|(1<<7)|(1<<9); //5,7,9 are outputs
	LPC_GPIO4->FIODIR |= (1<<29);//4.29 is an output
	LPC_PINCON->PINMODE9 |= (1<<27);
	
	LPC_GPIOINT->IO0IntEnF |= (1<<21);
	NVIC_EnableIRQ(EINT3_IRQn);
	
}

void ui_set(int l1,int l2,int l3,int l4){
	if(l1 == 1){
		LPC_GPIO4->FIOSET |= (1<<29);
	} else {
		LPC_GPIO4->FIOCLR |= (1<<29);
	}
	if(l2 == 1){
		LPC_GPIO0->FIOSET |= (1<<5);
	} else {
		LPC_GPIO0->FIOCLR |= (1<<5);
	}
	if(l3 == 1){
		LPC_GPIO0->FIOSET |= (1<<7);
	} else {
		LPC_GPIO0->FIOCLR |= (1<<7);
	}
	if(l4 == 1){
		LPC_GPIO0->FIOSET |= (1<<9);
	} else {
		LPC_GPIO0->FIOCLR |= (1<<9);
	}
}

int servo_steer(float value){
	int servo_set = 113; //Servo centered on Tamiya Chasis
	
	if((value < -1.0) || (value > 1.0)){
		return -1; 	//Don't update and return an error.
	} else {
		//servo_set = (int)(226 * value); //Get an integer value from our turn percentage.
		servo_set = (int)(113 * value);
	} 
	
	//max = 452
	// min = 226
	
	LPC_PWM1->MR1 = 4310 + 339 + servo_set;	//Add to the minimum servo value.
	LPC_PWM1->LER |= (1<<1);			//Load enable Match register 4 (MR4), sets the new value. 
	return 0;	
}

void servo_setup(void){
	LPC_SC->PCONP |= 1<<6; 				//Turn on PWM
	LPC_SC->PCLKSEL0 |= (1<<12)|(1<<13);// PWM pclock/8
	LPC_PINCON->PINSEL4 |= (1<<0); 		//2.0 is PWM 1.1

	LPC_PWM1->PCR |= (1<<9);			//Turn on PWM 1.1 output
	LPC_PWM1->PR = 40;					//Prescale register value to get ~60Hz
	LPC_PWM1->MR0 = 5000;				//Match0 register
	LPC_PWM1->MR1 = 4580;				//Match4 register, opposite because of hardware inverter.
	LPC_PWM1->TCR |= (1<<0)|(1<<3); 	//Enable counter and PWM.
}

int motor_speed(float value){
	if(LPC_MCPWM->MCPW1 > 0){
	//	return -1;
	}
	if((value < 0.0) || (value > 1.0)){
		return -1;
	}	
	int motor_speed = 0;
	motor_speed = (int)(20000*value);
	LPC_MCPWM->MCPW1 = 0;
	LPC_MCPWM->MCPW2 = motor_speed;
	return 0;
}

void digipot_setup(void){
	LPC_SC->PCONP |= (1<<19)|(1<<26); //Make sure I2c1 and 2 are on
	
	LPC_SC->PCLKSEL1 |= (1<<6)|(1<<7); //I2C clock /8
	LPC_SC->PCLKSEL1 |= (1<<20)|(1<<21);// I2C2 clock is /8
	//0.10,0.11
	LPC_PINCON->PINMODE0 |= (1<<1)|(1<<3); //Turn off pull-ups and pull downs.
	LPC_PINCON->PINMODE0 |= (1<<21)|(1<<23);//Turn off pullups for 0.10, 0.11
	
	LPC_PINCON->PINMODE_OD0 |= (1<<0)|(1<<1); //P0.0 and P0.1 are Open Drain
	LPC_PINCON->PINMODE_OD0 |= (1<<10)|(1<<11); //p0.10,0.11 are Open Drain
	
	LPC_PINCON->PINSEL0 |= (1<<0)|(1<<1)|(1<<2)|(1<<3); //P0.0 and P0.1 are SDA1 and SCL1
	LPC_PINCON->PINSEL0 |= (1<<21)|(1<<23);//0.10=sda, 0.11=scl
	
	NVIC_EnableIRQ(I2C1_IRQn);
	NVIC_EnableIRQ(I2C2_IRQn);
	
	LPC_I2C1->I2SCLL = 60; //Set SCL Low and High duty cycle
	LPC_I2C1->I2SCLH = 60;
	
	LPC_I2C2->I2SCLL = 60; //Set SCL Low and High duty cycle
	LPC_I2C2->I2SCLH = 60;
	
	LPC_I2C1->I2CONSET = (1<<6); //Enable I2C1
	LPC_I2C2->I2CONSET = (1<<6); //Enable I2C
}

int motor_brake(float value){
	if(LPC_MCPWM->MCPW2 > 0){
	//	return -1;
	}
	if((value < 0.0) || (value > 1.0)){
		return -1;
	}	
	int motor_brake = 0;
	motor_brake = (int)(20000*value);
	LPC_MCPWM->MCPW2 = 0;
	LPC_MCPWM->MCPW1 = motor_brake;
	return 0;
}

int rgb_set(int r,int g,int b){
	LPC_PWM1->MR2 = g*19.61;
	LPC_PWM1->MR3 = b*19.61;
	LPC_PWM1->MR4 = r*19.61;
	LPC_PWM1->LER |= (1<<4)|(1<<3)|(1<<2);		//Load enable Match register 2/3/4, sets the new value. 
	return 0;	
}

void rgb_setup(void){
	LPC_SC->PCONP |= 1<<6; 						//Turn on PWM
	LPC_SC->PCLKSEL0 |= (1<<12)|(1<<13);		// PWM pclock/8
	LPC_PINCON->PINSEL4 |= (1<<2)|(1<<4)|(1<<6); //2.1/2/3 is PWM 1.2/3/4
	LPC_PWM1->PCR |= (1<<10)|(1<<11)|(1<<12);	//Turn on PWM 1.4 output
	LPC_PWM1->MR4 = 0;							//Set R,G,B to 0,0,0
	LPC_PWM1->MR3 = 0;
	LPC_PWM1->MR2 = 0;
}

void mux_setup(){
	// A = 0.23
	// B = 0.24
	// C = 0.25
	// INH=0.28 (inhibit) (active high)
	
	LPC_PINCON->PINMODE1 |= (1<<15)|(1<<17)|(1<<19); //Turn off pullups on 0.23/24/25
	LPC_GPIO0->FIODIR |= (1<<23)|(1<<24)|(1<<25)|(1<<28); //Set 0.23/24/25/28 as outputs
	LPC_GPIO0->FIOCLR |= (1<<23)|(1<<24)|(1<<25); //Set 0.23/24/25 to 0
	LPC_GPIO0->FIOSET |= (1<<28);
	
}

int mux_set(int state){
	
	if((state < 0)|(state >= 8)){
		return -1;
	} else {
		LPC_GPIO0->FIOSET = (state << 23); //Set bits that are 1
		LPC_GPIO0->FIOCLR = (~(state << 23))&(7<<23); //Clear the bits that were 0
		return 1; //sucess;
	}
	
	return 0; //nani??
}

void motor_setup(void){
	LPC_SC->PCONP |= 1<<17; 			//Turn on Motor Control PWM
	LPC_SC->PCLKSEL1 |= (1<<30)|(1<<31);// MC pclock/8
	
	LPC_PINCON->PINMODE0 |= (1<<9); //OTW pullup off.
	LPC_PINCON->PINMODE_OD0 |= (1<<4);
	LPC_PINCON->PINMODE1 |= (1<<21); //I_Sense pullup off.
	LPC_PINCON->PINMODE3 |= (1<<25)|(1<<19); //turn off pullups

	LPC_GPIO0->FIODIR |= (1<<6);
	LPC_GPIO0->FIOCLR |= (1<<6);
	
	//1.28 = speed, 1.25 = brake
	
	//MCOA1 = brake
	//MCOA2 = speed
	
	//0.6  = SD
	//0.4 = OTW
	//0.26 = i_sense
	
	LPC_MCPWM->MCPER2 = 20000;
	LPC_MCPWM->MCPW2 = 0;
	LPC_MCPWM->MCPER1 = 20000;
	LPC_MCPWM->MCPW1 = 0;
	
	LPC_MCPWM->MCDEADTIME = (1024)|(1024 << 10);
	
	LPC_MCPWM->MCCON_SET = (1<<16)|(1<<8)|(1<<18)|(1<<10);
	LPC_PINCON->PINSEL3 |= (1<<24)|(1<<18); //P1.19 is MCOA0 and P1.22 is MCOB0,1.25 = MC0A1
	
}

volatile int pause = 1;
volatile int debounce = 0;
volatile int catch = 1;

void EINT3_IRQHandler(void)
{
	LPC_GPIOINT->IO0IntClr |= (1<<21);
	if(debounce == 0){
	pause ^= 1;
	debounce = 1;
	catch = 0;
	}
}

#define	ADDR_P1  0b01010000; //write address.
#define	ADDR_P2  0b01010010; //write address.
#define	GEN_CALL  0;	      //supports general call addressing
#define W0	 0b00000000; //write to all Wiper 0s
#define	W1	 0b00010000; //write to all Wiper 1s
int state = 0;
int i2c_resistor = 0;
int i2c_address = 0;
int i2c_wiper = 0;

int set_pot(int r, int address, int wiper){
	
	i2c_resistor = r; //get the desired resistance
	if(wiper == 0){
		i2c_wiper = W0;//wiper 1 or 2?
	} else {
		i2c_wiper = W1;
	}
	if(address == 0){
		i2c_address = ADDR_P1;//wiper 1 or 2?
	} else {
		i2c_address = ADDR_P2;
	}
	
	printf("Sending Start Bit\n\r");
	state = 0;
	LPC_I2C2->I2CONSET = (1<<5);
	
	return 0;
}

int main()
{
	// initialize the LED pin as an output
	LPC_GPIO1->FIODIR = 1 << 18 | 1 << 27;
	LPC_GPIO0->FIODIR = 1 << 4 | 1 << 5 | 1 << 6;
	rgb_setup();
	rgb_set(0x00,0xFF,00);
	volatile float derivative = 0.0;
	volatile float prev_error = 0.0;
	volatile int difference = 0;
	volatile float steer = 0.0;
	volatile float integral = 0.0;
	volatile float boost = 0.0;
	motor_setup();
	mux_setup();
	ui_setup();
	servo_setup();	 //Initialize servo
	delay_ms(100);
	ui_set(1,0,0,0);
	delay_ms(100);
	ui_set(1,1,0,0);
	delay_ms(100);
	ui_set(1,1,1,0);
	delay_ms(100);
	ui_set(1,1,1,1);
	delay_ms(100);
	ui_set(0,0,0,0);

	servo_steer(1); //Steer to center	
	//delay_ms(100);
	servo_steer(0);
	//delay_ms(100);
	servo_steer(.51);
	/* Mux Configuation Reference
	// 0 = 5V
	// 1 = 2.5V Reference
	// 2 = Ground
	// 3 = NC
	// 4 = CH4
	// 5 = CH1
	// 6 = CH3
	// 7 = CH2
	*/
	
	ext_adc_setup();
	rgb_set(0xFF,0xFF,0);
	
	servo_steer(0);
	int test_count = 0;
	int channel = 1;
	int average = 0;
	digipot_setup();

	
	mux_set(channel); //get 2.5V reference
	
	//R, address, wiper
	set_pot(150, 0, 0);
	delay_ms(5);
	set_pot(75, 0, 1);
	delay_ms(5);
	set_pot(100, 1, 0);
	delay_ms(5);
	set_pot(80, 1, 1);
	
	int b_rising = 0;
	int menu_count = 0;
	
	while(catch){
		if((LPC_GPIO4->FIOPIN & 1<<28)!= 0){
			rgb_set(0xFF,0xFF,0);
			if(b_rising == 0){
				b_rising = 1;
				menu_count++;
				if(menu_count >= 16) menu_count = 0;
			}
		} else {
			b_rising = 0;
			ui_set((menu_count>>0)%2,(menu_count>>1)%2,(menu_count>>2)%2,(menu_count>>3)%2);
			rgb_set(0,0,0xFF);
		}
	}
	
	rgb_set(0xFF,0,0);
	if(average == 0){
		for(int i = 0; i < 50 ; i++) {
			LPC_SSP0->DR = 0;
			while ( (LPC_SSP0->SR & ((SSP_SR_BSY)|SSP_SR_RNE)) != SSP_SR_RNE );
			average = ((LPC_SSP0->DR >> 2) + average)/2;			
		}
		printf("Average: %d \n\r", average);
	}
	
	volatile int waveform[200];
	volatile int left, right, center;
	channel = 5;
	mux_set(channel);
	
	while(1){

	
	for(int i = 0; i < 15 ; i++) {
		LPC_SSP0->DR = 0;
		while ( (LPC_SSP0->SR & ((SSP_SR_BSY)|SSP_SR_RNE)) != SSP_SR_RNE );
		waveform[i] = (LPC_SSP0->DR >> 2);			
	}
	
	int peak = 0;
	
	for(int i = 0; i < 15 ; i++) {
		if(waveform[i] > peak){
			peak = waveform[i];
		}
	}
	
	//printf("Peak for channel %d: %d \n\r", channel, peak);
	switch(channel){
		case 5:
		channel = 7;
		left = peak;
		break;
		case 7:
		channel = 6;
		right = peak;
		break;
		case 6:
		channel = 5;
		center = peak;
		break;

	}
	
	//printf("Right: %d, Left: %d, Center: %d \n\r", right, left, center);
	mux_set(channel);
	
	
	difference = right-left;
	steer = (float)difference/2000;
	
	if((steer > 0.5) & (steer < -0.5)){
		integral = integral + steer;
	}
	if((steer < 0.1) & (steer > -0.1)){
		integral = 0;
	} else {
		if(boost > 0.0){
			boost -= 0.001;
		} else {
			boost = 0;
		}
		
	}
	
	if(center > 2500){
	boost += .00001;
	}
	
	if(menu_count > 1){
	motor_speed((float)menu_count*0.01 + .05);
	}
	
	
	derivative = (steer - prev_error)/2;
	if(center > 2500){
	servo_steer(steer*2.8 + derivative*0.55 + integral*0.002);	
	//servo_steer(steer*2 + derivative*0.3 + integral*0.001);	
	//servo_steer(steer*.95 + derivative*0.45 + integral*0.001);
	prev_error = steer;
	
	}
	
	
	}
	

	
	//203 feet
	return 0;
}

/*
int	addr_p1 = 0b01010000; //write address.
int	addr_p2 = 0b01010010; //write address.
int	gen_call= 0;	      //supports general call addressing
int gen_w0	= 0b10000000; //write to all Wiper 0s
int gen_w1	= 0b10010000; //write to all Wiper 1s
*/

void I2C2_IRQHandler(void)
{
	int I2C_Status = LPC_I2C2->I2STAT;
	printf("Status: 0x%x", I2C_Status);
	switch(I2C_Status){
		case 0x8:
			LPC_I2C2->I2DAT = i2c_address;
			LPC_I2C2->I2CONSET = 0x04;
			LPC_I2C2->I2CONCLR = (1<<3)|(1<<5);	
			printf(": Done \n\r");
			break;
		case 0x18:
			LPC_I2C2->I2DAT = i2c_wiper;
			LPC_I2C2->I2CONSET = 0x04;
			LPC_I2C2->I2CONCLR = 0x08;
			printf(":Wrote: %x Ack Done \n\r", (int)LPC_I2C2->I2DAT);
			break;
		case 0x20:
			LPC_I2C2->I2CONSET = 0x14;
			LPC_I2C2->I2CONCLR = 0x08;
			printf(": Done \n\r");
			break;
		case 0x28:
		if(state == 0){
			LPC_I2C2->I2DAT = i2c_resistor;
			LPC_I2C2->I2CONSET = 0x04;
			LPC_I2C2->I2CONCLR = 0x08;
			printf(":Wrote: %x Ack Done \n\r", (int)LPC_I2C2->I2DAT);
			state = 1;
		} else {
			LPC_I2C2->I2CONSET = 0x14;
			LPC_I2C2->I2CONCLR = 0x08;
			printf(":Stop and Done \n\r");
						state = 0;
		}
			break;
		case 0x10:
			LPC_I2C2->I2DAT = 0x90;
			LPC_I2C2->I2CONSET = 0x04;
			LPC_I2C2->I2CONCLR = 0x08;
			LPC_I2C2->I2CONCLR = (1<<3);
			printf(": Done \n\r");
			break;
		case 0x48:
			LPC_I2C2->I2CONSET = 0x04;
			LPC_I2C2->I2CONCLR = 0x08;
			printf(": Done \n\r");
			break;
		case 0x38:
			LPC_I2C2->I2CONSET = 0x24;
			LPC_I2C2->I2CONCLR = 0x08;
			printf(": Done \n\r");
			break;
		case 0x40:
			LPC_I2C2->I2CONSET = (1<<2);
			LPC_I2C2->I2CONCLR = 0x08;
			printf(": Done \n\r");
			break;
		case 0x50:
			if(state == 0){
				LPC_I2C2->I2CONSET = 0x04;
				LPC_I2C2->I2CONCLR = 0x08;
				printf(":Read: %d  Done \n\r", (int)(LPC_I2C2->I2DAT<<4)/16);
				state = 1;
			} else {
				LPC_I2C2->I2CONCLR = 0x0C;
				printf(":Read: %d  Done \n\r", (int)(LPC_I2C2->I2DAT)*5/128);
				state = 0;
			}
		break;
		case 0x58:
			printf(":Read: %x  Done \n\r", (int)LPC_I2C2->I2DAT);	
			LPC_I2C2->I2CONSET = 0x14;
			LPC_I2C2->I2CONCLR = 0x08;	
			break;
		default:
			printf("Something went wrong...");
			delay_ms(1000);
			break;
		printf("\n\r");	
	}
	

}



