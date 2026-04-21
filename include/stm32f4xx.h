#include <stdint.h>

/* STM32F4 外设寄存器定义（精简版） */
#define PERIPH_BASE       0x40000000U
#define APB1_BASE         (PERIPH_BASE + 0x00000000U)
#define APB2_BASE         (PERIPH_BASE + 0x00010000U)
#define AHB1_BASE         (PERIPH_BASE + 0x00020000U)

/* RCC */
#define RCC_BASE          (AHB1_BASE + 0x1000U)
#define RCC_CR            (*(volatile uint32_t *)(RCC_BASE + 0x00))
#define RCC_CFGR          (*(volatile uint32_t *)(RCC_BASE + 0x08))
#define RCC_AHB1ENR       (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_PLLCFGR       (*(volatile uint32_t *)(RCC_BASE + 0x04))

/* GPIO */
#define GPIOA_BASE        (AHB1_BASE + 0x0000U)
#define GPIOF_BASE        (AHB1_BASE + 0x1400U)
#define GPIOA_MODER       (*(volatile uint32_t *)(GPIOA_BASE + 0x00U))
#define GPIOA_ODR         (*(volatile uint32_t *)(GPIOA_BASE + 0x14U))
#define GPIOA_IDR         (*(volatile uint32_t *)(GPIOA_BASE + 0x10U))
#define GPIOF_MODER       (*(volatile uint32_t *)(GPIOF_BASE + 0x00U))
#define GPIOF_ODR         (*(volatile uint32_t *)(GPIOF_BASE + 0x14U))

/* Flash */
#define FLASH_BASE        0x40023C00U
#define FLASH_ACR         (*(volatile uint32_t *)(FLASH_BASE + 0x00))

/* SysTick */
#define SCB_BASE          0xE000ED00U
#define SCB_SHPR3         (*(volatile uint32_t *)(SCB_BASE + 0xD0U))
#define SYST_CSR          (*(volatile uint32_t *)0xE000E010U)
#define SYST_RVR          (*(volatile uint32_t *)0xE000E014U)

/* 常量 */
#define HSI_FREQ          16000000U   /* 16 MHz */
#define PLL_M             8
#define PLL_N             168
#define PLL_P             2           /* div 2 */
#define PLL_Q             7

/* 初始化 PLL -> 168 MHz */
static void SystemInit(void) {
    /* 使能 HSI */
    RCC_CR |= (1 << 0);
    while (!(RCC_CR & (1 << 1))) { }

    /* FLASH: 5 wait states, ART on */
    FLASH_ACR = 0x705U;

    /* PLL 参数 */
    RCC_PLLCFGR = (PLL_M << 0) | (PLL_N << 6) | ((PLL_P >> 1) << 16) | (1 << 22);

    /* AHB prescaler = 1 */
    RCC_CFGR &= ~(7U << 4);

    /* 使能 PLL */
    RCC_CR |= (1 << 24);
    while (!(RCC_CR & (1 << 25))) { }

    /* 切换系统时钟到 PLL */
    RCC_CFGR = (RCC_CFGR & ~3U) | 2U;
    while ((RCC_CFGR & 3U) != 2U) { }

    /* SysTick: 168 MHz / 168 = 1 ms */
    SYST_RVR = 168000 - 1;
    SYST_CSR = 0x07;
}
