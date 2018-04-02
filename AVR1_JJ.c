#include <avr/io.h>
void delay_us(unsigned char time_us)
{
	register unsigned char i;	//ready for using R0
	for(i=0;i<time_us;i++)		//4 cycle
	{
		asm volatile("PUSH R0");//2 cycle
		asm volatile("POP R0"); //2 cycle
		asm volatile("PUSH R0");//2 cycle
		asm volatile("POP R0"); //2 cycle
		asm volatile("PUSH R0");//2 cycle
		asm volatile("POP R0"); //2 cycle
		// Sum = 16 cycle=1 us for 16MHz
	}
}
void delay_ms(unsigned int time_ms)
{
	register unsigned int i;
	for(i=0;i<time_ms;i++)		//4 cycle
	{
		delay_us(250);
		delay_us(250);
		delay_us(250);
		delay_us(250);
	}
}

int main(void)
{
	MCUCR = 0x00;
	DDRA = 0x0f;	//PA[7,6,5,4]: 0_IN, PA[3,2,1,0]: 1_OUT

	int state=0;

	while(1)
	{
		if ( ((PINA & 0x10) != 0x10) == (state == 1) )
		// Switch[0001 0000][765'4' 3210]
		// (push)PA4==0 ( '4'bit != 1) && N pushing
		{
			PORTA = 0x0f; // PA[3,2,1,0]_LED OFF
			state = 0;
			delay_ms(120);
		}
		else if ( ((PINA & 0x10) != 0x10) == (state == 0) )
		// (push)PA4==0 ( '4'bit != 1) && N+1 pushing
		{
			PORTA = 0x0; // PA[7~0]_LED ON
			state = 1;
			delay_ms(120);
		}
	}
}

