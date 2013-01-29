ViaCar12
========

ViaCar code and hardware files for my 2012 competition car.

See http://ieee.ucsd.edu/viacar/ for a description of what ViaCar is.

The code is dependent on the OpenLPC library for some functions (openlpc.com). 

My final code could run at 7+ft/s on a complex track. It is written in C for the LPC1769 and includes a PID control loop for steering, i2c state machine for digital potentiometers, UART debugging, brushed DC motor PWM control, and some comments. 

The majority of my time was spent on the hardware which is in this repository. ViaCar Main.brd is the main digital breakout and analog filtering PCB. Motor.brd is the motor controller board, it includes some power supply filtering, a half-H Bridge for the DC motor, and current sensing. ViaCar Sense V2.brd is the sensor breakout, it has a basic bandpass filter, on board LDO, and amplification. The specified filter values put the pass frequency at around 77kHz which is decently close to 75kHz as none of the filters have a very high Q, capacitors can be easily modified to shift this closer to 75kHz. 

Feel free to contact me with any questions.
