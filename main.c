#include "stm32f10x.h"

void GPIO_Config(void);
void TIM2_PWM_CH2_Init(void);
void delay_ms(uint32_t ms);
uint8_t button1_pressed(void);
uint8_t button2_pressed(void);
uint8_t button3_pressed(void);

int main(void)
{
    uint8_t motor_on = 0;
    uint8_t duty = 40;   // default duty = 50%

    GPIO_Config();
    TIM2_PWM_CH2_Init();

    // Fixed direction for Motor B
    // IN3 = 1, IN4 = 0
    GPIOA->BSRR = GPIO_BSRR_BS2;   // PA2 HIGH
    GPIOA->BSRR = GPIO_BSRR_BR3;   // PA3 LOW

    // Initially motor OFF
    TIM2->CCR2 = 0;

    while (1)
    {
        // S1: Start / Stop
        if (button1_pressed())
        {
            motor_on = !motor_on;

            if (motor_on)
            {
                TIM2->CCR2 = duty;   // run at current duty
            }
            else
            {
                TIM2->CCR2 = 0;      // stop motor
                duty = 50;           // reset duty to 50%
            }
        }

        // S2: Increase duty by +5%, max 95%
        if (button2_pressed())
        {
            if (duty < 95)
            {
                duty += 5;
            }

            if (motor_on)
            {
                TIM2->CCR2 = duty;
            }
        }

        // S3: Decrease duty by -5%, min 50%
        if (button3_pressed())
        {
            if (duty > 50)
            {
                duty -= 5;
            }

            if (motor_on)
            {
                TIM2->CCR2 = duty;
            }
        }
    }
}

void GPIO_Config(void)
{
    // Enable clocks for GPIOA, GPIOB, AFIO
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    // PA1 -> TIM2_CH2 -> Alternate Function Push Pull
    GPIOA->CRL &= ~(0xF << 4);
    GPIOA->CRL |=  (0xB << 4);

    // PA2 -> Output Push Pull
    GPIOA->CRL &= ~(0xF << 8);
    GPIOA->CRL |=  (0x2 << 8);

    // PA3 -> Output Push Pull
    GPIOA->CRL &= ~(0xF << 12);
    GPIOA->CRL |=  (0x2 << 12);

    // PB0 -> Input Pull-Up (S1)
    GPIOB->CRL &= ~(0xF << 0);
    GPIOB->CRL |=  (0x8 << 0);
    GPIOB->BSRR = GPIO_BSRR_BS0;

    // PB1 -> Input Pull-Up (S2)
    GPIOB->CRL &= ~(0xF << 4);
    GPIOB->CRL |=  (0x8 << 4);
    GPIOB->BSRR = GPIO_BSRR_BS1;

    // PB10 -> Input Pull-Up (S3)
    GPIOB->CRH &= ~(0xF << 8);
    GPIOB->CRH |=  (0x8 << 8);
    GPIOB->BSRR = GPIO_BSRR_BS10;
}

void TIM2_PWM_CH2_Init(void)
{
    // Enable TIM2 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // PWM frequency = 10 kHz
    // 72 MHz / 72 = 1 MHz
    // 1 MHz / 100 = 10 kHz
    TIM2->PSC = 71;
    TIM2->ARR = 99;

    TIM2->CCR2 = 0;

    // CH2 as output
    TIM2->CCMR1 &= ~TIM_CCMR1_CC2S;

    // PWM mode 1 on CH2
    TIM2->CCMR1 &= ~TIM_CCMR1_OC2M;
    TIM2->CCMR1 |= (6 << 12);

    // Preload enable
    TIM2->CCMR1 |= TIM_CCMR1_OC2PE;

    // Enable CH2 output
    TIM2->CCER |= TIM_CCER_CC2E;

    // Auto-reload preload enable
    TIM2->CR1 |= TIM_CR1_ARPE;

    // Update event
    TIM2->EGR |= TIM_EGR_UG;

    // Start timer
    TIM2->CR1 |= TIM_CR1_CEN;
}

uint8_t button1_pressed(void)
{
    if ((GPIOB->IDR & GPIO_IDR_IDR0) == 0)
    {
        delay_ms(20);

        if ((GPIOB->IDR & GPIO_IDR_IDR0) == 0)
        {
            while ((GPIOB->IDR & GPIO_IDR_IDR0) == 0);
            delay_ms(20);
            return 1;
        }
    }
    return 0;
}

uint8_t button2_pressed(void)
{
    if ((GPIOB->IDR & GPIO_IDR_IDR1) == 0)
    {
        delay_ms(20);

        if ((GPIOB->IDR & GPIO_IDR_IDR1) == 0)
        {
            while ((GPIOB->IDR & GPIO_IDR_IDR1) == 0);
            delay_ms(20);
            return 1;
        }
    }
    return 0;
}

uint8_t button3_pressed(void)
{
    if ((GPIOB->IDR & GPIO_IDR_IDR10) == 0)
    {
        delay_ms(20);

        if ((GPIOB->IDR & GPIO_IDR_IDR10) == 0)
        {
            while ((GPIOB->IDR & GPIO_IDR_IDR10) == 0);
            delay_ms(20);
            return 1;
        }
    }
    return 0;
}

void delay_ms(uint32_t ms)
{
    uint32_t i, j;
    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < 8000; j++)
        {
            __NOP();
        }
    }
}