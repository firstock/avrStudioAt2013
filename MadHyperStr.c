// 미구현	//오후 2:36 2013-11-21 > 지금것은 abcd 입력하면 bcda 한번 출력하고 말음.

#include <avr/io.h>
#include <avr/interrupt.h> 
// C:\Program Files\Atmel\Atmel Studio 6.0\extensions\Atmel\AVRGCC\3.4.1.95\AVRToolchain\avr\include\avr
#include <string.h>
// C:\Program Files\Atmel\Atmel Studio 6.0\extensions\Atmel\AVRGCC\3.4.1.95\AVRToolchain\avr32\include

#define BYTE  unsigned char

												// LCD 모듈로 접근하기 위한 외부 메모리 주소 선언
#define LCD_EN (*(volatile unsigned char *)0x8000)
#define LCD_WRITE (*(volatile unsigned char *)0x8002)

#define LINE1 0x00 // LCD_DISPLAY_LINE1			// LCD 모듈의 1,2 번째 라인 주소
//#define LINE2 0x40 // LCD_DISPLAY_LINE2

void delay_us(unsigned char);
void delay_ms(unsigned int);

void FunctionSet();								// LCD 초기화
void DisplayOnOffControl();
void DisplayClear();
void EntryModeSet();

void DisplayChar(unsigned char);				// LCD 글자표시 함수 (문자 및 문자열)
void DisplayString(unsigned char *);
void SetLCDAddress(unsigned char);

unsigned char rxd_data=0x00; 
unsigned char txd_data=0x00; 

unsigned char arr_str[20]=  {0,};
unsigned char flow_str[20]= {0,};
int i=0;					// for input string[i]
int j=0;					// for 흐르는(flowing) 문자열
char ch_temp=0;				// 문자열의 첫번째 글자를 뒤로 옮기는 용도.

SIGNAL(SIG_UART0_RECV)		// USART 0 receive interrupt vector		//키 입력때만 잠~깐 들어오는것에 유의.
{ 
	rxd_data = UDR0;
	txd_data = rxd_data;
	UDR0 = txd_data;		// echo back received data
	
	arr_str[i++]= rxd_data;	// <- 가정: 입력이 20개 이하

	if(rxd_data== 13)		// 13 : \n(엔터)
	{
		i= 0;
		SetLCDAddress(LINE1);
		//DisplayString((unsigned char *)arr_str);	// 엔터칠때까지의 문자열이 LCD에 나타남. //test

		strcpy(flow_str, arr_str);
			
		ch_temp = flow_str[0];
		for(j=0; j<19; j++)
		{	
			if(arr_str[j]== 13) break;
			flow_str[j]= flow_str[j+1];				
		}
		flow_str[j-1]= ch_temp;							// j-1: \n 자리에 첫글자 넣기.

		DisplayString((unsigned char *)flow_str);
	}
	
}
void TXChar(BYTE tx_data)
{
  
  // wait until transmitter buffer is ready
	while((UCSR0A&0x20) == 0x00); 	//USCR0A[0010 0000]= UDREn bit[ 1:transmitter buffer empty, 0: not empty ]
	UDR0 = tx_data;		// send data to transmitter buffer
						//UDR0 = USART0의 transmitter buffer.
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
	UCSR0B = 0x98;	// Receiver, Transmitter 기능 활성화
	UCSR0C = 0x06;	// character size : 8bit
	UBRR0H = 0x00;	// baud rate 57600 UBRR0=16
	UBRR0L = 0x10;
	//(MCU Clock / 16 * 57600) - 1 = 16.36 => 16 
	//16을 UBRRO에 넣어준다. 


					// SFR 초기화
	MCUCR = 0x80;	// I/O 포트를 특수기능 모드로 설정
	XMCRA = 0X40;	// 외부 메모리 섹터 설정
	XMCRB = 0X80;	// 외부 메모리 섹터 설정

					// LCD 초기화
	FunctionSet();
	DisplayOnOffControl();
	DisplayClear();
	EntryModeSet();
	
	SetLCDAddress(LINE1);
	PutChar((unsigned char *)"Enter a message\r\n", 0);
	//\r: carrage return. 커서를 줄의 맨 처음으로.

	sei();			// enable interrupt

	while(1);
}



// ----------------= 이하: LCD 세팅 =------------------ //
void DisplayChar(unsigned char c)
{
	LCD_WRITE = c;
	delay_us(50);
}
void DisplayString(unsigned char* s)
{ // 문자열 출력함수. 주어진 문자열의 길이만큼 반복 출력한다.
	unsigned char i, j;
	i = 0;
	j = strlen((char *)s)-1; //Null을 제외하기 위해서 1 감소한다.
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

