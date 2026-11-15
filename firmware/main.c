/**
 * @file    main.c
 * @brief   Bare-metal Modbus RTU Slave implementation for STM32F103 (Blue Pill).
 * @details Implements a lightweight, zero-dependency Modbus RTU communication layer
 *          interfacing with a CODESYS PLC Master over USART1 (115200 8N1).
 *          Maps 9 16-bit Holding Registers (FC03 Read / FC16 Write) with a
 *          1000 ms fail-safe communication watchdog.
 *
 * @author  Turcu Alexandru-Costin
 * @date    2026
 */

#include <stdint.h>
#include "registers.h"

/* ========================================================================== */
/*                          GLOBAL DATA & BUFFERS                             */
/* ========================================================================== */

/**
 * @defgroup Modbus_Registers Register Map Definitions
 * @brief Unified memory-mapped registers accessible by Modbus Master.
 * @{
 */
uint16_t holding_registers[9] = {0};
/** @} */

/**
 * @defgroup Modbus_State Serial Communication Buffers & State Flags
 * @{
 */
volatile uint8_t rx_buf[32];
volatile uint8_t rx_idx = 0;
volatile uint8_t frame_ready = 0;

uint8_t tx_buf[32];
uint8_t tx_len = 0;

volatile uint32_t ms_ticks = 0;
uint32_t last_modbus_rx_ms = 0;
/** @} */

/* ========================================================================== */
/*                            FUNCTION PROTOTYPES                             */
/* ========================================================================== */

void System_Init(void);
void Hardware_IO_Init(void);
uint16_t ADC_Read_Channel(uint8_t channel);
uint16_t ADC_Read_Averaged(uint8_t channel);
void Control_Loop_Update(void);
void Modbus_Process_Frame(void);
uint16_t Modbus_CRC16(const uint8_t *buffer, uint16_t length);

/* ========================================================================== */
/*                                MAIN ENTRY                                  */
/* ========================================================================== */

int main(void)
{
    uint32_t last_heartbeat = 0;
    uint32_t last_io_update = 0;

    /* 1. Low-level core initialization (72 MHz System Clock & NVIC vectors) */
    System_Init();

    /* 2. Peripheral initialization (GPIOs, ADC1, TIM2 PWM, TIM3 T3.5 timer, USART1) */
    Hardware_IO_Init();

    /* 3. Infinite cooperative main loop */
    while (1)
    {
        /* Task A: Event-Driven Modbus Frame Parser */
        if (frame_ready)
        {
            Modbus_Process_Frame();
        }

        /* Task B: Periodic Industrial Control Loop (20 Hz / 50 ms Period) */
        if ((ms_ticks - last_io_update) >= 50U)
        {
            last_io_update = ms_ticks;
            Control_Loop_Update();
        }

        /* Task C: Visual Heartbeat Diagnostic LED (1 Hz Toggle / 500 ms) */
        if ((ms_ticks - last_heartbeat) >= 500U)
        {
            last_heartbeat = ms_ticks;

            if (GPIOC->ODR & (1U << 13))
            {
                GPIOC->BRR = (1U << 13);   /**< Turn LED ON */
            }
            else
            {
                GPIOC->BSRR = (1U << 13);  /**< Turn LED OFF */
            }
        }
    }
}

/* ========================================================================== */
/*                        INTERRUPT SERVICE ROUTINES                          */
/* ========================================================================== */

void SysTick_Handler(void)
{
    ms_ticks++;
}

void USART1_IRQHandler(void)
{
    uint32_t sr = USART1->SR;
    uint8_t  dr = (uint8_t)(USART1->DR & 0xFF);

    /* Framing, Noise, or Overrun Error Recovery */
    if (sr & ((1U << 3) | (1U << 1) | (1U << 0)))
    {
        rx_idx = 0;
        return;
    }

    if (sr & USART_SR_RXNE)
    {
        if (!frame_ready)
        {
            if (rx_idx < sizeof(rx_buf))
            {
                rx_buf[rx_idx++] = dr;
            }
            else
            {
                rx_idx = 0; // Buffer overflow protection
            }

            /* Reload T3.5 Inter-Frame Silence Timer (TIM3) */
            TIM3->CR1 &= ~TIM_CR1_CEN;
            TIM3->CNT = 0;
            TIM3->SR &= ~TIM_SR_UIF;
            TIM3->CR1 |= TIM_CR1_CEN;
        }
    }
}

void TIM3_IRQHandler(void)
{
    if (TIM3->SR & TIM_SR_UIF)
    {
        TIM3->SR &= ~TIM_SR_UIF;
        TIM3->CR1 &= ~TIM_CR1_CEN;

        if (rx_idx >= 4U)
        {
            frame_ready = 1; /**< Frame ready for parsing */
        }
        else
        {
            rx_idx = 0;      /**< Discard runt frames */
        }
    }
}

/* ========================================================================== */
/*                         PERIPHERAL INITIALIZATION                          */
/* ========================================================================== */

void System_Init(void)
{
    /* 1. Flash Latency: 2 Wait States + Prefetch Buffer */
    FLASH->ACR |= (1U << 4) | (2U << 0);

    /* 2. Enable HSE (8 MHz Crystal) */
    RCC->CR |= (1U << 16);
    uint32_t timeout = 0x10000U;
    while (!(RCC->CR & (1U << 17)) && --timeout);

    if (timeout > 0U)
    {
        RCC->CFGR &= ~((3U << 0) | (0xFU << 18) | (7U << 8) | (3U << 14));
        RCC->CFGR |=  (1U << 16) | (7U << 18)  | (4U << 8) | (2U << 14);

        RCC->CR |= (1U << 24); // Enable PLL
        while (!(RCC->CR & (1U << 25)));

        RCC->CFGR |= (2U << 0); // Select PLL as SYSCLK
        while ((RCC->CFGR & (3U << 2)) != (2U << 2));
    }

    /* 3. Enable Peripheral Clocks (AFIO, GPIOA..C, ADC1, USART1, TIM2..4) */
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN   |
                    RCC_APB2ENR_IOPAEN   |
                    RCC_APB2ENR_IOPBEN   |
                    RCC_APB2ENR_IOPCEN   |
                    RCC_APB2ENR_ADC1EN   |
                    RCC_APB2ENR_USART1EN;

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN   |
                    RCC_APB1ENR_TIM3EN   |
                    RCC_APB1ENR_TIM4EN;

    /* 4. Default Pin Remap Guard */
    AFIO->MAPR &= ~(1U << 2);

    /* 5. PC13 Status LED (Output 2 MHz Push-Pull) */
    GPIOC->CRH &= ~(0x00F00000U);
    GPIOC->CRH |=  (0x00200000U);

    /* 6. USART1 Pins: PA9 (AF PP 50MHz), PA10 (Input Floating) */
    GPIOA->CRH &= ~(0x00000FF0U);
    GPIOA->CRH |=  (0x000004B0U);

    /* 7. SysTick 1 ms Setup @ 72 MHz */
    SYSTICK->LOAD = 72000U - 1U;
    SYSTICK->VAL  = 0U;
    SYSTICK->CTRL = (1U << 2) | (1U << 1) | (1U << 0);

    /* 8. USART1 Baud Rate: 115200 Baud @ 72 MHz */
    USART1->BRR = 0x0271U;
    USART1->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE | USART_CR1_UE;

    /* 9. TIM3 T3.5 Gap Timer (~2.0 ms) */
    TIM3->PSC   = 7199U;
    TIM3->ARR   = 20U;
    TIM3->DIER |= TIM_DIER_UIE;

    /* 10. Clear Hardware Flags */
    TIM3->SR = 0U;
    USART1->SR &= ~(USART_SR_RXNE | (1U << 3));

    /* 11. Enable NVIC Vector Lines */
    NVIC->ISER[0] |= (1U << TIM3_IRQn);
    NVIC->ISER[1] |= (1U << (USART1_IRQn - 32U));

    __enable_irq();
}

void Hardware_IO_Init(void)
{
    /* 1. PA0..PA5 Analog Inputs, PA2 = TIM2_CH3 PWM */
    GPIOA->CRL &= ~0x00FFFFFFU;
    GPIOA->CRL |=  0x00000B00U;

    /* RS-485 DE/RE Pin Setup (PA8 Output Push-Pull 50 MHz) */
    GPIOA->CRH &= ~(0x0000000FU);
    GPIOA->CRH |=  (0x00000003U);
    GPIOA->BRR  =  RS485_DE_PIN; // Receive Mode Default

    /* 2. GPIOB Setup: PB8/PB9 PWM (0xB), PB12 Output (0x2), PB13 Input (0x8) */
    GPIOB->CRH &= ~((0xFU << 0) | (0xFU << 4) | (0xFU << 16) | (0xFU << 20));
    GPIOB->CRH |=  (0xBU << 0) | (0xBU << 4) | (0x2U << 16) | (0x8U << 20);
    GPIOB->BSRR =  (1U << 13);  // Activate E-Stop Internal Pull-Up

    /* 3. ADC1 Channel Sampling (239.5 Cycles) & Calibration */
    ADC1->SMPR2 |= (7U << 0) | (7U << 9) | (7U << 12) | (7U << 15);
    ADC1->SQR3   = 0U;
    ADC1->CR2   |= ADC_CR2_ADON;

    for (volatile uint32_t i = 0U; i < 1000U; i++);

    ADC1->CR2 |= ADC_CR2_RSTCAL;
    while (ADC1->CR2 & ADC_CR2_RSTCAL);

    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL);

    /* 4. TIM2 Motor PWM (1 kHz) */
    TIM2->PSC   = 719U;
    TIM2->ARR   = 100U;
    TIM2->CCMR2 |= (6U << 4);
    TIM2->CCER  |= (1U << 8);
    TIM2->CR1   |= TIM_CR1_CEN;

    /* 5. TIM4 Dual LED PWM (1 kHz) */
    TIM4->PSC   = 719U;
    TIM4->ARR   = 100U;
    TIM4->CCMR2 |= (6U << 4) | (6U << 12);
    TIM4->CCER  |= (1U << 8) | (1U << 12);
    TIM4->CR1   |= TIM_CR1_CEN;
}

uint16_t ADC_Read_Channel(uint8_t channel)
{
    ADC1->SQR3 = (channel & 0x1FU);
    ADC1->SQR1 = 0U;

    /* Trigger dummy conversion (SWSTART requires EXTSEL = 111b and EXTTRIG = 1) */
    ADC1->CR2 |= ADC_CR2_SWSTART | ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL_SWSTART;
    while (!(ADC1->SR & ADC_SR_EOC));
    (void)ADC1->DR; // Flush dummy sample

    /* Trigger real conversion */
    ADC1->CR2 |= ADC_CR2_SWSTART | ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL_SWSTART;
    while (!(ADC1->SR & ADC_SR_EOC));

    return (uint16_t)(ADC1->DR & 0x0FFFU);
}

uint16_t ADC_Read_Averaged(uint8_t channel)
{
    uint32_t sum = 0U;
    for (uint8_t i = 0U; i < 16U; i++)
    {
        sum += ADC_Read_Channel(channel);
    }
    return (uint16_t)(sum >> 4U);
}

void Control_Loop_Update(void)
{
    uint16_t val_potentiometer = ADC_Read_Averaged(0U);
    uint16_t val_ldr           = ADC_Read_Averaged(5U);
    uint16_t val_thermistor    = ADC_Read_Averaged(3U);
    uint16_t val_gas           = ADC_Read_Averaged(4U);
    uint16_t val_estop        = (GPIOB->IDR & (1U << 13)) ? 1U : 0U;

    holding_registers[0] = val_potentiometer;
    holding_registers[1] = val_ldr;
    holding_registers[2] = val_gas;
    holding_registers[3] = val_thermistor;
    holding_registers[4] = val_estop;

    /* Fail-Safe Communication Watchdog: Shut down actuators if PLC link is lost */
    if ((ms_ticks - last_modbus_rx_ms) >= MODBUS_WATCHDOG_TIMEOUT_MS)
    {
        holding_registers[5] = 0U; // Motor PWM = 0%
        holding_registers[6] = 0U; // LED 1 PWM = 0%
        holding_registers[7] = 0U; // LED 2 PWM = 0%
        holding_registers[8] = 0U; // Buzzer    = OFF
    }

    uint16_t motor_duty = holding_registers[5];
    if (motor_duty > 100U) motor_duty = 100U;
    TIM2->CCR3 = motor_duty;

    uint16_t led1_duty = holding_registers[6];
    if (led1_duty > 100U) led1_duty = 100U;
    TIM4->CCR3 = led1_duty;

    uint16_t led2_duty = holding_registers[7];
    if (led2_duty > 100U) led2_duty = 100U;
    TIM4->CCR4 = led2_duty;

    if (holding_registers[8] > 0U)
    {
        GPIOB->BSRR = (1U << 12);
    }
    else
    {
        GPIOB->BRR  = (1U << 12);
    }
}

/* ========================================================================== */
/*                             MODBUS RTU ENGINE                              */
/* ========================================================================== */

uint16_t Modbus_CRC16(const uint8_t *buffer, uint16_t length)
{
    uint16_t crc = 0xFFFFU;

    for (uint16_t pos = 0U; pos < length; pos++)
    {
        crc ^= (uint16_t)buffer[pos];

        for (uint8_t i = 8U; i > 0U; i--)
        {
            if (crc & 0x0001U)
            {
                crc >>= 1U;
                crc ^= 0xA001U;
            }
            else
            {
                crc >>= 1U;
            }
        }
    }
    return crc;
}

void Modbus_Process_Frame(void)
{
    uint16_t len = rx_idx;

    if (len >= 8U)
    {
        uint8_t slave_id = rx_buf[0];
        uint8_t function = rx_buf[1];

        if (slave_id == MODBUS_SLAVE_ADDR)
        {
            uint16_t crc_rec  = (uint16_t)rx_buf[len - 2U] | ((uint16_t)rx_buf[len - 1U] << 8U);
            uint16_t crc_calc = Modbus_CRC16((const uint8_t *)rx_buf, len - 2U);

            if (crc_rec == crc_calc)
            {
                last_modbus_rx_ms = ms_ticks;
                tx_len = 0U;

                /* Function Code 03: Read Holding Registers */
                if (function == MODBUS_FC_READ_HOLDING_REGS)
                {
                    uint16_t start_addr = ((uint16_t)rx_buf[2] << 8U) | rx_buf[3];
                    uint16_t reg_count  = ((uint16_t)rx_buf[4] << 8U) | rx_buf[5];

                    if ((reg_count < 1U) || (reg_count > 125U))
                    {
                        tx_buf[0] = slave_id;
                        tx_buf[1] = function | 0x80U;
                        tx_buf[2] = MODBUS_EX_ILLEGAL_VALUE; // Exception 0x03
                        tx_len    = 3U;
                    }
                    else if ((start_addr + reg_count) > 9U)
                    {
                        tx_buf[0] = slave_id;
                        tx_buf[1] = function | 0x80U;
                        tx_buf[2] = MODBUS_EX_ILLEGAL_ADDRESS; // Exception 0x02
                        tx_len    = 3U;
                    }
                    else
                    {
                        tx_buf[0] = slave_id;
                        tx_buf[1] = function;
                        tx_buf[2] = (uint8_t)(reg_count * 2U);
                        tx_len    = 3U;

                        for (uint16_t i = 0U; i < reg_count; i++)
                        {
                            uint16_t val = holding_registers[start_addr + i];
                            tx_buf[tx_len++] = (uint8_t)((val >> 8U) & 0xFFU);
                            tx_buf[tx_len++] = (uint8_t)(val & 0xFFU);
                        }
                    }
                }
                /* Function Code 16 (0x10): Write Multiple Registers */
                else if (function == MODBUS_FC_WRITE_MULTIPLE_REGS)
                {
                    uint16_t start_addr = ((uint16_t)rx_buf[2] << 8U) | rx_buf[3];
                    uint16_t reg_count  = ((uint16_t)rx_buf[4] << 8U) | rx_buf[5];
                    uint8_t  byte_count = rx_buf[6];

                    if ((byte_count != (reg_count * 2U)) || (reg_count == 0U) || (reg_count > 123U))
                    {
                        tx_buf[0] = slave_id;
                        tx_buf[1] = function | 0x80U;
                        tx_buf[2] = MODBUS_EX_ILLEGAL_VALUE; // Exception 0x03
                        tx_len    = 3U;
                    }
                    else if ((start_addr < 5U) || ((start_addr + reg_count) > 9U))
                    {
                        tx_buf[0] = slave_id;
                        tx_buf[1] = function | 0x80U;
                        tx_buf[2] = MODBUS_EX_ILLEGAL_ADDRESS; // Exception 0x02
                        tx_len    = 3U;
                    }
                    else
                    {
                        for (uint16_t i = 0U; i < reg_count; i++)
                        {
                            uint16_t val = ((uint16_t)rx_buf[7U + (i * 2U)] << 8U) |
                            rx_buf[8U + (i * 2U)];
                            holding_registers[start_addr + i] = val;
                        }

                        tx_buf[0] = slave_id;
                        tx_buf[1] = function;
                        tx_buf[2] = rx_buf[2];
                        tx_buf[3] = rx_buf[3];
                        tx_buf[4] = rx_buf[4];
                        tx_buf[5] = rx_buf[5];
                        tx_len    = 6U;
                    }
                }
                else
                {
                    tx_buf[0] = slave_id;
                    tx_buf[1] = function | 0x80U;
                    tx_buf[2] = MODBUS_EX_ILLEGAL_FUNCTION; // Exception 0x01
                    tx_len    = 3U;
                }

                /* Transmit Response Frame over RS-485 */
                if (tx_len > 0U)
                {
                    uint16_t tx_crc = Modbus_CRC16(tx_buf, tx_len);
                    tx_buf[tx_len++] = (uint8_t)(tx_crc & 0xFFU);
                    tx_buf[tx_len++] = (uint8_t)((tx_crc >> 8U) & 0xFFU);

                    /* Drive RS-485 Transceiver into TRANSMIT Mode (PA8 HIGH) */
                    GPIOA->BSRR = RS485_DE_PIN;

                    for (uint16_t i = 0U; i < tx_len; i++)
                    {
                        while (!(USART1->SR & USART_SR_TXE));
                        USART1->DR = tx_buf[i];
                    }
                    while (!(USART1->SR & USART_SR_TC));

                    /* Return RS-485 Transceiver to RECEIVE Mode (PA8 LOW) */
                    GPIOA->BRR = RS485_DE_PIN;
                }
            }
        }
    }

    rx_idx = 0U;
    frame_ready = 0U;
}
