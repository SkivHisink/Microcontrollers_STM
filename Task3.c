#include <stm32f0xx.h>
#include <time.h>
//#define SLEEP PCON |= 0x01
void Init(void);
void timer_init(uint32_t period);
//A15 C12 in| out A4 A5

static unsigned int red = GPIO_ODR_6;
static unsigned int yellow = GPIO_ODR_8;
static unsigned int green = GPIO_ODR_9;
static unsigned int blue = GPIO_ODR_7;
void Init()
{
	 RCC->AHBENR |= RCC_AHBENR_GPIOCEN|RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~GPIO_MODER_MODER0;
	GPIOC->MODER |= GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0 
	| GPIO_MODER_MODER12_0;
	GPIOA->MODER |= GPIO_MODER_MODER15_0 | GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1; 
}

void timer_init(uint32_t period)
{
	SysTick->LOAD = period - 1;
	SysTick->VAL = period - 1;
	SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
}

static struct signal{
	int pos;
	int state;
};
	static struct signal in_1 = {GPIO_ODR_15, 0};
	static struct signal in_2 = {GPIO_ODR_12, 0};
	static struct signal out_1 = {GPIO_IDR_4, 0};
	static struct signal out_2 = {GPIO_IDR_5, 0};

void sleep(unsigned int sec);
void setBit( unsigned int port);
void resetBit( unsigned int port);
//PA0 user button
void setBitA(unsigned int port)
{
	GPIOA->ODR |= port;
}
void setBitC(unsigned int port)
{
	GPIOC->ODR |= port;
}
void resetBitC(unsigned int port)
{
	GPIOC->ODR |= port;
}
void resetBitA(unsigned int port)
{
	GPIOA->ODR &= ~port;
}
volatile uint32_t timestamp = 0;

void SysTick_Handler (void)
{
  // timestamp++;
	//if(timestamp>SYSTICK_MAX_VALUE)
		//timestamp=0;
	if(timestamp ==0)
		setBitA(red);
	timestamp++;
	if(GPIOA->IDR && out_1.pos && out_2.pos){
		out_1.state=1;
		out_2.state=0;
	}
	else if(GPIOA->IDR && out_1.pos){
		out_1.state=1;
		out_2.state=0;
	}
	else if(GPIOA->IDR && out_2.pos){
		out_2.state=0;
		out_1.state=1;
	}
}
void sleeper(unsigned int sec)
{
	timestamp=0;
	while(1){
	if(timestamp>sec*100)
		break;
	}
}

#define SYSTICK_MAX_VALUE 16777215
void sleep(const uint32_t time) {
    if (time > SYSTICK_MAX_VALUE || time == 0)
        return;
		
    SysTick->LOAD = (SystemCoreClock/10 * time);
    SysTick->VAL = (SystemCoreClock/10 *time);
		SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
		while(1)
		{
			if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk){
				break;
			}
		}
}

//6-green
//7-yellow
//9-red
//statuses:
//1 - init
//0 - green light
//2 - red light

int main(void)
{
	Init();
	int status = 1;
	int sig_1;
	int sig_2;

	int button1 = 0;
	int button2 = 0;
	int button3 = 0;
	int button4 = 0;
	timer_init(8000);
	while(1)
	{
		if(out_1.state == 1) {
				button1 = 1;
			}
			if(out_2.state == 1) {
				button3 = 1;
			}
		GPIOC->BSRR |= in_1.pos;
		GPIOA->BSRR |= in_2.pos;
		if(out_1.state == 1){
			GPIOA->BSRR &= in_2.pos;
			if(out_1.state == 1) {
				button1 = 1;
			}
			if(out_2.state == 1) {
				button3 = 1;
			}
		}
		if(out_2.state == 1) {
			GPIOC->BSRR &= in_1.pos;
			if(out_1.state == 1) {
				button2 = 1;
			}
			if(out_2.state == 1) {
				button3 = 1;
			}
		}
		if(button1){
			setBitA(red);
		}
		else{
			resetBitA(red);
		}
		if(button2){
			setBitA(yellow);
		}
		else{
			resetBitA(yellow);
		}
		if(button3){
			setBitA(blue);
		}
		else{
			resetBitA(blue);
		}
		if(button4){
			setBitA(green);
		}
		else{
			resetBitA(green);
		}
	}
}
