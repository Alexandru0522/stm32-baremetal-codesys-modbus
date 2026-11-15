/**
 * @file    registers.h
 * @brief   Bare-metal peripheral header and Modbus definitions for STM32F103.
 * @details Contains low-level hardware register structures, base memory addresses,
 *          peripheral bitmasks, NVIC IRQ assignments, and Modbus RTU protocol constants.
 *
 * @author  Turcu Alexandru-Costin
 * @date    2026
 */

#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>

/* ========================================================================== */
/*                          MODBUS PROTOCOL DEFINITIONS                       */
/* ========================================================================== */

/** @defgroup Modbus_Constants Protocol Function & Exception Codes */
/**@{*/
#define MODBUS_SLAVE_ADDR             0x01    /**< Target Slave Address */
#define MODBUS_WATCHDOG_TIMEOUT_MS    1000U   /**< Fail-safe actuator cutoff timeout (ms) */

#define MODBUS_FC_READ_HOLDING_REGS   0x03    /**< Function Code 03: Read Holding Registers */
#define MODBUS_FC_WRITE_MULTIPLE_REGS 0x10    /**< Function Code 16: Write Multiple Registers */

#define MODBUS_EX_ILLEGAL_FUNCTION    0x01    /**< Exception Code 01: Function code not supported */
#define MODBUS_EX_ILLEGAL_ADDRESS     0x02    /**< Exception Code 02: Register address out of range */
#define MODBUS_EX_ILLEGAL_VALUE       0x03    /**< Exception Code 03: Data payload value invalid */
#define MODBUS_EX_SLAVE_FAILURE       0x04    /**< Exception Code 04: Unrecoverable execution error */
/**@}*/

/* ========================================================================== */
/*                          NVIC INTERRUPT POSITIONS                          */
/* ========================================================================== */

/** @defgroup NVIC_IRQ_Numbers STM32F103 Interrupt Vectors */
/**@{*/
#define TIM3_IRQn                     29      /**< TIM3 Global Interrupt (T3.5 Timer) */
#define USART1_IRQn                   37      /**< USART1 Global Interrupt */
/**@}*/

/* ========================================================================== */
/*                            PERIPHERAL STRUCTS                              */
/* ========================================================================== */

typedef struct {
    volatile uint32_t ACR;
    volatile uint32_t KEYR;
    volatile uint32_t OPTKEYR;
    volatile uint32_t SR;
    volatile uint32_t CR;
    volatile uint32_t AR;
    volatile uint32_t RESERVED;
    volatile uint32_t OBR;
    volatile uint32_t WRPR;
} FLASH_TypeDef;

typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_TypeDef;

typedef struct {
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
} RCC_TypeDef;

typedef struct {
    volatile uint32_t EVCR;
    volatile uint32_t MAPR;
    volatile uint32_t EXTICR[4];
    volatile uint32_t RESERVED0;
    volatile uint32_t MAPR2;
} AFIO_TypeDef;

typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} USART_TypeDef;

typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_TypeDef;

typedef struct {
    volatile uint32_t ISER[8];
    volatile uint32_t RESERVED0[24];
    volatile uint32_t ICER[8];
    volatile uint32_t RESERVED1[24];
    volatile uint32_t ISPR[8];
    volatile uint32_t RESERVED2[24];
    volatile uint32_t ICPR[8];
    volatile uint32_t RESERVED3[24];
    volatile uint32_t IABR[8];
    volatile uint32_t RESERVED4[56];
    volatile uint8_t  IP[240];
} NVIC_TypeDef;

typedef struct {
    volatile uint32_t SR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMPR1;
    volatile uint32_t SMPR2;
    volatile uint32_t JOFR1;
    volatile uint32_t JOFR2;
    volatile uint32_t JOFR3;
    volatile uint32_t JOFR4;
    volatile uint32_t HTR;
    volatile uint32_t LTR;
    volatile uint32_t SQR1;
    volatile uint32_t SQR2;
    volatile uint32_t SQR3;
    volatile uint32_t JSQR;
    volatile uint32_t JDR1;
    volatile uint32_t JDR2;
    volatile uint32_t JDR3;
    volatile uint32_t JDR4;
    volatile uint32_t DR;
} ADC_TypeDef;

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    volatile uint32_t RCR;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
    volatile uint32_t BDTR;
    volatile uint32_t DCR;
    volatile uint32_t DMAR;
} TIM_TypeDef;

/* ========================================================================== */
/*                          BASE ADDRESSES & POINTERS                         */
/* ========================================================================== */

#define FLASH_BASE      (0x40022000UL)
#define RCC_BASE        (0x40021000UL)
#define AFIO_BASE       (0x40010000UL)
#define GPIOA_BASE      (0x40010800UL)
#define GPIOB_BASE      (0x40010C00UL)
#define GPIOC_BASE      (0x40011000UL)
#define USART1_BASE     (0x40013800UL)
#define ADC1_BASE       (0x40012400UL)
#define TIM2_BASE       (0x40000000UL)
#define TIM3_BASE       (0x40000400UL)
#define TIM4_BASE       (0x40000800UL)

#define SYSTICK_BASE    (0xE000E010UL)
#define NVIC_BASE       (0xE000E100UL)

#define FLASH           ((FLASH_TypeDef   *) FLASH_BASE)
#define RCC             ((RCC_TypeDef     *) RCC_BASE)
#define AFIO            ((AFIO_TypeDef    *) AFIO_BASE)
#define GPIOA           ((GPIO_TypeDef    *) GPIOA_BASE)
#define GPIOB           ((GPIO_TypeDef    *) GPIOB_BASE)
#define GPIOC           ((GPIO_TypeDef    *) GPIOC_BASE)
#define USART1          ((USART_TypeDef   *) USART1_BASE)
#define ADC1            ((ADC_TypeDef     *) ADC1_BASE)
#define TIM2            ((TIM_TypeDef     *) TIM2_BASE)
#define TIM3            ((TIM_TypeDef     *) TIM3_BASE)
#define TIM4            ((TIM_TypeDef     *) TIM4_BASE)

#define SYSTICK         ((SysTick_TypeDef *) SYSTICK_BASE)
#define NVIC            ((NVIC_TypeDef    *) NVIC_BASE)

/* ========================================================================== */
/*                          HARDWARE BIT DEFINITIONS                          */
/* ========================================================================== */

/** @defgroup Register_Bitmasks Essential Register Flags */
/**@{*/

/* RCC APB2 Clock Enables */
#define RCC_APB2ENR_AFIOEN            (1U << 0)   /**< Alternate Function I/O Clock Enable */
#define RCC_APB2ENR_IOPAEN            (1U << 2)   /**< GPIO Port A Clock Enable */
#define RCC_APB2ENR_IOPBEN            (1U << 3)   /**< GPIO Port B Clock Enable */
#define RCC_APB2ENR_IOPCEN            (1U << 4)   /**< GPIO Port C Clock Enable */
#define RCC_APB2ENR_ADC1EN            (1U << 9)   /**< ADC1 Interface Clock Enable */
#define RCC_APB2ENR_USART1EN          (1U << 14)  /**< USART1 Clock Enable */

/* RCC APB1 Clock Enables */
#define RCC_APB1ENR_TIM2EN            (1U << 0)   /**< Timer 2 Clock Enable */
#define RCC_APB1ENR_TIM3EN            (1U << 1)   /**< Timer 3 Clock Enable */
#define RCC_APB1ENR_TIM4EN            (1U << 2)   /**< Timer 4 Clock Enable */

/* USART Bitmasks */
#define USART_SR_RXNE                 (1U << 5)   /**< Read Data Register Not Empty */
#define USART_SR_TC                   (1U << 6)   /**< Transmission Complete */
#define USART_SR_TXE                  (1U << 7)   /**< Transmit Data Register Empty */

#define USART_CR1_RE                  (1U << 2)   /**< Receiver Enable */
#define USART_CR1_TE                  (1U << 3)   /**< Transmitter Enable */
#define USART_CR1_RXNEIE              (1U << 5)   /**< RXNE Interrupt Enable */
#define USART_CR1_UE                  (1U << 13)  /**< USART Enable */

/* ADC Bitmasks */
#define ADC_SR_EOC                    (1U << 1)   /**< End of Conversion Flag */
#define ADC_CR2_ADON                  (1U << 0)   /**< Power ON ADC */
#define ADC_CR2_CAL                   (1U << 2)   /**< Calibration Start */
#define ADC_CR2_RSTCAL                (1U << 3)   /**< Reset Calibration */
#define ADC_CR2_EXTSEL_SWSTART        (7U << 17)  /**< Select SWSTART as External Trigger (111b) */
#define ADC_CR2_EXTTRIG               (1U << 20)  /**< External Trigger Conversion Mode */
#define ADC_CR2_SWSTART               (1U << 22)  /**< Software Start Conversion Trigger */

/* Timer Bitmasks */
#define TIM_CR1_CEN                   (1U << 0)   /**< Counter Enable */
#define TIM_CR1_OPM                   (1U << 3)   /**< One-Pulse Mode */
#define TIM_SR_UIF                    (1U << 0)   /**< Update Interrupt Flag */
#define TIM_DIER_UIE                  (1U << 0)   /**< Update Interrupt Enable */

/* RS-485 Direction Control Pin */
#define RS485_DE_PIN                  (1U << 8)   /**< PA8: RS-485 DE/RE Transceiver Direction Control */

/**@}*/

/* ========================================================================== */
/*                          INLINE ASSEMBLY HELPERS                           */
/* ========================================================================== */

static inline void __attribute__((always_inline)) __disable_irq(void) {
    __asm__ volatile ("cpsid i" : : : "memory");
}

static inline void __attribute__((always_inline)) __enable_irq(void) {
    __asm__ volatile ("cpsie i" : : : "memory");
}

#endif /* REGISTERS_H */
