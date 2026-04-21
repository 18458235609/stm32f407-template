#include "stm32f4xx.h"
#include "pindef.h"

/* 前向声明 */
void Reset_Handler(void);
int main(void);

/* 中断处理函数统一用默认死循环 */
void Default_Handler(void) { while (1) {} }
void NMI_Handler(void)       __attribute__((alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((alias("Default_Handler")));

/* 外部链接器符号 */
extern unsigned long _sidata, _sdata, _edata, _sbss, _ebss;

/* 中断向量表 */
void (* const vectors[])(void) __attribute__((section(".isr_vector"))) = {
    [0]  = (void (*)(void))0x20030000,  /* 初始 SP */
    [1]  = Reset_Handler,
    [2]  = NMI_Handler,
    [3]  = HardFault_Handler,
    [4]  = MemManage_Handler,
    [5]  = BusFault_Handler,
    [6]  = UsageFault_Handler,
    [11] = SVC_Handler,
    [12] = DebugMon_Handler,
    [14] = PendSV_Handler,
};

/* GPIO 使能宏 */
#define RCC_GPIO_EN(port)  (RCC_AHB1ENR |= (1 << (port)))
#define GPIOEN_PORT_A  0
#define GPIOEN_PORT_F  5

/* GPIO 输出模式宏（mode: 0=输入, 1=输出, 2=复用, 3=模拟）*/
#define GPIO_SET_MODER(port_base, pin, mode) \
    do { *(volatile uint32_t *)((port_base) + 0x00U) = \
        (*(volatile uint32_t *)((port_base) + 0x00U) & ~(0x3U << ((pin)*2))) | \
        ((mode) & 0x3U) << ((pin)*2); \
    } while (0)

/* 软延时（168 MHz 主频）*/
static void delay_ms(volatile uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 1000; i++) {
        __asm__ volatile ("nop");
    }
}

void Reset_Handler(void) {
    unsigned long *src = &_sidata;
    unsigned long *dest = &_sdata;
    while (dest < &_edata) *dest++ = *src++;
    dest = &_sbss;
    while (dest < &_ebss) *dest++ = 0;
    SystemInit();
    main();
    while (1) {}
}

/* ===== 用户代码 ===== */
int main(void) {
    /* 使能 GPIOF（LED_RED=PF9, LED_GREEN=PF10）和 GPIOA（KEY_UP=PA0）*/
    RCC_GPIO_EN(GPIOEN_PORT_F);
    RCC_GPIO_EN(GPIOEN_PORT_A);

    /* PF9 (LED_RED)  -> 输出 */
    GPIOF_MODER = (GPIOF_MODER & ~0x30000U) | 0x10000U;   /* bit[19:18]=01 输出 */
    /* PF10 (LED_GREEN) -> 输出 */
    GPIOF_MODER = (GPIOF_MODER & ~0xC0000U) | 0x40000U;   /* bit[21:20]=01 输出 */

    /* PA0 (KEY_UP) -> 输入（默认），无需配置 */

    /* 初始化：两个 LED 灭 */
    GPIOF_ODR &= ~((1U << 9) | (1U << 10));

    while (1) {
        /* 读取 KEY_UP(PA0) 状态（按下=高电平 PA0=1，松手=低 PA0=0）*/
        volatile uint32_t key_up = (GPIOA_IDR & (1U << 0));

        if (key_up) {
            /* 按下：两个 LED 交替闪烁 */
            GPIOF_ODR ^= (1U << 9);          /* 翻转 PF9 (RED) */
            delay_ms(200);
            GPIOF_ODR &= ~(1U << 9);         /* 灭 RED */
            GPIOF_ODR ^= (1U << 10);          /* 亮 GREEN */
            delay_ms(200);
            GPIOF_ODR &= ~(1U << 10);         /* 灭 GREEN */
        } else {
            /* 松手：红绿同时闪烁 */
            GPIOF_ODR ^= (1U << 9) | (1U << 10);  /* 翻转两个 LED */
            delay_ms(300);
        }
    }
}
