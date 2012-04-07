#include <msp430.h>
#include <legacymsp430.h>

#define     LED                     BIT7

#define     MOTOR_UP                BIT0	//Red LED
#define     MOTOR_DOWN              BIT6	//Green LED
#define     MOTOR_DIR               P1DIR
#define     MOTOR_OUT               P1OUT
#define     MOTOR_OFF               P1OUT = MOTOR_UP | MOTOR_DOWN | LED

#define     SW_TOP     BIT4
#define     SW_BOTTOM  BIT5
#define     SW_START   BIT3

#define SW_TOP_HIT    ((P1IN & SW_TOP) == 0)
#define SW_BOTTOM_HIT ((P1IN & SW_BOTTOM) == 0)
#define SW_START_HIT  ((P1IN & SW_START) == 0)

#define IS_MOTOR_RUNNING ((MOTOR_OUT & (MOTOR_UP | MOTOR_DOWN)) != (MOTOR_UP | MOTOR_DOWN))

#define SECONDS(x) (x)

unsigned duration;
char ignore_button = 0;

int main(void) {

  WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

  MOTOR_DIR |= MOTOR_UP + MOTOR_DOWN + LED;  //Set LED pins as outputs
  MOTOR_OUT  = MOTOR_UP + MOTOR_DOWN;	     //Turn on both LEDs

  P1IES |= SW_TOP | SW_BOTTOM | SW_START;
  P1IE  |= SW_TOP | SW_BOTTOM | SW_START;

  BCSCTL3 |= LFXT1S_2;	//Set ACLK to use internal VLO (12 kHz clock)
			
  eint();	//Enable global interrupts

  while(1) {
	//Loop forever, interrupts take care of the rest
	__bis_SR_register(CPUOFF + GIE);  // sleep waiting for a character or timer wakeup
  }
}

void start(unsigned char dir)
{
	MOTOR_OUT &= ~dir;
	MOTOR_OUT |= LED;
	duration = 0;
	ignore_button = SECONDS(2);

	TACTL = TASSEL_1 | MC_1;	//Set TimerA to use auxiliary clock in UP mode
	TACCTL0 = CCIE;	//Enable the interrupt for TACCR0 match
	TACCR0 = 10000;	/*Set TACCR0 which also starts the timer. At */
}

void stop()
{
        // don't let start button work for a while
	ignore_button = SECONDS(5);

	MOTOR_OFF;
}

interrupt(TIMERA0_VECTOR) TIMERA0_ISR(void)
{
	duration++;

	if (ignore_button > 0)
		ignore_button--;

	if (IS_MOTOR_RUNNING)
	{
		if (SW_BOTTOM_HIT || SW_TOP_HIT || duration > SECONDS(30))
		{
			stop();
		}
	}
	else
	{
		if (ignore_button == 0)
			TACTL = TASSEL_1; // timer off	
	}
}

interrupt(PORT1_VECTOR) Port_1(void)
{ 
	if (P1IFG & SW_START)
	{
		if (ignore_button == 0)
		{
			if (IS_MOTOR_RUNNING)
				stop();
			else if (SW_BOTTOM_HIT)
				start(MOTOR_UP);
			else
				start(MOTOR_DOWN);
		}
	}

	if (P1IFG & SW_BOTTOM || P1IFG & SW_TOP)
	{
		stop();
	}
	P1IFG = 0;
}

