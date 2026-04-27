/**
 * @file stm32f4xx.h
 * @brief STM32F4 最小寄存器定义 + 系统初始化
 */

#include <stdint.h>

#define PERIPH_BASE       0x40000000U
#define APB1_BASE         (PERIPH_BASE + 0x00000000U)
#define APB2_BASE         (PERIPH_BASE + 0x00010000U)
#define AHB1_BASE         (PERIPH_BASE + 0x00020000U)

#define RCC_BASE          (AHB1_BASE + 0x3800U)

#define RCC_CR            (*(volatile uint32_t *)(RCC_BASE + 0x00))
#define RCC_CFGR          (*(volatile uint32_t *)(RCC_BASE + 0x08))
#define RCC_PLLCFGR       (*(volatile uint32_t *)(RCC_BASE + 0x04))
#define RCC_AHB1ENR       (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB1ENR       (*(volatile uint32_t *)(RCC_BASE + 0x40U))
#define RCC_APB2ENR       (*(volatile uint32_t *)(RCC_BASE + 0x44U))

#define GPIOA_BASE        (AHB1_BASE + 0x0000U)
#define GPIOF_BASE        (AHB1_BASE + 0x1400U)

#define GPIOA_MODER       (*(volatile uint32_t *)(GPIOA_BASE + 0x00U))
#define GPIOA_ODR         (*(volatile uint32_t *)(GPIOA_BASE + 0x14U))
#define GPIOA_IDR         (*(volatile uint32_t *)(GPIOA_BASE + 0x10U))

#define GPIOF_MODER       (*(volatile uint32_t *)(GPIOF_BASE + 0x00U))
#define GPIOF_ODR         (*(volatile uint32_t *)(GPIOF_BASE + 0x14U))

#define FLASHIF_BASE      0x40023C00U
#define FLASH_ACR         (*(volatile uint32_t *)(FLASHIF_BASE + 0x00))

#define SCB_BASE          0xE000ED00U
#define SCB_VTOR          (*(volatile uint32_t *)(SCB_BASE + 0x08U))
#define SCB_CPACR         (*(volatile uint32_t *)(SCB_BASE + 0x88U))
#define SCB_SHPR3         (*(volatile uint32_t *)(SCB_BASE + 0xD0U))

#define SYST_CSR          (*(volatile uint32_t *)0xE000E010U)
#define SYST_RVR          (*(volatile uint32_t *)0xE000E014U)

#define PWR_BASE          (APB1_BASE + 0x7000U)
#define PWR_CR            (*(volatile uint32_t *)(PWR_BASE + 0x00U))

#define PLL_M             8
#define PLL_N             336
#define PLL_P             2
#define PLL_Q             7

static void SystemInit(void) {

    SCB_CPACR |= (3 << 20) | (3 << 22);

    RCC_CR |= (1 << 0);
    while (!(RCC_CR & (1 << 1))) { }

    RCC_CFGR = 0x00000000;
    RCC_CR &= 0xFEF6FFFF;
    RCC_PLLCFGR = 0x24003010;
    RCC_CR &= 0xFFFBFFFF;

    volatile uint32_t StartUpCounter = 0;
    volatile uint32_t HSEStatus = 0;

    RCC_CR |= (1 << 16);
    do {
        HSEStatus = RCC_CR & (1 << 17);
        StartUpCounter++;
    } while ((HSEStatus == 0) && (StartUpCounter != 0x0500));

    if ((RCC_CR & (1 << 17)) != 0) {
        HSEStatus = 0x01;
    } else {
        HSEStatus = 0x00;
    }

    if (HSEStatus == 0x01) {
        RCC_APB1ENR |= (1 << 28);
        PWR_CR |= (1 << 14);

        RCC_CFGR |= (0 << 4);
        RCC_CFGR |= (4 << 13);
        RCC_CFGR |= (5 << 10);

        RCC_PLLCFGR = (PLL_M << 0)
                     | (PLL_N << 6)
                     | (((PLL_P >> 1) - 1) << 16)
                     | (1 << 22)
                     | (PLL_Q << 24);

        RCC_CR |= (1 << 24);
        StartUpCounter = 0;
        while (((RCC_CR & (1 << 25)) == 0) && (StartUpCounter != 0x0500)) {
            StartUpCounter++;
        }

        if ((RCC_CR & (1 << 25)) != 0) {
            FLASH_ACR = (1 << 8) | (1 << 9) | (1 << 10) | 5;

            RCC_CFGR &= ~3U;
            RCC_CFGR |= 2U;
            StartUpCounter = 0;
            while (((RCC_CFGR & 0x0CU) != 0x08U) && (StartUpCounter != 0x0500)) {
                StartUpCounter++;
            }
        }
    }

    SCB_VTOR = 0x08000000U;

    SYST_RVR = 168000 - 1;
    SYST_CSR = 0x07;
}
