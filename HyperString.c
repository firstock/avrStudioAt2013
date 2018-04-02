// Hyper~LCD

#include <avr/io.h>
#include <avr/interrupt.h> 
#include <string.h>

#define BYTE  unsigned char

												// LCD ���� �����ϱ� ���� �ܺ� �޸� �ּ� ����
#define LCD_EN (*(volatile unsigned char *)0x8000)
#define LCD_WRITE (*(volatile unsigned char *)0x8002)

#define LINE1 0x00 // LCD_DISPLAY_LINE1			// LCD ����� 1,2 ��° ���� �ּ�
//#define LINE2 0x40 // LCD_DISPLAY_LINE2

void delay_us(unsigned char);
void delay_ms(unsigned int);

void FunctionSet();								// LCD �ʱ�ȭ
void DisplayOnOffControl();
void DisplayClear();
void EntryModeSet();

void DisplayChar(unsigned char);				// LCD ����ǥ�� �Լ� (���� �� ���ڿ�)
void DisplayString(unsigned char *);
void SetLCDAddress(unsigned char);

unsigned char rxd_data=0x00; 
unsigned char txd_data=0x00; 

unsigned char arr_str[20]=  {0,};
unsigned char flow_str[20]= {0,};
int i=0;		// for input string[i]
int j=0;		// for �帣��(flowing) ���ڿ�
char ch_temp=0;		// ���ڿ��� ù��° ���ڸ� �ڷ� �ű�� �뵵.


SIGNAL(SIG_UART0_RECV)		// USART 0 receive interrupt vector
{ 
	rxd_data = UDR0;
	txd_data = rxd_data;
	UDR0 = txd_data;		// echo back received data
	
	arr_str[i++]= rxd_data;	// <- ����: �Է��� 20�� ����

	if(rxd_data== 13)		// 13 : \n(����)
	{
		i= 0;
		SetLCDAddress(LINE1);
		// DisplayString((unsigned char *)arr_str);	// ����ĥ�������� ���ڿ��� LCD�� ��Ÿ��.
		//test// �ڵ� ��û �ٲ�µ� ������ �� �۵��ϸ� �Ʒ� �ּ� /* ~ */ Ǯ� ��.



		/*
		flow_str =  arr_str	// < right?. strcpy ����� �Ƚᵵ �ǳ�?		strcpy(flow_str, arr_str);

		delay_ms(300);		// < why?
			
		ch_temp = flow_str[0];
		for(j=0; j<19; j++)
		{	
			// if(arr_str[j]== 13) break;			// ���1
			flow_str[j]= flow_str[j+1];				
		}

		flow_str[19]= ch_temp;		// 19���ڰ� �ƴҰ��, Hello Worl         d ��� ������ ������ ����.
		// flow_str[j-1]= ch_temp;					// ���1		// j-1: \n �ڸ��� ù���� �ֱ�.

		//�ذ�� ���1: if( == \n) break;
		// ex. abcd+���� -> flow_str[]= arr_str[]= {a,b,c,d,\n}
						// 19,20 ������ strlen�� �ٲ��ʿ� ����!
		// cf. ����
					
		DisplayString((unsigned char *)flow_str);
		*/


	}
	
}
void TXChar(BYTE tx_data)
{
  
  // wait until transmitter buffer is ready
	while((UCSR0A&0x20) == 0x00); 
	//USCR0A�� 0x20 �κп� �ִ� bit�� UDREn bit��
	//UDREn bit�� ���� : transmitter buffer�� ��������� 1, ������ ����( �� �۽� �غ� �Ǹ�) 0
	UDR0 = tx_data;		// send data to transmitter buffer
						//UDR0 = USART0�� transmitter buffer��.
}

/* maybe test statement?
BYTE strlen( BYTE *str )
{
	BYTE i=0;

	while( *str++ ) i++;
	return i;
}
*/


void PutChar(BYTE *databuf,BYTE num)
{
	BYTE CharCount;
	BYTE i;

	CharCount = num	;
	if( !CharCount ) CharCount = strlen( (char *)databuf );
	for(i=0; i<CharCount; i++){
		TXChar(databuf[i]);
	}
}

int main(void)
{
	//SP=0x10ff;	// stack pointer
	cli();			// disable interrupt

	MCUCR = 0x01;	// Enable change of interrupt vectors IVCE=1
	MCUCR = 0x00;	// interrupt vector to flash start address IVSEL=0
	//���ͷ�Ʈ ������ ����ġ ���� ������ �����ϱ� ���� ���� IVCE�� 1�� �ְ�,
	//4cycle�� ���� ��, IVSEL ��Ʈ�� ���ϴ� ���� ����

	/* USART0 initialization */
	UCSR0A = 0x00;
	UCSR0B = 0x98;	// Receiver, Transmitter ��� Ȱ��ȭ
	UCSR0C = 0x06;	// character size : 8bit
	UBRR0H = 0x00;	// baud rate 57600 UBRR0=16
	UBRR0L = 0x10;
	//(MCU Clock / 16 * 57600) - 1 = 16.36 => 16 
	//16�� UBRRO�� �־��ش�. 


	// SFR �ʱ�ȭ
	MCUCR = 0x80; // I/O ��Ʈ�� Ư����� ���� ����
	XMCRA = 0X40; // �ܺ� �޸� ���� ����
	XMCRB = 0X80; // �ܺ� �޸� ���� ����
	// LCD �ʱ�ȭ
	FunctionSet();
	DisplayOnOffControl();
	DisplayClear();
	EntryModeSet();

	PutChar((unsigned char *)"Enter a message\r\n", 0);
	//\r�� ���� ���� �� ó������ ����� ��ɾ���.

	sei();			// enable interrupt

	while(1);
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
	j = strlen((char *)s)-1; //Null�� �����ϱ� ���ؼ� 1 �����Ѵ�.
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


/* flow string

#include <avr/io.h>
#include <string.h>

// LCD ���� �����ϱ� ���� �ܺ� �޸� �ּ� ����
#define LCD_EN (*(volatile unsigned char *)0x8000)
#define LCD_WRITE (*(volatile unsigned char *)0x8002)
// LCD ����� ù��° ����, �ι�° ���� �ּ�
#define LINE1 0x00 // LCD_DISPLAY_LINE1
//#define LINE2 0x40 // LCD_DISPLAY_LINE2

void delay_us(unsigned char);
void delay_ms(unsigned int);
// LCD �ʱ�ȭ ����
void FunctionSet();
void DisplayOnOffControl();
void DisplayClear();
void EntryModeSet();
// LCD ����ǥ�� �Լ� (���� �� ���ڿ�)
void DisplayChar(unsigned char);
void DisplayString(unsigned char *);
void SetLCDAddress(unsigned char);

int main(void)
{
	MCUCR= 0x00;
	DDRG= 0X00;	//PG3: 0_IN


	int i;
	// LCD�� ǥ���� ���ڿ�
	// 2nd array- table. >> better?
	char s[11] = {"Hello World"};
	char s_temp=0;

	// SFR �ʱ�ȭ
	MCUCR = 0x80; // I/O ��Ʈ�� Ư����� ���� ����
	XMCRA = 0X40; // �ܺ� �޸� ���� ����
	XMCRB = 0X80; // �ܺ� �޸� ���� ����
	// LCD �ʱ�ȭ
	FunctionSet();
	DisplayOnOffControl();
	DisplayClear();
	EntryModeSet();
	// LCD�� ù �ٿ� Hello ǥ��

	while(1)
	{
		SetLCDAddress(LINE1);
		DisplayString((unsigned char *)s);
		delay_ms(300);	
			
		s_temp = s[0];
		for(i=0; i<10; i++)
		{	
			s[i]=s[i+1];				
		}
		s[10]=s_temp;
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
	j = strlen((char *)s)-1; //Null�� �����ϱ� ���ؼ� 1 �����Ѵ�.
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
	

*/
