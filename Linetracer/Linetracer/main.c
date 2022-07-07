#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


void Uart_Init(void);
//일단 시작 하는 걸 버튼 1을 넣었을 때 라고 생각하고
//주행 모드를 코딩해 볼까요
//예외 처리 없이 일단 가게 만들어보자 


int cnt;				//주기를 정해주겠어
int ModeSet;			//모드 어떤 걸로 할 지 담는 녀석.
void Get_ADC(void);		//ADC 값 받아오는 녀석임.
void MaxMin(void);		//ADC값 하이 로우 받기
void SpeedControl(void); //모터 스피드 바꾸는 거 여기서 그걸 해보자고 바로 주행하는 걸
int adc[8];
double adcmax[8] = {0,0,0,0,0,0,0,0};
double adcmin[8] = {2000,2000,2000,2000,2000,2000,2000,2000};
double normalize[8];
unsigned int IR[8];
unsigned int Weight[8] = {-8, -4, -2, -1, 1, 2, 4, 8};
int total = 0;


ISR(INT0_vect){
	//MODE 최대 최소 받기
	
	ModeSet=0;
	
	
}




ISR(INT1_vect){
	//MODE 주행
	
	ModeSet=1;
	
}


ISR(TIMER0_OVF_vect){
	
	cnt++;
	TCNT0 = 131;
						
	if (cnt==100)						//0.1초
	{									//이제 주행하는 걸 넣어야 되는 걸까??
		
		ADMUX = 0x40;
		for(int i=0;i<8;i++){
			ADCSRA |= (1<< ADSC);
			++ADMUX;
			while(!(ADCSRA&(1<<ADIF)));
			adc[i] = ADC;				// 반복문 돌면서 에이디씨 제로부터 배열에 집어 넣는 거임.
		}
										//ADC를 받아라잉
		if(ModeSet==0)
		{
			for(int n = 0; n < 8; n ++) //큰가 작은가
			{
				if( adc[n] > adcmax[n]) // 클때 넣는거
					adcmax[n] = adc[n];
				if( adc[n] < adcmin[n]) //작을 때 넣는 거
					adcmin[n] = adc[n];
			}
		}
		//라면 최대최소를 받으라
		
		if(ModeSet==1){
			
			
			for(int t =0 ; t < 8; t++)
			{
				/*NOrmalize Data = (data - min)/ (max-min)* resolution*/

				normalize[t] = ((double)(adc[t]-adcmin[t]))/(adcmax[t]-adcmin[t])*100;
			}
			
			for(int b =0 ; b < 8; b++)
			{
				if (normalize[b] < 50)
				{	
					IR[b] = 1; // black
				}
				else {
					IR[b] = 0; // white
				}
			}
			
			total = 0;
			for(int k=0; k<8; k++){
				total += IR[k]*Weight[k];
			}
			
			if(total == 0){
				OCR1A = 639; OCR1B = 639;
			}
			else if (total < 0){//왼쪽으로 돌아라잉
				OCR1A = 639; OCR1B = 0;
			}
			else if(total > 0){//오른쪽으로 돌아라잉
				OCR1A = 0; OCR1B = 639;
			}
			
			
			//라면 주행하는걸 하겠지
			//정규화 과정
			//주행
			/*
			double Weight[8] = {-8, -4, -2, -1, 1, 2, 4, 8};
			double weightData[8];
			for(int k=0; k<8; k++){
				weightData[k] = normalize[k]*Weight[k];}
			double TotalWeight = weightData[0] + weightData[1] + weightData[2] + weightData[3] + weightData[4] + weightData[5] + weightData[6] + weightData[7];
			
			//밑에 지우고 토탈 weight 에 따라서 모터 속도 조절을 하면 되겠구먼 ㅎㅎㅎ
			
			if (TotalWeight > 2048 ){//1024*2
				PORTA = 0x0f; OCR1B = 499; OCR1A = 0; // 오른쪽으로 돌기?
				
			}
			
			if(TotalWeight < -2048){//1024*-2
				PORTA = 0xf0; OCR1A = 0; OCR1A = 499; // 왼쪽으로 돌기?	
					}
					
		
			if(TotalWeight < 2048 && TotalWeight > -2048){//1024*-2
				 OCR1A = 499; OCR1A = 499; // 왼쪽으로 돌기?
			}*/	
		}
	}
}


int main(void)
{
	
	
	DDRA=0xFF;//출력 설정
	DDRF=0x00;//IR적외선 입력 설정
	
	ADCSRA=0b10000111;//ADC활성화  128분주비 
	//int led=0xFF;//1111 1111 다 꺼짐
	
	///타이머 레지스터 설정
	
	TCCR0 = (0<<WGM01)|(0<<WGM00)|(0<<COM01)|(0<<COM00)|(1<<CS02)|(1<<CS01)|(0<<CS00);// 노말 모드 논인벌팅 모드 256 분주비
	TIMSK = (1<<TOIE0);		//0번 오벌플로우 인터럽트
	TCNT0 = 131;//분주비 계산하기 거시기해서 예제코드랑 같은거 씀
	
	
	 Uart_Init();
	
	////모터 설정~~~~
	
	DDRB = 0xFF;	//출력 설정 1111 1111 모터
	DDRD = 0b00001000;	//입력설정 스위치
	DDRE = 0x0F;	//모터방향출력 설정
	
	PORTE=0x0a;		//모터 방향 설정  0b 1010 1010
	
	ADMUX = (1<< REFS1);// 기준 전압 5볼트
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<< ADPS0);
	
	
	TCCR1A = (1<<COM1A1)|(0<<COM1A0)|(1<<COM1B1)|(0<<COM1B0)|(1<<WGM11);//타이머 AB케널 PWM설정 빠른, 피엠아 모드설정
	TCCR1B = (1<<WGM13)|(1<<WGM12)|(0<<CS02)|(0<<CS01)|(1<<CS00);//14모드 설정 페스트피더블유엠 탑이 아이시알 오시알은 바텀
	//뒤에 3개는 분주비의미 인데 1분주비
	
	EIMSK = (1<<INT0)|(1<<INT1);
	
	EICRA = (1<<ISC01)|(0<<ISC00)|(1<<ISC11)|(0<<ISC10);
	
	ICR1 = 799;//탑값
	OCR1A = 0;//에이채널 피더블유엠
	OCR1B = 0;//비 채널    모터출력 80퍼라는 뜻 799*0.8값639
	
	
	sei();
	
    while (1) 
    {
		///////////////////////
		//PORTA=~led;
		//led=0b00000000;
		
		/*for(int a=0;a<8;a++)//채널 반북하기
		{
			
			ADMUX=0b01000000|a;				//아날로그 입력채널 선택 전압선택 변환결과 저장 형식지정
			ADCSRA = ADCSRA|(1<<ADSC);		//ADC변환시작
			while(!(ADCSRA&(1<<ADIF)));		//값이 들어왔어
						//ADC값을 넣어 adc에다가
			if(adc[a]< 500)						//근데 그 값이 500이 넘어
			{
				//그러면 led켜!
				led = led|(1<<a);			//채널에 맞게 led켜기
			}
		
		 }*/
	}

}

void Uart_Init()
{
	UCSR1A=0x00;
	UCSR1B=(1<<RXEN)|(1<<TXEN);
	UCSR1C=(1<<UCSZ11)|(1<<UCSZ10);
	UBRR1H=0;
	UBRR1L=103;
}

void Uart_Trans(unsigned char data)
{
	while (!( UCSR1A & (1<<UDRE1)));
	UDR1 = data;
}