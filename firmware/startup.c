/**
 * @file    startup.c
 * @brief   Production-Grade Bare-Metal Startup & Vector Table for STM32F103.
 * @author  Turcu Alexandru-Costin
 * @date    2026
 */

#include <stdint.h>

/* ========================================================================== */
/*                           LINKER SYMBOL DECLARATIONS                       */
/* ========================================================================== */

extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

/* Standard C library constructor initializer array hook */
extern void __libc_init_array(void);
extern int main(void);

/* Weak fallback for __libc_init_array when linking with -nostdlib */
void __attribute__((weak)) __libc_init_array(void) {}

/* ========================================================================== */
/*                       FUNCTION PROTOTYPES & ALIASES                        */
/* ========================================================================== */

void Reset_Handler(void);
void Default_Handler(void);
void HardFault_Handler_C(uint32_t *pulFaultArgs);

/* System Exception Handlers */
void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void); // Implemented with assembly naked wrapper below
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("Default_Handler")));

/* Optional System Initialization Hook */
void SystemInit(void)         __attribute__((weak));

/* Peripheral Interrupt Service Routines (STM32F103 Medium/High-Density) */
void WWDG_IRQHandler(void)               __attribute__((weak, alias("Default_Handler")));
void PVD_IRQHandler(void)                __attribute__((weak, alias("Default_Handler")));
void TAMPER_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void RTC_IRQHandler(void)                __attribute__((weak, alias("Default_Handler")));
void FLASH_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void RCC_IRQHandler(void)                __attribute__((weak, alias("Default_Handler")));
void EXTI0_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void EXTI1_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void EXTI2_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void EXTI3_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void EXTI4_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel1_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel2_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel3_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel4_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel5_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel6_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel7_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void ADC1_2_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void USB_HP_CAN1_TX_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void USB_LP_CAN1_RX0_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void CAN1_RX1_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void CAN1_SCE_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void EXTI9_5_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void TIM1_BRK_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void TIM1_UP_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void TIM1_TRG_COM_IRQHandler(void)       __attribute__((weak, alias("Default_Handler")));
void TIM1_CC_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void TIM2_IRQHandler(void)               __attribute__((weak, alias("Default_Handler")));
void TIM3_IRQHandler(void)               __attribute__((weak, alias("Default_Handler")));
void TIM4_IRQHandler(void)               __attribute__((weak, alias("Default_Handler")));
void I2C1_EV_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void I2C1_ER_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void I2C2_EV_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void I2C2_ER_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void SPI1_IRQHandler(void)               __attribute__((weak, alias("Default_Handler")));
void SPI2_IRQHandler(void)               __attribute__((weak, alias("Default_Handler")));
void USART1_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void USART3_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void EXTI15_10_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));
void RTC_Alarm_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));
void USBWakeUp_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));

/* ========================================================================== */
/*                         VECTOR TABLE DEFINITION                            */
/* ========================================================================== */

__attribute__((section(".isr_vector"), used, aligned(512)))
void (* const g_pfnVectors[])(void) = {
    (void (*)(void))(uintptr_t)&_estack,    /* Initial Stack Pointer (Casted cleanly via uintptr_t) */
    Reset_Handler,                          /* Reset Handler */
    NMI_Handler,                            /* NMI Handler */
    HardFault_Handler,                      /* Hard Fault Handler */
    MemManage_Handler,                      /* MPU Fault Handler */
    BusFault_Handler,                       /* Bus Fault Handler */
    UsageFault_Handler,                     /* Usage Fault Handler */
    0, 0, 0, 0,                             /* Reserved */
    SVC_Handler,                            /* SVCall Handler */
    DebugMon_Handler,                       /* Debug Monitor Handler */
    0,                                      /* Reserved */
    PendSV_Handler,                         /* PendSV Handler */
    SysTick_Handler,                        /* SysTick Handler */

    /* External Interrupts (STM32F103 Peripheral Vectors) */
    WWDG_IRQHandler,                        /* Window Watchdog */
    PVD_IRQHandler,                         /* PVD through EXTI Line detect */
    TAMPER_IRQHandler,                      /* Tamper */
    RTC_IRQHandler,                         /* RTC */
    FLASH_IRQHandler,                       /* Flash */
    RCC_IRQHandler,                         /* RCC */
    EXTI0_IRQHandler,                       /* EXTI Line 0 */
    EXTI1_IRQHandler,                       /* EXTI Line 1 */
    EXTI2_IRQHandler,                       /* EXTI Line 2 */
    EXTI3_IRQHandler,                       /* EXTI Line 3 */
    EXTI4_IRQHandler,                       /* EXTI Line 4 */
    DMA1_Channel1_IRQHandler,               /* DMA1 Channel 1 */
    DMA1_Channel2_IRQHandler,               /* DMA1 Channel 2 */
    DMA1_Channel3_IRQHandler,               /* DMA1 Channel 3 */
    DMA1_Channel4_IRQHandler,               /* DMA1 Channel 4 */
    DMA1_Channel5_IRQHandler,               /* DMA1 Channel 5 */
    DMA1_Channel6_IRQHandler,               /* DMA1 Channel 6 */
    DMA1_Channel7_IRQHandler,               /* DMA1 Channel 7 */
    ADC1_2_IRQHandler,                      /* ADC1 & ADC2 */
    USB_HP_CAN1_TX_IRQHandler,              /* USB High Priority / CAN1 TX */
    USB_LP_CAN1_RX0_IRQHandler,             /* USB Low Priority / CAN1 RX0 */
    CAN1_RX1_IRQHandler,                    /* CAN1 RX1 */
    CAN1_SCE_IRQHandler,                    /* CAN1 SCE */
    EXTI9_5_IRQHandler,                     /* EXTI Line 9..5 */
    TIM1_BRK_IRQHandler,                    /* TIM1 Break */
    TIM1_UP_IRQHandler,                     /* TIM1 Update */
    TIM1_TRG_COM_IRQHandler,                /* TIM1 Trigger & Commutation */
    TIM1_CC_IRQHandler,                     /* TIM1 Capture Compare */
    TIM2_IRQHandler,                        /* TIM2 */
    TIM3_IRQHandler,                        /* TIM3 */
    TIM4_IRQHandler,                        /* TIM4 */
    I2C1_EV_IRQHandler,                     /* I2C1 Event */
    I2C1_ER_IRQHandler,                     /* I2C1 Error */
    I2C2_EV_IRQHandler,                     /* I2C2 Event */
    I2C2_ER_IRQHandler,                     /* I2C2 Error */
    SPI1_IRQHandler,                        /* SPI1 */
    SPI2_IRQHandler,                        /* SPI2 */
    USART1_IRQHandler,                      /* USART1 */
    USART2_IRQHandler,                      /* USART2 */
    USART3_IRQHandler,                      /* USART3 */
    EXTI15_10_IRQHandler,                   /* EXTI Line 15..10 */
    RTC_Alarm_IRQHandler,                   /* RTC Alarm through EXTI Line */
    USBWakeUp_IRQHandler                    /* USB Wakeup through EXTI Line */
};

/* ========================================================================== */
/*                         SYSTEM RESET IMPLEMENTATION                        */
/* ========================================================================== */

void Reset_Handler(void)
{
    /* 1. Optional System Init Call */
    if (SystemInit != 0)
    {
        SystemInit();
    }

    /* 2. Fast 32-bit Word Copy for .data Section Initialization */
    uint32_t *pSrc = (uint32_t *)&_sidata;
    uint32_t *pDst = (uint32_t *)&_sdata;

    while (pDst < (uint32_t *)&_edata)
    {
        *pDst++ = *pSrc++;
    }

    /* 3. Fast 32-bit Word Zeroing for .bss Section */
    pDst = (uint32_t *)&_sbss;
    while (pDst < (uint32_t *)&_ebss)
    {
        *pDst++ = 0U;
    }

    /* 4. Execute C Static Initializers / Constructors */
    __libc_init_array();

    /* 5. Jump to Application Main */
    (void)main();

    /* 6. Production Safety Net: Software System Reset if main() returns */
    *(volatile uint32_t *)0xE000ED0C = (0x5FAU << 16) | (1U << 2);

    while (1)
    {
        __asm__ volatile ("nop");
    }
}

/* ========================================================================== */
/*                    HARDFAULT DIAGNOSTIC CAPTURE ENGINE                     */
/* ========================================================================== */

void __attribute__((naked)) HardFault_Handler(void)
{
    __asm__ volatile (
        " tst lr, #4                                    \n"
        " ite eq                                        \n"
        " mrseq r0, msp                                 \n"
        " mrsne r0, psp                                 \n"
        " ldr r1, [r0, #24]                             \n"
        " b HardFault_Handler_C                         \n"
    );
}

void HardFault_Handler_C(uint32_t *pulFaultArgs)
{
    volatile uint32_t r0  = pulFaultArgs[0];
    volatile uint32_t r1  = pulFaultArgs[1];
    volatile uint32_t r2  = pulFaultArgs[2];
    volatile uint32_t r3  = pulFaultArgs[3];
    volatile uint32_t r12 = pulFaultArgs[4];
    volatile uint32_t lr  = pulFaultArgs[5];
    volatile uint32_t pc  = pulFaultArgs[6];
    volatile uint32_t psr = pulFaultArgs[7];

    (void)r0; (void)r1; (void)r2; (void)r3; (void)r12; (void)lr; (void)pc; (void)psr;

    *(volatile uint32_t *)0xE000ED0C = (0x5FAU << 16) | (1U << 2);

    while (1);
}

void Default_Handler(void)
{
    while (1)
    {
        __asm__ volatile ("nop");
    }
}
