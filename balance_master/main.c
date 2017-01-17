/*
 * main.c
 */
#pragma once
#include "project_header.h"

void init_quadrature();
void init_pwm(void);

int main(void)
{
	init_pwm();
	init_quadrature();

	return 0;


}


volatile int qeiPosition;
volatile int qeiDirection;


void init_pwm(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);

	SysCtlPWMClockSet(SYSCTL_PWMDIV_16);

	HWREG(GPIO_PORTF_BASE+GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE+GPIO_O_CR) |= 0x01;

	GPIOPinConfigure(GPIO_PF0_M1PWM4);

	GPIOPinTypePWM(GPIO_PORTF_BASE,GPIO_PIN_0);

	PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2,2000);

	PWMPulseWidthSet(PWM1_BASE,PWM_OUT_4,1000);

	PWMGenEnable(PWM1_BASE, PWM_GEN_2);

	PWMOutputState(PWM1_BASE,(PWM_OUT_4_BIT),true);

}



void init_quadrature(void)
{
	//Set the clock source for the Run Gate Clock for the QEI Module
	SysCtlClockSet(SYSCTL_SYSDIV_1|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	// Enable QEI Peripherals
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI0);

	//Unlock GPIOD7 - Like PF0 its used for NMI - Without this step it doesn't work
	HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY; //Right to the GPIO register for Port D
	HWREG(GPIO_PORTD_BASE + GPIO_O_CR) |= 0x80;
	HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = 0;

	//Set Pins to be PHA0 and PHB0
	GPIOPinConfigure(GPIO_PD6_PHA0);
	GPIOPinConfigure(GPIO_PD7_PHB0);

	//Set GPIO pins for QEI. PhA0 -> PD6, PhB0 ->PD7. I believe this sets the pull up and makes them inputs
	GPIOPinTypeQEI(GPIO_PORTD_BASE, GPIO_PIN_6 |  GPIO_PIN_7);

	//DISable peripheral and int before configuration
	QEIDisable(QEI0_BASE);
	QEIIntDisable(QEI0_BASE,QEI_INTERROR | QEI_INTTIMER | QEI_INTINDEX);

	// Configure quadrature encoder, use an arbitrary top limit of 1000
	QEIConfigure(QEI0_BASE, (QEI_CONFIG_CAPTURE_A_B  | QEI_CONFIG_NO_RESET 	| QEI_CONFIG_QUADRATURE | QEI_CONFIG_NO_SWAP), 2000);

	//SETUP filter to debounce the steps
	QEIFilterConfigure(QEI0_BASE, QEI_FILTCNT_17);

	//enable the filter
	QEIFilterEnable(QEI0_BASE);

	// Enable the quadrature encoder.
	QEIEnable(QEI0_BASE);

	//Set position to a middle value so we can see if things are working
	QEIPositionSet(QEI0_BASE, 1000);

	//Add qeiPosition as a watch expression to see the value inc/dec
	while (1) //This is the main loop of the program
		{
			qeiPosition = QEIPositionGet(QEI0_BASE);
			qeiDirection = QEIDirectionGet(QEI0_BASE);


			PWMPulseWidthSet(PWM1_BASE,PWM_OUT_4,qeiPosition);

			SysCtlDelay (1000);
		}

}
