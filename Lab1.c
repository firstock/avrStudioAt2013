#include <avr/io.h>


int main(void)
{
	int state12=0;

	MCUCR = 0x00;
	DDRA = 0x0f;		//0: in, 1: out
	

	while(1)
	{
		if ((PINA & 0x10) && (state12 == 1))	// PA4 > switch's vcc	//no pushing
		{
			PORTA = 0x01;	// PA0: OFF (5V~diode~5V)
			state12= 0;
			for(int i=0; i<100000; i++);

		}
		if((PINA & 0x10) && (state12 == 0))
		{
			PORTA = 0x0;	// PA0: ON (0V~diode~5V)
			state12= 1;
			for(int i=0; i<100000; i++);
		}
	}
}
