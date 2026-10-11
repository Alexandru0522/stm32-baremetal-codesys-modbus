/**
 * @file main.c
 * @brief STM32 Bare-Metal Modbus RTU Slave (USART1 @ 115200 baud, 8E1, 72MHz)
 */

#include <stdint.h>

#define SLAVE_ID 0x01

/* Registri Flash & RCC pentru configurare Ceas 72MHz */
#define FLASH_BASE      0x40022000UL
#define FLASH_ACR       (*(volatile uint32_t *)(FLASH_BASE + 0x00))

#define RCC_BASE        0x40021000UL
#define RCC_CR          (*(volatile uint32_t *)(RCC_BASE + 0x00))
#define RCC_CFGR        (*(volatile uint32_t *)(RCC_BASE + 0x04))
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18))

/* Registri GPIOC (PC13 - LED integrat) */
#define GPIOC_BASE      0x40011000UL
#define GPIOC_CRH       (*(volatile uint32_t *)(GPIOC_BASE + 0x04))
#define GPIOC_BSRR      (*(volatile uint32_t *)(GPIOC_BASE + 0x10))

/* Registri GPIOA (PA9 - TX, PA10 - RX) */
#define GPIOA_BASE      0x40010800UL
#define GPIOA_CRH       (*(volatile uint32_t *)(GPIOA_BASE + 0x04))
#define GPIOA_ODR       (*(volatile uint32_t *)(GPIOA_BASE + 0x0C))

/* Registri USART1 */
#define USART1_BASE     0x40013800UL
#define USART1_SR       (*(volatile uint32_t *)(USART1_BASE + 0x00))
#define USART1_DR       (*(volatile uint32_t *)(USART1_BASE + 0x04))
#define USART1_BRR      (*(volatile uint32_t *)(USART1_BASE + 0x08))
#define USART1_CR1      (*(volatile uint32_t *)(USART1_BASE + 0x0C))

static uint16_t holding_registers[10] = {0};

#define RX_BUF_SIZE 64
static uint8_t rx_buf[RX_BUF_SIZE];
static uint16_t rx_idx = 0;

/* Configurare Quartz Extern (HSE 8MHz -> PLL x9 = 72MHz) */
static void system_clock_72mhz(void) {
    RCC_CR |= (1 << 16); /* Pornire HSE */
    while (!(RCC_CR & (1 << 17))){
    } /* Așteptare HSE stabil */

        FLASH_ACR |= 0x02; /* Latenta Flash pentru 72MHz */

        /* PLL: HSE * 9 = 72MHz */
        RCC_CFGR &= ~((0x0F << 18) | (1 << 16));
    RCC_CFGR |= (1 << 16) | (0x07 << 18);

    RCC_CR |= (1 << 24); /* Pornire PLL */
    while (!(RCC_CR & (1 << 25))){
    } /* Așteptare PLL stabil */

        RCC_CFGR &= ~(0x03);
    RCC_CFGR |= 0x02; /* Mutare SYSCLK pe PLL */
    while ((RCC_CFGR & 0x0C) != 0x08);
}

static uint16_t modbus_crc16(const uint8_t *buffer, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buffer[pos];
        for (int i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

static void usart1_init(void) {
    RCC_APB2ENR |= (1 << 0) | (1 << 2) | (1 << 14); /* AFIO, GPIOA, USART1 */

    /* PA9 (TX): Alternate Function Push-Pull 50MHz */
    GPIOA_CRH &= ~(0x0F << 4);
    GPIOA_CRH |= (0x0B << 4);

    /* PA10 (RX): Input Pull-Up */
    GPIOA_CRH &= ~(0x0F << 8);
    GPIOA_CRH |= (0x08 << 8);
    GPIOA_ODR |= (1 << 10);

    /* 115200 baud la exact 72MHz */
    USART1_BRR = 0x271;

    /* Frame 8E1: 8 biți date + Paritate Even */
    USART1_CR1 |= (1 << 13) | (1 << 12) | (1 << 10) | (1 << 3) | (1 << 2);
}

static void usart1_send_char(uint8_t c) {
    while (!(USART1_SR & (1 << 7))){
    } /* Așteptare buffer gol (TXE) */
        USART1_DR = c;
}

static void usart1_send_buf(const uint8_t *buf, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        usart1_send_char(buf[i]);
    }
    while (!(USART1_SR & (1 << 6))); /* Așteptare finalizare transmisie (TC) */
}

static void process_modbus_frame(uint16_t len) {
    if (len < 8) return;

    uint8_t slave_id = rx_buf[0];
    uint8_t func_code = rx_buf[1];

    if (slave_id != SLAVE_ID) return;

    uint16_t rx_crc = (rx_buf[len - 1] << 8) | rx_buf[len - 2];
    uint16_t calc_crc = modbus_crc16(rx_buf, len - 2);

    if (rx_crc != calc_crc) return; /* CRC incorect */

        /* Funcția 0x03: Read Holding Registers */
        if (func_code == 0x03) {
            uint16_t reg_addr = (rx_buf[2] << 8) | rx_buf[3];
            uint16_t reg_count = (rx_buf[4] << 8) | rx_buf[5];

            if (reg_addr < 10 && (reg_addr + reg_count) <= 10) {
                uint8_t tx_buf[64];
                tx_buf[0] = SLAVE_ID;
                tx_buf[1] = 0x03;
                tx_buf[2] = reg_count * 2;

                uint16_t idx = 3;
                for (uint16_t i = 0; i < reg_count; i++) {
                    uint16_t val = holding_registers[reg_addr + i];
                    tx_buf[idx++] = (val >> 8) & 0xFF;
                    tx_buf[idx++] = val & 0xFF;
                }

                uint16_t crc = modbus_crc16(tx_buf, idx);
                tx_buf[idx++] = crc & 0xFF;
                tx_buf[idx++] = (crc >> 8) & 0xFF;

                usart1_send_buf(tx_buf, idx);
            }
        }
        /* Funcția 0x06: Write Single Register */
        else if (func_code == 0x06) {
            uint16_t reg_addr = (rx_buf[2] << 8) | rx_buf[3];
            uint16_t reg_val  = (rx_buf[4] << 8) | rx_buf[5];

            if (reg_addr < 10) {
                holding_registers[reg_addr] = reg_val;

                /* Control LED PC13 de la Registrul 0 (0 = OFF, 1 = ON) */
                if (reg_addr == 0) {
                    if (reg_val == 1) {
                        GPIOC_BSRR = (1 << 29); /* Aprinde LED (Active Low) */
                    } else {
                        GPIOC_BSRR = (1 << 13); /* Stinge LED */
                    }
                }

                uint16_t crc = modbus_crc16(rx_buf, 6);
                rx_buf[6] = crc & 0xFF;
                rx_buf[7] = (crc >> 8) & 0xFF;
                usart1_send_buf(rx_buf, 8);
            }
        }
}

static void delay_us(uint32_t us) {
    uint32_t count = us * 12;
    while (count--) {
        __asm("nop");
    }
}

int main(void) {
    /* 1. Setare Frecvență Procesor la 72MHz */
    system_clock_72mhz();

    /* 2. Configurare Pin PC13 (LED) */
    RCC_APB2ENR |= (1 << 4);
    GPIOC_CRH &= ~(0x0F << 20);
    GPIOC_CRH |= (0x02 << 20);
    GPIOC_BSRR = (1 << 13); /* LED stins inițial */

    /* 3. Configurare USART1 */
    usart1_init();

    uint32_t idle_time_us = 0;

    while (1) {
        if (USART1_SR & (1 << 5)) { /* Bit RXNE */
            uint8_t byte = (uint8_t)(USART1_DR & 0xFF);
            if (rx_idx < RX_BUF_SIZE) {
                rx_buf[rx_idx++] = byte;
            }
            idle_time_us = 0;
        } else {
            if (rx_idx > 0) {
                delay_us(10);
                idle_time_us += 10;

                /* Pauză t3.5 Modbus RTU (~3.5 ms la 115200) */
                if (idle_time_us > 3500) {
                    process_modbus_frame(rx_idx);
                    rx_idx = 0;
                    idle_time_us = 0;
                }
            }
        }
    }

    return 0;
}
