// LAB 8. EX2

#include <avr/io.h>
#include <string.h>

// LCD ���� �����ϱ� ���� �ܺ� �޸� �ּ� ����
#define LCD_EN (*(volatile unsigned char *)0x8000)
#define LCD_WRITE (*(volatile unsigned char *)0x8002)

// LCD ����� ù��° ����, �ι�° ���� �ּ�
#define LINE1 0x00 // LCD_DISPLAY_LINE1
//#define LINE2 0x40 // LCD_DISPLAY_LINE2

// ����
void delay_us(unsigned char);
void delay_ms(unsigned int);

// LCD �ʱ�ȭ ����
void FunctionSet();
void DisplayOnOffControl();
void DisplayClear();
void EntryModeSet();

// LCD ����ǥ�� (���� �� ���ڿ�)
void DisplayChar(unsigned char);
void DisplayString(unsigned char *);
void SetLCDAddress(unsigned char);

int main(void)
{
	MCUCR= 0x00;	// ?? what function ??
	DDRG= 0X00;	//PG3: 0_IN


	int i;
	// LCD�� ǥ���� ���ڿ�
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

	// SFR �ʱ�ȭ
	MCUCR = 0x80; // I/O ��Ʈ�� Ư����� ���� ����
	XMCRA = 0X40; // �ܺ� �޸� ���� ����
	XMCRB = 0X80; // �ܺ� �޸� ���� ����
	// LCD �ʱ�ȭ
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
{ // ���ڿ� ����Լ�. �־��� ���ڿ��� ���̸�ŭ �ݺ� ����Ѵ�.
	unsigned char i, j;
	i = 0;
	j = strlen((char *)s)-1; // 

	for(i=0;i<=j;i++)
	{ // ���ڿ��� ���̸�ŭ �� ���ھ� ����Ѵ�.
		DisplayChar(s[i]);
	}
}
void SetLCDAddress(unsigned char mode)
{
// ��巹�� ������ ���ؼ��� MSB�� ������ 1�� �Ǿ�� ��.
// ����ڰ� ���� �ּҸ� �޾� MSB�� 1�� ������ش�.
// Set DD RAM Address ����
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
	
