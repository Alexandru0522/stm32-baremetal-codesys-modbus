/**
 * @file    startup_stm32f103c8tx.c
 * @brief   STM32F103C8Tx Microcontroller Vector Table and Startup Code.
 * @note    Target: ARM Cortex-M3, bare-metal environment.
 */

#include <stdint.h>

/* ========================================================================== */
/*                   SIMBOLURI EXPORTATE DE LINKER SCRIPT                     */
/* ========================================================================== */

extern uint32_t _estack;        /* Punctul de top al stivei (RAM End)        */
extern uint32_t _sidata;        /* Adresa de start a .data în FLASH          */
extern uint32_t _sdata;         /* Adresa de start a .data în RAM            */
extern uint32_t _edata;         /* Adresa de sfârșit a .data în RAM          */
extern uint32_t _sbss;          /* Adresa de start a .bss în RAM             */
extern uint32_t _ebss;          /* Adresa de sfârșit a .bss în RAM           */

/* ========================================================================== */
/*                       PROTOTIPURI DE FUNCȚII                               */
/* ========================================================================== */

extern int main(void);
extern void SystemInit(void) __attribute__((weak));

void Reset_Handler(void);
void Default_Handler(void);

/* Prototipuri Excepții Cortex-M3 Core (Weak Aliases) */
void NMI_Handler(void)          __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)    __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)    __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)          __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)       __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)      __attribute__((weak, alias("Default_Handler")));

/* Prototipuri Întreruperi Periferice STM32F103 (Subset utilizat frecvent) */
void WWDG_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void PVD_IRQHandler(void)       __attribute__((weak, alias("Default_Handler")));
void TAMPER_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void RTC_IRQHandler(void)       __attribute__((weak, alias("Default_Handler")));
void FLASH_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void RCC_IRQHandler(void)       __attribute__((weak, alias("Default_Handler")));
void EXTI0_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void EXTI1_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void EXTI2_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void EXTI3_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void EXTI4_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void ADC1_2_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void EXTI9_5_IRQHandler(void)   __attribute__((weak, alias("Default_Handler")));
void TIM1_UP_IRQHandler(void)   __attribute__((weak, alias("Default_Handler")));
void TIM2_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void TIM3_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void TIM4_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void I2C1_EV_IRQHandler(void)   __attribute__((weak, alias("Default_Handler")));
void SPI1_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void USART1_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void USART3_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void EXTI15_10_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

/* ========================================================================== */
/*                       TABELUL DE VECTORI DE ÎNTRERUPERI                   */
/* ========================================================================== */

__attribute__((section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
    (void (*)(void))&_estack,    /* 0x00: Stack Pointer Inițial               */
    Reset_Handler,               /* 0x04: Reset Handler                       */
    NMI_Handler,                 /* 0x08: NMI Handler                         */
    HardFault_Handler,           /* 0x0C: Hard Fault Handler                  */
    MemManage_Handler,           /* 0x10: MPU Fault Handler                   */
    BusFault_Handler,            /* 0x14: Bus Fault Handler                   */
    UsageFault_Handler,          /* 0x18: Usage Fault Handler                 */
    0, 0, 0, 0,                  /* 0x1C - 0x28: Rezervat                      */
    SVC_Handler,                 /* 0x2C: SVCall Handler                      */
    DebugMon_Handler,            /* 0x30: Debug Monitor Handler               */
    0,                           /* 0x34: Rezervat                            */
    PendSV_Handler,              /* 0x38: PendSV Handler                      */
    SysTick_Handler,             /* 0x3C: SysTick Handler                     */

    /* External Interrupts (STM32F103 Peripheral Vectors) */
    WWDG_IRQHandler,             /* 0x40: Window Watchdog                     */
    PVD_IRQHandler,              /* 0x44: PVD through EXTI Line               */
    TAMPER_IRQHandler,           /* 0x48: Tamper                              */
    RTC_IRQHandler,              /* 0x4C: RTC Global                          */
    FLASH_IRQHandler,            /* 0x50: Flash Global                        */
    RCC_IRQHandler,              /* 0x54: RCC Global                          */
    EXTI0_IRQHandler,            /* 0x58: EXTI Line 0                         */
    EXTI1_IRQHandler,            /* 0x5C: EXTI Line 1                         */
    EXTI2_IRQHandler,            /* 0x60: EXTI Line 2                         */
    EXTI3_IRQHandler,            /* 0x64: EXTI Line 3                         */
    EXTI4_IRQHandler,            /* 0x68: EXTI Line 4                         */
    DMA1_Channel1_IRQHandler,    /* 0x6C: DMA1 Channel 1                      */
    0, 0, 0, 0, 0, 0,            /* 0x70 - 0x84: DMA Channels / Other         */
    ADC1_2_IRQHandler,           /* 0x88: ADC1 and ADC2                       */
    0, 0, 0, 0,                  /* 0x8C - 0x98: USB High / CAN               */
    EXTI9_5_IRQHandler,          /* 0x9C: EXTI Line [9:5]                     */
    TIM1_UP_IRQHandler,          /* 0xA0: TIM1 Update                         */
    0, 0,                        /* 0xA4 - 0xA8: TIM1 Trigger / CC            */
    TIM2_IRQHandler,             /* 0xAC: TIM2 Global                         */
    TIM3_IRQHandler,             /* 0xB0: TIM3 Global                         */
    TIM4_IRQHandler,             /* 0xB4: TIM4 Global                         */
    I2C1_EV_IRQHandler,          /* 0xB8: I2C1 Event                          */
    0, 0, 0,                     /* 0xBC - 0xC4: I2C1 Error / SPI             */
    SPI1_IRQHandler,             /* 0xC8: SPI1 Global                         */
    0,                           /* 0xCC: SPI2 Global                         */
    USART1_IRQHandler,           /* 0xD0: USART1 Global                       */
    USART2_IRQHandler,           /* 0xD4: USART2 Global                       */
    USART3_IRQHandler,           /* 0xD8: USART3 Global                       */
    EXTI15_10_IRQHandler         /* 0xDC: EXTI Line [15:10]                   */
};

/* ========================================================================== */
/*                       IMPLEMENTARE RESET HANDLER                           */
/* ========================================================================== */

/**
 * @brief  Punctul de intrare primar la boot hardware.
 * @details Inițializează secțiunile memoriei RAM (.data și .bss) și
 *          transferă controlul către codul de aplicație (main).
 */
void Reset_Handler(void)
{
    /* 1. Copierea secțiunii .data din FLASH în RAM */
    uint32_t size = (uintptr_t)&_edata - (uintptr_t)&_sdata;
    uint8_t *pDst = (uint8_t *)&_sdata;
    uint8_t *pSrc = (uint8_t *)&_sidata;

    for (uint32_t i = 0; i < size; i++)
    {
        *pDst++ = *pSrc++;
    }

    /* 2. Inițializarea secțiunii .bss în RAM cu valoarea 0 */
    size = (uintptr_t)&_ebss - (uintptr_t)&_sbss;
    pDst = (uint8_t *)&_sbss;

    for (uint32_t i = 0; i < size; i++)
    {
        *pDst++ = 0;
    }

    /* 3. Apel de sistem opțional pentru configurarea Clocks/PLL (dacă există) */
    if (SystemInit)
    {
        SystemInit();
    }

    /* 4. Trecerea la execuția aplicației principale */
    main();

    /* 5. Trap de siguranță în caz de ieșire neprevăzută din main() */
    while (1)
    {
        __asm__("NOP");
    }
}

/**
 * @brief  Handler implicit pentru excepții neasociate.
 */
void Default_Handler(void)
{
    while (1)
    {
        __asm__("NOP");
    }
}
