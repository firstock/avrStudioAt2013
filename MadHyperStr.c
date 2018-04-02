// �̱���	//���� 2:36 2013-11-21 > ���ݰ��� abcd �Է��ϸ� bcda �ѹ� ����ϰ� ����.

#include <avr/io.h>
#include <avr/interrupt.h> 
// C:\Program Files\Atmel\Atmel Studio 6.0\extensions\Atmel\AVRGCC\3.4.1.95\AVRToolchain\avr\include\avr
#include <string.h>
// C:\Program Files\Atmel\Atmel Studio 6.0\extensions\Atmel\AVRGCC\3.4.1.95\AVRToolchain\avr32\include

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
int i=0;					// for input string[i]
int j=0;					// for �帣��(flowing) ���ڿ�
char ch_temp=0;				// ���ڿ��� ù��° ���ڸ� �ڷ� �ű�� �뵵.

SIGNAL(SIG_UART0_RECV)		// USART 0 receive interrupt vector		//Ű �Է¶��� ��~�� �����°Ϳ� ����.
{ 
	rxd_data = UDR0;
	txd_data = rxd_data;
	UDR0 = txd_data;		// echo back received data
	
	arr_str[i++]= rxd_data;	// <- ����: �Է��� 20�� ����

	if(rxd_data== 13)		// 13 : \n(����)
	{
		i= 0;
		SetLCDAddress(LINE1);
		//DisplayString((unsigned char *)arr_str);	// ����ĥ�������� ���ڿ��� LCD�� ��Ÿ��. //test

		strcpy(flow_str, arr_str);
			
		ch_temp = flow_str[0];
		for(j=0; j<19; j++)
		{	
			if(arr_str[j]== 13) break;
			flow_str[j]= flow_str[j+1];				
		}
		flow_str[j-1]= ch_temp;							// j-1: \n �ڸ��� ù���� �ֱ�.

		DisplayString((unsigned char *)flow_str);
	}
	
}
void TXChar(BYTE tx_data)
{
  
  // wait until transmitter buffer is ready
	while((UCSR0A&0x20) == 0x00); 	//USCR0A[0010 0000]= UDREn bit[ 1:transmitter buffer empty, 0: not empty ]
	UDR0 = tx_data;		// send data to transmitter buffer
						//UDR0 = USART0�� transmitter buffer.
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
	MCUCR = 0x00;	// interrupt vector to flash start address IVSEL=0 -> next 4 cycle -> IVCE= 0

	/* USART0 initialization */
	UCSR0A = 0x00;
	UCSR0B = 0x98;	// Receiver, Transmitter ��� Ȱ��ȭ
	UCSR0C = 0x06;	// character size : 8bit
	UBRR0H = 0x00;	// baud rate 57600 UBRR0=16
	UBRR0L = 0x10;
	//(MCU Clock / 16 * 57600) - 1 = 16.36 => 16 
	//16�� UBRRO�� �־��ش�. 


					// SFR �ʱ�ȭ
	MCUCR = 0x80;	// I/O ��Ʈ�� Ư����� ���� ����
	XMCRA = 0X40;	// �ܺ� �޸� ���� ����
	XMCRB = 0X80;	// �ܺ� �޸� ���� ����

					// LCD �ʱ�ȭ
	FunctionSet();
	DisplayOnOffControl();
	DisplayClear();
	EntryModeSet();
	
	SetLCDAddress(LINE1);
	PutChar((unsigned char *)"Enter a message\r\n", 0);
	//\r: carrage return. Ŀ���� ���� �� ó������.

	sei();			// enable interrupt

	while(1);
}



// ----------------= ����: LCD ���� =------------------ //
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

