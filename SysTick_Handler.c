#include <stm32f0xx.h>
#include <stdbool.h>
#include <stdbool.h>

typedef struct _Button
{
	bool currentTrueState;
	bool presssed;
	bool nowRawState;
	bool wasFrontUp;
	bool wasFrontDown;
	bool valChanged;
	int countTicksAfterLastToggling;
	int ticksDisabling;
} Button;

typedef struct Frame_
{
	bool data[8][8];
	int stage;
} Frame;
Button buttons[4];
static uint32_t orange_led = GPIO_ODR_8;
static uint32_t green_led = GPIO_ODR_9;
static uint32_t red_led = GPIO_ODR_6;
static uint32_t blue_led = GPIO_ODR_7;

Frame frame;

void buttonInitialization(Button* button_);
void sleep(unsigned int sec);
void init(void);
void SysTick_Handler(void);
void ButtonEveryTick(Button* button, bool rawState);
void setBitV(volatile uint32_t* bit, uint32_t value);
void resetBitV(volatile uint32_t* bit, uint32_t value);
void dot_runner(int pos_x, int pos_y);
void drawPointMatrix(int x, int y);
void drawCross(int x, int y);

void buttonInitialization(Button* button_)
{
	button_->currentTrueState = false;
	button_->presssed = false;
	button_->nowRawState = false;
	button_->wasFrontUp = false;
	button_->wasFrontDown = false;
	button_->valChanged = false;
	button_->countTicksAfterLastToggling = 0;
	button_->ticksDisabling = 5;
}

void sleep(unsigned int sec)
{
	unsigned int timer = SystemCoreClock / 2000 * sec;
	SysTick->LOAD = timer;
	SysTick->VAL = timer;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
	while (1)
	{
		if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
		{
			break;
		}
	}
}

void init(void)
{
	RCC->APB1ENR |= RCC_AHBENR_GPIOBEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOAEN;
	///LED initialization 
	GPIOC->MODER |= GPIO_MODER_MODER12_0
		| GPIO_MODER_MODER6_0
		| GPIO_MODER_MODER7_0
		| GPIO_MODER_MODER8_0
		| GPIO_MODER_MODER9_0;

	GPIOA->MODER |= GPIO_MODER_MODER15_0;
	GPIOA->MODER &= ~GPIO_MODER_MODER4;
	GPIOA->MODER &= ~GPIO_MODER_MODER5;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR4_1 | GPIO_PUPDR_PUPDR5_1;
	///SPI initialization
	// Set Alternate function 1 to pin PB15
	GPIOB->AFR[1] = 0 << (15 - 8) * 4;
	// Set alternate func 1 (AF0) to pin PB13
	GPIOB->AFR[1] = 0 << (13 - 8) * 4;
	// Clocking
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOAEN;
	SPI2->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_BR
		| SPI_CR1_MSTR | SPI_CR1_CPOL | SPI_CR1_CPHA;
	SPI2->CR2 = SPI_CR2_DS;
	GPIOB->MODER |= GPIO_MODER_MODER13_1 | GPIO_MODER_MODER15_1;
	GPIOA->MODER |= GPIO_MODER_MODER8_0;
	SPI2->CR1 |= SPI_CR1_SPE;
	//button initialization
	for (int i = 0; i < 4; ++i) {
		buttonInitialization(&buttons[i]);
	}
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 5000);
}

void SysTick_Handler(void)
{
	GPIOA->ODR &= ~GPIO_ODR_15;
	GPIOC->ODR &= ~GPIO_ODR_12;
	GPIOA->IDR &= ~GPIO_IDR_4;
	GPIOA->IDR &= ~GPIO_IDR_5;

	GPIOA->ODR |= GPIO_ODR_15;
	GPIOC->ODR &= ~GPIO_ODR_12;

	ButtonEveryTick(&buttons[0], GPIOA->IDR & GPIO_IDR_4);
	ButtonEveryTick(&buttons[1], GPIOA->IDR & GPIO_IDR_5);

	GPIOA->ODR &= ~GPIO_ODR_15;
	GPIOC->ODR &= ~GPIO_ODR_12;
	GPIOA->IDR &= ~GPIO_IDR_4;
	GPIOA->IDR &= ~GPIO_IDR_5;

	GPIOA->ODR &= ~GPIO_ODR_15;
	GPIOC->ODR |= GPIO_ODR_12;

	ButtonEveryTick(&buttons[2], GPIOA->IDR & GPIO_IDR_4);
	ButtonEveryTick(&buttons[3], GPIOA->IDR & GPIO_IDR_5);

	GPIOA->ODR &= ~GPIO_ODR_15;
	GPIOC->ODR &= ~GPIO_ODR_12;
	GPIOA->IDR &= ~GPIO_IDR_4;
	GPIOA->IDR &= ~GPIO_IDR_5;
	if (!(SPI2->SR & SPI_SR_BSY))
	{
		if (frame.stage % 2 == 0)
		{
			GPIOA->ODR &= ~GPIO_ODR_8;
			SPI2->DR = (((0x1U << 0) & (frame.data[0][frame.stage / 2] ? 0xFF : 0x00)) |
						((0x1U << 1) & (frame.data[1][frame.stage / 2] ? 0xFF : 0x00)) |
						((0x1U << 2) & (frame.data[2][frame.stage / 2] ? 0xFF : 0x00)) |
						((0x1U << 3) & (frame.data[3][frame.stage / 2] ? 0xFF : 0x00)) |
						((0x1U << 4) & (frame.data[4][frame.stage / 2] ? 0xFF : 0x00)) |
						((0x1U << 5) & (frame.data[5][frame.stage / 2] ? 0xFF : 0x00)) |
						((0x1U << 6) & (frame.data[6][frame.stage / 2] ? 0xFF : 0x00)) |
						((0x1U << 7) & (frame.data[7][frame.stage / 2] ? 0xFF : 0x00))) << 8 |
						 (0x1U << frame.stage / 2);
		}
		if (frame.stage % 2 == 1)
		{
			GPIOA->ODR |= GPIO_ODR_8;
		}
		++frame.stage;
		if (frame.stage == 16)
		{
			frame.stage = 0;
		}
	}
}

void ButtonEveryTick(Button* button, bool rawState)
{
	button->valChanged = false;
	if (button->nowRawState == false && rawState == true)
	{
		button->wasFrontUp = true;
	}
	else
	{
		button->wasFrontUp = false;
	}
	if (button->nowRawState == true && rawState == false)
	{
		button->wasFrontDown = true;
	}
	else
	{
		button->wasFrontDown = false;
	}

	button->nowRawState = rawState;

	if (!button->wasFrontUp && !button->wasFrontDown)
	{
		button->countTicksAfterLastToggling++;
		if (button->countTicksAfterLastToggling > button->ticksDisabling)
		{
			button->countTicksAfterLastToggling = button->ticksDisabling;
		}
	}
	else
	{
		button->countTicksAfterLastToggling = 0;
	}

	if (button->countTicksAfterLastToggling == button->ticksDisabling)
	{
		if (button->currentTrueState != button->nowRawState)
		{
			button->currentTrueState = button->nowRawState;
			if (button->currentTrueState) {
				button->presssed = !button->presssed;
				button->valChanged = true;
			}
		}
	}
}

void setBitV(volatile uint32_t* bit, uint32_t value)
{
	*bit |= value;
}

void resetBitV(volatile uint32_t* bit, uint32_t value)
{
	*bit &= ~value;
}

void dot_runner(int pos_x, int pos_y)
{
	resetBitV(&GPIOC->ODR, red_led);
	resetBitV(&GPIOC->ODR, blue_led);
	resetBitV(&GPIOC->ODR, orange_led);
	resetBitV(&GPIOC->ODR, green_led);


	if (buttons[0].valChanged)
	{
		if (pos_y > 0)
		{
			--pos_y;
		}
	}
	if (buttons[1].valChanged)
	{
		if (pos_y < 7)
		{
			++pos_y;
		}
	}
	if (buttons[2].valChanged)
	{
		if (pos_x < 7)
		{
			++pos_x;
		}
	}
	if (buttons[3].valChanged)
	{
		if (pos_x > 0)
		{
			--pos_x;
		}
	}
	GPIOA->ODR &= ~GPIO_ODR_8;
	while (SPI2->SR & SPI_SR_BSY);
	SPI2->DR = (0x1U << pos_x) << 8 | (0x1U << pos_y);
	GPIOA->ODR |= GPIO_ODR_8;
}

void drawPointMatrix(int x, int y)
{
	GPIOA->ODR &= ~GPIO_ODR_8;
	while (SPI2->SR & SPI_SR_BSY);
	SPI2->DR = (0x1U << (x)) << 8 | (0x1U << (y));
	GPIOA->ODR |= GPIO_ODR_8;
	while (SPI2->SR & SPI_SR_BSY);
}
bool updated = true;

void drawCross(int x, int y)
{
	if (updated) {
		for (int i = 0; i < 8; i++)
			for (int j = 0; j < 8; j++)
			{
				frame.data[i][j] = false;
			}

		frame.data[x][y] = true;
		frame.data[x + 1][y] = true;
		frame.data[x - 1][y] = true;
		frame.data[x][y - 1] = true;
		frame.data[x][y + 1] = true;
		updated = false;
	}
}

int main(void)
{
	init();
	int pos_x = 1;
	int pos_y = 1;
	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 8; ++j)
		{
			frame.data[i][j] = false;
		}
	}
	frame.stage = 0;
	drawCross(pos_x, pos_y);
	while (1)
	{
		if (buttons[0].valChanged)
		{
			buttons[0].valChanged = false;
			if (pos_y > 1)
			{
				--pos_y;
			}
			updated = true;
		}
		if (buttons[1].valChanged)
		{
			buttons[1].valChanged = false;
			if (pos_y < 6)
			{
				++pos_y;
			}
			updated = true;
		}
		if (buttons[2].valChanged)
		{
			buttons[2].valChanged = false;
			if (pos_x < 6)
			{
				++pos_x;
			}
			updated = true;
		}
		if (buttons[3].valChanged)
		{
			buttons[3].valChanged = false;
			if (pos_x > 1)
			{
				--pos_x;
			}
			updated = true;
		}
		drawCross(pos_x, pos_y);
	}
}

