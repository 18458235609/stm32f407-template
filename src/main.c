/**
 * @file main.c
 * @brief STM32F407VG 应用程序入口
 */

#include "stm32f4xx.h"
#include "pindef.h"

void Reset_Handler(void);
int main(void);

void Default_Handler(void) {
    while (1) {
    }
}

void NMI_Handler(void)        __attribute__((alias("Default_Handler")));
void HardFault_Handler(void)   __attribute__((alias("Default_Handler")));
void MemManage_Handler(void)   __attribute__((alias("Default_Handler")));
void BusFault_Handler(void)    __attribute__((alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((alias("Default_Handler")));
void DebugMon_Handler(void)    __attribute__((alias("Default_Handler")));
void PendSV_Handler(void)      __attribute__((alias("Default_Handler")));

void SysTick_Handler(void) {
}

extern unsigned long _estack;
extern unsigned long _sidata, _sdata, _edata, _sbss, _ebss;

void (* const vectors[])(void) __attribute__((section(".isr_vector"))) = {
    (void (*)(void))0x20030000,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0,
    0,
    0,
    0,
    SVC_Handler,
    DebugMon_Handler,
    0,
    PendSV_Handler,
    SysTick_Handler,
};

#define RCC_GPIO_EN(port)  (RCC_AHB1ENR |= (1 << (port)))
#define GPIOEN_PORT_A  0
#define GPIOEN_PORT_F  5

static void delay_ms(volatile uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 10000; i++) {
        __asm__ volatile ("nop");
    }
}

void Reset_Handler(void) {
    unsigned long *src  = &_sidata;
    unsigned long *dest = &_sdata;
    while (dest < &_edata) {
        *dest++ = *src++;
    }

    dest = &_sbss;
    while (dest < &_ebss) {
        *dest++ = 0;
    }

    SystemInit();

    main();

    while (1) {
        __asm__ volatile ("nop");
    }
}

int main(void) {
    RCC_GPIO_EN(GPIOEN_PORT_F);
    RCC_GPIO_EN(GPIOEN_PORT_A);

    GPIOF_MODER = (GPIOF_MODER & ~0xC0000U) | 0x40000U;
    GPIOF_MODER = (GPIOF_MODER & ~0x300000U) | 0x100000U;

    GPIOF_ODR |= ((1U << 9) | (1U << 10));

    while (1) {
        GPIOF_ODR ^= (1U << 9) | (1U << 10);
        delay_ms(500);
    }
}
