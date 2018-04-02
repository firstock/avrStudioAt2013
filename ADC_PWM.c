//오후 6:19 2013-11-22

// int overflow(0~65535) 때문에 0~1023 > 0~5 로 맵핑하는 과정에서 소수점 이하 두번째 자리를 표현할 방법이 애매함
// #은 for(0~1의 자리숫자) display("#"); 하면 해결될것.

#include <avr/io.h>
#include <string.h>
#define LCD_EN (*(volatile unsigned char *)0x8000)
#define LCD_WRITE (*(volatile unsigned char *)0x8002)
#define LINE1 0x00 // LCD_DISPLAY_LINE1
#define LINE2 0x40 // LCD_DISPLAY_LINE2

void delay_us(unsigned char);
void delay_ms(unsigned int);

void FunctionSet();
void DisplayOnOffControl();
void DisplayClear();
void EntryModeSet();
void DisplayChar(unsigned char);
void DisplayString(unsigned char *);
void SetLCDAddress(unsigned char);

unsigned char ADC_L = 0x00; 
unsigned char ADC_H = 0x00; 
unsigned int ADC_RESULT = 0x00; // 숫자 +48 -> ASCII
unsigned char c1,c2,c3,c4; 		// ASCII
int i=0;

//unsigned int VCC_05= 0x00;	// maybe 2byte> 0~65535
unsigned char v1,v2,v3;

int main(void)
{
	MCUCR = 0x80; 
	XMCRA = 0X40;
	XMCRB = 0X80;
	DDRD = 0xff;
	DDRB = 0xff;
	PORTD = 0xff;

	FunctionSet();
	DisplayOnOffControl();
	DisplayClear();
	EntryModeSet();

	ADCH = 0x00; 
	ADCL = 0x00; 
	ADMUX = 0x00; 
	
	TCCR0 = 0x7c; /* Timer0 Fast PWM clk/64 */
	while(1)
	{
		ADCSR = 0xc7; 
		while((ADCSR & 0x10)==0);
		ADC_L = ADCL; 
		ADC_H = ADCH; 
		PORTD = ~(ADC_L << 4);
		//PORTD = ~PORTD;
		OCR0=(ADC_L>>2) | (ADC_H<<6);
		
		ADC_RESULT = 0x00;
		ADC_RESULT = ADC_RESULT | ADC_H;
		ADC_RESULT = 0x0300 & (ADC_RESULT << 8);
		ADC_RESULT = ADC_RESULT | ADC_L;

		c4 = ADC_RESULT / 1000;
		c3 = (ADC_RESULT % 1000) / 100;
		c1 = ADC_RESULT % 100;
		c2 = c1/10;
		c1 = c1 % 10;

		//test
		//VCC_05 = (ADC_RESULT * 500 )/ 1023;		//0~500 밑에 자리수는 버림

		v3 = ADC_RESULT*5/1023;
		v2 = (ADC_RESULT*50/1023) % 10;	// 이것도 *5/102.3 ?

		v1 = ADC_RESULT *50/102.3 % 10;	// *500= 511500 -> overflow? (> 65535: int ?)	// 그냥 v2/10은 안되었던걸로 기억
										// 해결책: OV 안 나게 '잘' 소수점 둘째 자리수 만들기.
										// 1)꼭 /1023 할것은X 2) 우선순위 이용- 안 잘려나가는지 3) 잠시 형변환.
		
		//
		SetLCDAddress(LINE1);
		DisplayString((unsigned char*)"ADC:");
		DisplayChar(c4+0x30);		//ex. 1		//0x30 > 48 > ASCII_0		//문제> 계속 0 나옴.
		DisplayChar(c3+0x30);		//0
		DisplayChar(c2+0x30);		//2
		DisplayChar(c1+0x30);		//3
		delay_ms(5);


		SetLCDAddress(LINE2);				//test 
		//DisplayString((unsigned char*)"ADC:");	//test
											// LINE2 첫글자에 알 수없는 0이 나옴.
		DisplayChar(v3+0x30);				// ex 5
		DisplayString((unsigned char*)".");	// .
		DisplayChar(v2+0x30);				// 0	< 안나옴.
		DisplayChar(v1+0x30);				// 0
		
		for(i=0; i< v3; i++)
			DisplayString((unsigned char*)"#");	// 1의자리 Voltage만큼 # 출력.

		//DisplayChar(v1+0x30);				// test
		delay_ms(5);

	}
}


// --------------- 이하: 기타 셋팅 --------------- //
void DisplayChar(unsigned char c)
{
	LCD_WRITE = c;
	delay_us(50);
}
void DisplayString(unsigned char* s)
{
	unsigned char i, j;
	i = 0;
	j = strlen((char *)s)-1;
	for(i=0;i<=j;i++)
	{
		DisplayChar(s[i]);
	}
}
void SetLCDAddress(unsigned char mode)
{
	LCD_EN = (0x80 | mode);
	delay_us(50);
}
void FunctionSet()
{
	delay_ms(50);
	LCD_EN = 0x38;
}
void DisplayOnOffControl()
{
	delay_us(40);
	LCD_EN = 0x0c;
}
void DisplayClear()
{
	delay_us(40);
	LCD_EN = 0x01;
}
void EntryModeSet()
{
	delay_ms(2);
	LCD_EN = 0x06;
	delay_us(40);
}
void delay_us(unsigned char time_us)
{
	register unsigned char i;
	for(i=0;i<time_us;i++) //4 cycle
	{
		asm volatile("PUSH R0"); //2 cycle
		asm volatile("POP R0"); //2 cycle
		asm volatile("PUSH R0"); //2 cycle
		asm volatile("POP R0"); //2 cycle
		asm volatile("PUSH R0"); //2 cycle
		asm volatile("POP R0"); //2 cycle
		// Sum = 16 cycle=1 us for 16MHz
	}
}
void delay_ms(unsigned int time_ms)
{
	register unsigned int i;
	for(i=0;i<time_ms;i++) //4 cycle
	{
		delay_us(250);
		delay_us(250);
		delay_us(250);
		delay_us(250);
	}
}
