// LAB 8. EX2

#include <avr/io.h>
#include <string.h>

// LCD 모듈로 접근하기 위한 외부 메모리 주소 선언
#define LCD_EN (*(volatile unsigned char *)0x8000)
#define LCD_WRITE (*(volatile unsigned char *)0x8002)

// LCD 모듈의 첫번째 라인, 두번째 라인 주소
#define LINE1 0x00 // LCD_DISPLAY_LINE1
//#define LINE2 0x40 // LCD_DISPLAY_LINE2

// 지연
void delay_us(unsigned char);
void delay_ms(unsigned int);

// LCD 초기화 관련
void FunctionSet();
void DisplayOnOffControl();
void DisplayClear();
void EntryModeSet();

// LCD 글자표시 (문자 및 문자열)
void DisplayChar(unsigned char);
void DisplayString(unsigned char *);
void SetLCDAddress(unsigned char);

int main(void)
{
	MCUCR= 0x00;	// ?? what function ??
	DDRG= 0X00;	//PG3: 0_IN


	int i;
	// LCD에 표시할 문자열
	// 2nd array- table. >> better? << because.. this: 108 byte 
	char s[12][9] = {"Hello Wo",
				"ello Wor",
				"llo Worl",
				"lo World",
				"o World ",
				" World H",
				"World He",
				"orld Hel",
				"rld Hell",
				"ld Hello",
				"d Hello ",
				" Hello W" };

	// SFR 초기화
	MCUCR = 0x80; // I/O 포트를 특수기능 모드로 설정
	XMCRA = 0X40; // 외부 메모리 섹터 설정
	XMCRB = 0X80; // 외부 메모리 섹터 설정
	// LCD 초기화
	FunctionSet();
	DisplayOnOffControl();
	DisplayClear();
	EntryModeSet();

	while(1)
	{
		for(i=0; i<12; i++)	
		{
			if (((PING & 0x08) != 0x08))//1st push> stop
			{
				delay_ms(120);			//~ chattering
				while(PING & 0x08){}	//2nd push> PG3==0> break;
			}
			

			SetLCDAddress(LINE1);
			DisplayString((unsigned char *)s[i]);
			delay_ms(120);
		}
	}
}
void DisplayChar(unsigned char c)
{
	LCD_WRITE = c;
	delay_us(50);
}
void DisplayString(unsigned char* s)
{ // 문자열 출력함수. 주어진 문자열의 길이만큼 반복 출력한다.
	unsigned char i, j;
	i = 0;
	j = strlen((char *)s)-1; // 

	for(i=0;i<=j;i++)
	{ // 문자열의 길이만큼 한 글자씩 출력한다.
		DisplayChar(s[i]);
	}
}
void SetLCDAddress(unsigned char mode)
{
// 어드레스 세팅을 위해서는 MSB는 무조건 1이 되어야 함.
// 사용자가 원한 주소를 받아 MSB를 1로 만들어준다.
// Set DD RAM Address 참조
	LCD_EN = 0x80 | mode;
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
	
