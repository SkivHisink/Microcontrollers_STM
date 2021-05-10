#include <stm32f0xx.h>
#include <time.h>
//#define SLEEP PCON |= 0x01
void Init(void);
void timer_init(uint32_t period);

void Init()
{
	 RCC->AHBENR |= RCC_AHBENR_GPIOCEN|RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~GPIO_MODER_MODER0;
	GPIOC->MODER |= GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0;
}

void timer_init(uint32_t period)
{
	SysTick->LOAD = period - 1;
	SysTick->VAL = period - 1;
	SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
}

void sleep(unsigned int sec);
void setBit( unsigned int port);
void resetBit( unsigned int port);
//PA0 user button

void setBit(unsigned int port)
{
	GPIOC->ODR |= port;
}

void resetBit(unsigned int port)
{
	GPIOC->ODR &= ~port;
}
volatile uint32_t timestamp = 0;

void SysTick_Handler (void)
{
  // timestamp++;
	//if(timestamp>SYSTICK_MAX_VALUE)
		//timestamp=0;
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

static unsigned int red= GPIO_ODR_6;
static unsigned int yellow= GPIO_ODR_8;
static unsigned int green= GPIO_ODR_9;
static unsigned int blue =GPIO_ODR_7;

void stopSignal()
{
			sleep(10);
			resetBit(red);
			resetBit(green);
			sleep(10);
			setBit(green);
			sleep(10);
			resetBit(green);
			sleep(10);
			setBit(green);
			sleep(10);
			resetBit(green);
			sleep(10);
	    setBit(yellow);
			sleep(20);
			resetBit(yellow);
	    setBit(red);
}
void greenSignal()
{
			setBit(yellow);
			sleep(20);
	    resetBit(red);
			resetBit(yellow);
			sleep(2);
			setBit(green);
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
	int sig=1;
	timer_init(8000);
	while(1)
	{
		
		if(status==1){
			setBit(green);
			status=0;
			timestamp=0;
		}	
		if (GPIOA->IDR & GPIO_IDR_0 && status==0){
			stopSignal();
			timestamp=0;
			status=2;
		}
		if(timestamp>5000 && (status==0 )){
			stopSignal();
			timestamp=0;
			status=2;
		}
		if(timestamp>5000 && status==2){
			greenSignal();
			timestamp=0;
			status=0;
			if (GPIOA->IDR & GPIO_IDR_0){}
		}
		//comment it if you want use it with handler
		sleep(1);timestamp+=100;
	}
}
