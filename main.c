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

#define IS_MOTOR_RUNNING_UP   ((MOTOR_OUT & MOTOR_UP) == 0)
#define IS_MOTOR_RUNNING_DOWN ((MOTOR_OUT & MOTOR_DOWN) == 0)

#define SECONDS(x) (200 * (x))

unsigned duration;
unsigned ignore_button = 0;

// Delay Routine from mspgcc help file
static void __inline__ delay(register unsigned int n)
{
  __asm__ __volatile__ (
  "1: \n"
  " dec %[n] \n"
  " jne 1b \n"
        : [n] "+r"(n));
}


#define DELAY_5ms 1650

char is_hit(unsigned char pin, unsigned char first, unsigned char second)
{
	return ((first & pin) == 0) && ((second & pin) == 0);
}

void start(unsigned char dir)
{
	MOTOR_OUT &= ~dir;
	MOTOR_OUT |= LED;
	duration = 0;
	ignore_button = SECONDS(2);
}

void stop()
{
        // don't let start button work for a while
	ignore_button = SECONDS(5);
	MOTOR_OFF;
}

int main(void) {
	unsigned char first;
	unsigned char second;

	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT
 
	BCSCTL1 = CALBC1_1MHZ;                    // Set range
	DCOCTL = CALDCO_1MHZ;

	MOTOR_DIR |= MOTOR_UP + MOTOR_DOWN + LED;  //Set LED pins as outputs
	MOTOR_OUT  = MOTOR_UP + MOTOR_DOWN;	     //Turn on both LEDs

  while(1) {
	first = P1IN;
	delay(DELAY_5ms);
	second = P1IN;
	
	if (IS_MOTOR_RUNNING_UP && is_hit(SW_TOP, first, second))
	{
		stop();
	}

	if (IS_MOTOR_RUNNING_DOWN && is_hit(SW_BOTTOM, first, second))
	{
		stop();
	}
	
	if (IS_MOTOR_RUNNING && duration++ > SECONDS(20))
	{
		stop();
	}

	if (ignore_button > 0)
		ignore_button--;

	if (ignore_button == 0)
	{
		if (is_hit(SW_START, first, second))
		{
			if (IS_MOTOR_RUNNING)
				stop();
			else if (SW_BOTTOM_HIT)
				start(MOTOR_UP);
			else
				start(MOTOR_DOWN);
		}
	}	
  }
}

