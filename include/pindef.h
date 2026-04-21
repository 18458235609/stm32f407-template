/**
 * @file pindef.h
 * @brief 神舟号开发板 IO 引脚分配表
 *
 * 芯片：STM32F407ZGT6
 * 封装：LQFP144
 *
 * 列说明：
 *   GPIO   - STM32F407ZGT6 的引脚名（如 PA0、PB3）
 *   PIN    - 对应引脚编号（芯片引脚号，非GPIO编号）
 *   AF     - 默认复用功能
 *   IND    - Y=可完全独立（拔跳线/禁片选后可悬空）N=直连外设，无法隔离
 *   DESC   - 连接关系与使用提示
 *
 * 使用提示：
 *   - IND=N 的引脚不建议做普通IO，除非禁用对应外设
 *   - PC14/PC15 为 RTC 晶振专用，禁止挪作他用
 *   - Pxx 跳线帽用于选择外设连接，拔掉即可隔离
 */

#ifndef __PINDET_H__
#define __PINDET_H__

/* ================================================================
 * PA 端口（Port A）
 * ================================================================ */

/** PA0 - PIN34 - WK_UP / 独立: Y
 *   按键KEY_UP，可做待机唤醒脚(WKUP)
 *   只要KEY_UP不按下，该IO完全独立 */
#define PIN_PA0   34, 0, 0, 1   /* WK_UP, Y */

/** PA1 - PIN35 - RMII_REF_CLK / 独立: N
 *   接LAN8720的REFCLKO脚（50M时钟，一直有）
 *   该IO直接接LAN8720的REFCLKO引脚，不建议做普通IO */
#define PIN_PA1   35, 1, 0, 0   /* RMII_REF_CLK, N */

/** PA2 - PIN36 - USART2_TX / RS485_RX / ETH_MDIO / 独立: N
 *   1，RS232串口2(COM2)RX脚(P9设置)
 *   2，RS485 RX脚(P9设置)
 *   3，LAN8720的MDIO脚
 *   该IO通过P9选择连接RS232还是RS485，并同时连接了LAN8720的MDIO脚 */
#define PIN_PA2   36, 2, 0, 0   /* USART2_TX/RS485_RX/ETH_MDIO, N */

/** PA3 - PIN37 - USART2_RX / RS485_TX / PWM_DAC / 独立: N
 *   1，RS232串口2(COM2)TX脚(P9设置)
 *   2，RS485 TX脚(P9设置)
 *   3，PWM_DAC输出脚
 *   通过P9选择RS232/RS485，并同时连接了PWM_DAC */
#define PIN_PA3   37, 3, 0, 0   /* USART2_RX/RS485_TX/PWM_DAC, N */

/** PA4 - PIN40 - STM_DAC / DCMI_HREF / 独立: Y
 *   1，DAC_OUT1输出脚
 *   2，OLED/CAMERA接口的HREF引脚
 *   不插外设时完全独立 */
#define PIN_PA4   40, 4, 0, 1   /* STM_DAC/DCMI_HREF, Y */

/** PA5 - PIN41 - STM_ADC / TPAD / 独立: Y
 *   ADC输入引脚，同时做TPAD检测脚
 *   拔掉P12跳线帽则完全独立 */
#define PIN_PA5   41, 5, 0, 1   /* STM_ADC/TPAD, Y */

/** PA6 - PIN42 - DCMI_PCLK / 独立: Y
 *   OLED/CAMERA接口的PCLK脚
 *   不使用时完全独立 */
#define PIN_PA6   42, 6, 0, 1   /* DCMI_PCLK, Y */

/** PA7 - PIN43 - RMII_CRS_DV / 独立: N
 *   接LAN8720的CRS_DV脚
 *   LAN8720复位状态时可做普通IO */
#define PIN_PA7   43, 7, 0, 0   /* RMII_CRS_DV, N */

/** PA8 - PIN100 - DCMI_XCLK / REMOTE_IN / 独立: N
 *   1，OLED/CAMERA接口的XCLK脚
 *   2，接HS0038红外接收头（有4.7K上拉）
 *   不建议做普通IO */
#define PIN_PA8   100, 8, 0, 0  /* DCMI_XCLK/REMOTE_IN, N */

/** PA9 - PIN101 - USART1_TX / 独立: Y
 *   串口1 TX脚，默认连接CH340的RX（P6设置）
 *   去掉P6跳线帽则完全独立 */
#define PIN_PA9   101, 9, 0, 1  /* USART1_TX, Y */

/** PA10 - PIN102 - USART1_RX / 独立: Y
 *   串口1 RX脚，默认连接CH340的TX（P6设置）
 *   去掉P6跳线帽则完全独立 */
#define PIN_PA10  102, 10, 0, 1 /* USART1_RX, Y */

/** PA11 - PIN103 - USB_D- / CAN_RX / 独立: Y
 *   通过P11选择连接USB D-还是CAN_RX
 *   去掉P11跳线帽则完全独立 */
#define PIN_PA11  103, 11, 0, 1 /* USB_D-/CAN_RX, Y */

/** PA12 - PIN104 - USB_D+ / CAN_TX / 独立: Y
 *   通过P11选择连接USB D+还是CAN_TX
 *   去掉P11跳线帽则完全独立 */
#define PIN_PA12  104, 12, 0, 1 /* USB_D+/CAN_TX, Y */

/** PA13 - PIN105 - JTMS / SWDIO / 独立: N
 *   JTAG/SWD仿真接口，没接任何外设
 *   SWD模式仅需SWDIO和SWDCLK两个信号即可仿真
 *   不用仿真器时可用作普通IO（有10K上/下拉电阻） */
#define PIN_PA13  105, 13, 0, 0 /* JTMS/SWDIO, N */

/** PA14 - PIN109 - JTCK / SWDCLK / 独立: N
 *   JTAG/SWD仿真接口，没接任何外设 */
#define PIN_PA14  109, 14, 0, 0 /* JTCK/SWDCLK, N */

/** PA15 - PIN110 - JTDI / USB_PWR / 独立: N
 *   1，JTAG仿真口(JTDI)
 *   2，USB_HOST接口供电控制脚（有10K上拉电阻）
 *   不用JTAG和USB_HOST时可做普通IO */
#define PIN_PA15  110, 15, 0, 0 /* JTDI/USB_PWR, N */

/* ================================================================
 * PB 端口（Port B）
 * ================================================================ */

/** PB0 - PIN46 - T_SCK / 独立: Y
 *   TFTLCD接口触摸屏SCK信号
 *   不插TFTLCD模块时完全独立 */
#define PIN_PB0   46, 0, 1, 1   /* T_SCK, Y */

/** PB1 - PIN47 - T_PEN / 独立: Y
 *   TFTLCD接口触摸屏PEN信号（中断）
 *   不插TFTLCD模块时完全独立 */
#define PIN_PB1   47, 1, 1, 1   /* T_PEN, Y */

/** PB2 - PIN48 - BOOT1 / T_MISO / 独立: N
 *   1，BOOT1，启动选择配置引脚（上电时用）
 *   2，TFTLCD接口触摸屏MISO信号
 *   有10K上拉/下拉电阻，B0控制 */
#define PIN_PB2   48, 2, 1, 0   /* BOOT1/T_MISO, N */

/** PB3 - PIN133 - JTDO / SPI1_SCK / 独立: N
 *   1，JTAG仿真口(JTDO)
 *   2，W25Q128和WIRELESS接口的SCK信号
 *   禁用JTAG且不插外设时可做普通IO */
#define PIN_PB3   133, 3, 1, 0  /* JTDO/SPI1_SCK, N */

/** PB4 - PIN134 - JTRST / SPI1_MISO / 独立: N
 *   1，JTAG仿真口(JTRST)
 *   2，W25Q128和WIRELESS接口的MISO信号
 *   禁用JTAG且不插外设时可做普通IO */
#define PIN_PB4   134, 4, 1, 0  /* JTRST/SPI1_MISO, N */

/** PB5 - PIN135 - SPI1_MOSI / 独立: N
 *   W25Q128和WIRELESS接口的MOSI信号
 *   禁止W25Q128片选且不插WIRELESS时可做普通IO */
#define PIN_PB5   135, 5, 1, 0  /* SPI1_MOSI, N */

/** PB6 - PIN136 - DCMI_D5 / 独立: Y
 *   OLED/CAMERA接口的D5脚
 *   不插OLED/CAMERA时完全独立 */
#define PIN_PB6   136, 6, 1, 1  /* DCMI_D5, Y */

/** PB7 - PIN137 - DCMI_VSYNC / 独立: Y
 *   OLED/CAMERA接口的VSYNC脚
 *   不插OLED/CAMERA时完全独立 */
#define PIN_PB7   137, 7, 1, 1  /* DCMI_VSYNC, Y */

/** PB8 - PIN139 - IIC_SCL / 独立: N
 *   接24C02 & MPU6050 & WM8978的SCL（有4.7K上拉）
 *   不建议做普通IO */
#define PIN_PB8   139, 8, 1, 0  /* IIC_SCL, N */

/** PB9 - PIN140 - IIC_SDA / 独立: N
 *   接24C02 & MPU6050 & WM8978的SDA（有4.7K上拉）
 *   不建议做普通IO */
#define PIN_PB9   140, 9, 1, 0  /* IIC_SDA, N */

/** PB10 - PIN69 - USART3_TX / 独立: Y
 *   1，RS232串口3(COM3)RX脚（P10设置）
 *   2，ATK-MODULE接口的RXD脚（P10设置）
 *   去掉P10跳线帽则完全独立 */
#define PIN_PB10  69, 10, 1, 1  /* USART3_TX, Y */

/** PB11 - PIN70 - USART3_RX / 独立: Y
 *   1，RS232串口3(COM3)TX脚（P10设置）
 *   2，ATK-MODULE接口的TXD脚（P10设置）
 *   去掉P10跳线帽则完全独立 */
#define PIN_PB11  70, 11, 1, 1  /* USART3_RX, Y */

/** PB12 - PIN73 - I2S_LRCK / 独立: N
 *   WM8978的LRCK信号
 *   不用WM8978时可做普通IO */
#define PIN_PB12  73, 12, 1, 0  /* I2S_LRCK, N */

/** PB13 - PIN74 - I2S_SCLK / 独立: N
 *   WM8978的SCLK信号（BCLK）
 *   不用WM8978时可做普通IO */
#define PIN_PB13  74, 13, 1, 0  /* I2S_SCLK, N */

/** PB14 - PIN75 - F_CS / 独立: N
 *   W25Q128的片选信号
 *   不建议做普通IO */
#define PIN_PB14  75, 14, 1, 0  /* F_CS, N */

/** PB15 - PIN76 - LCD_BL / 独立: Y
 *   TFTLCD接口背光控制脚
 *   不插TFTLCD模块时完全独立 */
#define PIN_PB15  76, 15, 1, 1  /* LCD_BL, Y */

/* ================================================================
 * PC 端口（Port C）
 * ================================================================ */

/** PC0 - PIN26 - GBC_LED / 3D_INT / 独立: N
 *   1，ATK-MODULE接口的LED引脚
 *   2，MPU6050模块的中断脚
 *   不用ATK-MODULE和MPU6050时可做普通IO */
#define PIN_PC0   26, 0, 2, 0   /* GBC_LED/3D_INT, N */

/** PC1 - PIN27 - ETH_MDC / 独立: N
 *   接LAN8720的MDC脚
 *   LAN8720复位时可做普通IO */
#define PIN_PC1   27, 1, 2, 0   /* ETH_MDC, N */

/** PC2 - PIN28 - I2S_SDOUT / 独立: N
 *   WM8978的SDOUT信号（ADCDAT）
 *   不用WM8978时可做普通IO */
#define PIN_PC2   28, 2, 2, 0   /* I2S_SDOUT, N */

/** PC3 - PIN29 - I2S_SDIN / 独立: N
 *   WM8978的SDIN信号（ADCDAT）
 *   不用WM8978时可做普通IO */
#define PIN_PC3   29, 3, 2, 0   /* I2S_SDIN, N */

/** PC4 - PIN44 - RMII_RXD0 / 独立: N
 *   接LAN8720的RXD0脚
 *   LAN8720复位时可做普通IO */
#define PIN_PC4   44, 4, 2, 0   /* RMII_RXD0, N */

/** PC5 - PIN45 - RMII_RXD1 / 独立: N
 *   接LAN8720的RXD1脚
 *   LAN8720复位时可做普通IO */
#define PIN_PC5   45, 5, 2, 0   /* RMII_RXD1, N */

/** PC6 - PIN96 - I2S_MCLK / DCMI_D0 / 独立: N
 *   1，WM8978的MCLK脚
 *   2，OLED/CAMERA接口的D0脚
 *   不用WM8978且不插OLED/CAMERA时可做普通IO */
#define PIN_PC6   96, 6, 2, 0   /* I2S_MCLK/DCMI_D0, N */

/** PC7 - PIN97 - DCMI_D1 / 独立: Y
 *   OLED/CAMERA接口的D1脚
 *   不插OLED/CAMERA时完全独立 */
#define PIN_PC7   97, 7, 2, 1   /* DCMI_D1, Y */

/** PC8 - PIN98 - SDIO_D0 / DCMI_D2 / 独立: N
 *   1，SD卡接口的D0
 *   2，OLED/CAMERA接口的D2（有47K上拉）
 *   不用SD卡和OLED/CAMERA时可做普通IO */
#define PIN_PC8   98, 8, 2, 0   /* SDIO_D0/DCMI_D2, N */

/** PC9 - PIN99 - SDIO_D1 / DCMI_D3 / 独立: N
 *   1，SD卡接口的D1
 *   2，OLED/CAMERA接口的D3（有47K上拉）
 *   不用SD卡和OLED/CAMERA时可做普通IO */
#define PIN_PC9   99, 9, 2, 0   /* SDIO_D1/DCMI_D3, N */

/** PC10 - PIN111 - SDIO_D2 / 独立: N
 *   SD卡接口的D2（有47K上拉）
 *   不用SD卡时可做普通IO */
#define PIN_PC10  111, 10, 2, 0 /* SDIO_D2, N */

/** PC11 - PIN112 - SDIO_D3 / 独立: N
 *   SD卡接口的D3（有47K上拉）
 *   不用SD卡时可做普通IO */
#define PIN_PC11  112, 11, 2, 0 /* SDIO_D3, N */

/** PC12 - PIN113 - SDIO_SCK / DCMI_D4 / 独立: Y
 *   1，SD卡接口的SCK
 *   2，OLED/CAMERA接口的D4
 *   不用SD卡和OLED/CAMERA时可做普通IO */
#define PIN_PC12  113, 12, 2, 1 /* SDIO_SCK/DCMI_D4, Y */

/** PC13 - PIN7 - T_CS / 独立: Y
 *   TFTLCD接口触摸屏CS信号
 *   不插TFTLCD模块时完全独立 */
#define PIN_PC13  7, 13, 2, 1   /* T_CS, Y */

/** PC14 - PIN8 - RTC晶振 / 独立: N
 *   接32.768K晶振，**不可用做IO** */
#define PIN_PC14  8, 14, 2, 0   /* RTC_XTAL, N (NOT IO) */

/** PC15 - PIN9 - RTC晶振 / 独立: N
 *   接32.768K晶振，**不可用做IO** */
#define PIN_PC15  9, 15, 2, 0   /* RTC_XTAL, N (NOT IO) */

/* ================================================================
 * PD 端口（Port D）
 * ================================================================ */

/** PD0 - PIN114 - FSMC_D2 / 独立: N
 *   FSMC总线数据线D2（TFTLCD/IS62WV51216共用）
 *   禁止外设片选后可做普通IO */
#define PIN_PD0   114, 0, 3, 0  /* FSMC_D2, N */

/** PD1 - PIN115 - FSMC_D3 / 独立: N
 *   FSMC总线数据线D3（TFTLCD/IS62WV51216共用） */
#define PIN_PD1   115, 1, 3, 0  /* FSMC_D3, N */

/** PD2 - PIN116 - SDIO_CMD / 独立: N
 *   SD卡接口的CMD（有47K上拉）
 *   不用SD卡时可做普通IO */
#define PIN_PD2   116, 2, 3, 0  /* SDIO_CMD, N */

/** PD3 - PIN117 - ETH_RESET / 独立: N
 *   接LAN8720的复位脚（有10K下拉）
 *   不建议做普通IO */
#define PIN_PD3   117, 3, 3, 0  /* ETH_RESET, N */

/** PD4 - PIN118 - FSMC_NOE / 独立: N
 *   FSMC总线NOE(RD)（TFTLCD/IS62WV51216共用）
 *   禁止外设片选后可做普通IO */
#define PIN_PD4   118, 4, 3, 0  /* FSMC_NOE, N */

/** PD5 - PIN119 - FSMC_NWE / 独立: N
 *   FSMC总线NWE(WR)（TFTLCD/IS62WV51216共用） */
#define PIN_PD5   119, 5, 3, 0  /* FSMC_NWE, N */

/** PD6 - PIN122 - DCMI_SCL / 独立: Y
 *   OLED/CAMERA接口的SCL脚
 *   不插OLED/CAMERA时完全独立 */
#define PIN_PD6   122, 6, 3, 1  /* DCMI_SCL, Y */

/** PD7 - PIN123 - DCMI_SDA / 独立: Y
 *   OLED/CAMERA接口的SDA脚
 *   不插OLED/CAMERA时完全独立 */
#define PIN_PD7   123, 7, 3, 1  /* DCMI_SDA, Y */

/** PD8 - PIN77 - FSMC_D13 / 独立: N
 *   FSMC总线数据线D13（TFTLCD/IS62WV51216共用） */
#define PIN_PD8   77, 8, 3, 0   /* FSMC_D13, N */

/** PD9 - PIN78 - FSMC_D14 / 独立: N
 *   FSMC总线数据线D14（TFTLCD/IS62WV51216共用） */
#define PIN_PD9   78, 9, 3, 0   /* FSMC_D14, N */

/** PD10 - PIN79 - FSMC_D15 / 独立: N
 *   FSMC总线数据线D15（TFTLCD/IS62WV51216共用） */
#define PIN_PD10  79, 10, 3, 0  /* FSMC_D15, N */

/** PD11 - PIN80 - FSMC_A16 / 独立: N
 *   FSMC总线地址线A17（IS62WV51216专用）
 *   禁止IS62WV51216片选后可做普通IO */
#define PIN_PD11  80, 11, 3, 0  /* FSMC_A16, N */

/** PD12 - PIN81 - FSMC_A17 / 独立: N
 *   FSMC总线地址线A18（IS62WV51216专用） */
#define PIN_PD12  81, 12, 3, 0  /* FSMC_A17, N */

/** PD13 - PIN82 - FSMC_A18 / 独立: N
 *   FSMC总线地址线A19（IS62WV51216专用） */
#define PIN_PD13  82, 13, 3, 0  /* FSMC_A18, N */

/** PD14 - PIN85 - FSMC_D0 / 独立: N
 *   FSMC总线数据线D0（TFTLCD/IS62WV51216共用） */
#define PIN_PD14  85, 14, 3, 0  /* FSMC_D0, N */

/** PD15 - PIN86 - FSMC_D1 / 独立: N
 *   FSMC总线数据线D1（TFTLCD/IS62WV51216共用） */
#define PIN_PD15  86, 15, 3, 0  /* FSMC_D1, N */

/* ================================================================
 * PE 端口（Port E）
 * ================================================================ */

/** PE0 - PIN141 - FSMC_NBL0 / 独立: N
 *   FSMC总线NBL0（IS62WV51216专用）
 *   禁止IS62WV51216片选后可做普通IO */
#define PIN_PE0   141, 0, 4, 0  /* FSMC_NBL0, N */

/** PE1 - PIN142 - FSMC_NBL1 / 独立: N
 *   FSMC总线NBL1（IS62WV51216专用） */
#define PIN_PE1   142, 1, 4, 0  /* FSMC_NBL1, N */

/** PE2 - PIN1 - KEY2 / 独立: Y
 *   接按键KEY2
 *   只要KEY2不按下，该IO完全独立 */
#define PIN_PE2   1, 2, 4, 1     /* KEY2, Y */

/** PE3 - PIN2 - KEY1 / 独立: Y
 *   接按键KEY1
 *   只要KEY1不按下，该IO完全独立 */
#define PIN_PE3   2, 3, 4, 1    /* KEY1, Y */

/** PE4 - PIN3 - KEY0 / 独立: Y
 *   接按键KEY0
 *   只要KEY0不按下，该IO完全独立 */
#define PIN_PE4   3, 4, 4, 1    /* KEY0, Y */

/** PE5 - PIN4 - DCMI_D6 / 独立: Y
 *   OLED/CAMERA接口的D6脚
 *   不插OLED/CAMERA时完全独立 */
#define PIN_PE5   4, 5, 4, 1    /* DCMI_D6, Y */

/** PE6 - PIN5 - DCMI_D7 / 独立: Y
 *   OLED/CAMERA接口的D7脚
 *   不插OLED/CAMERA时完全独立 */
#define PIN_PE6   5, 6, 4, 1    /* DCMI_D7, Y */

/** PE7 - PIN58 - FSMC_D4 / 独立: N
 *   FSMC总线数据线D4（TFTLCD/IS62WV51216共用） */
#define PIN_PE7   58, 7, 4, 0   /* FSMC_D4, N */

/** PE8 - PIN59 - FSMC_D5 / 独立: N
 *   FSMC总线数据线D5（TFTLCD/IS62WV51216共用） */
#define PIN_PE8   59, 8, 4, 0   /* FSMC_D5, N */

/** PE9 - PIN60 - FSMC_D6 / 独立: N
 *   FSMC总线数据线D6（TFTLCD/IS62WV51216共用） */
#define PIN_PE9   60, 9, 4, 0   /* FSMC_D6, N */

/** PE10 - PIN63 - FSMC_D7 / 独立: N
 *   FSMC总线数据线D7（TFTLCD/IS62WV51216共用） */
#define PIN_PE10  63, 10, 4, 0   /* FSMC_D7, N */

/** PE11 - PIN64 - FSMC_D8 / 独立: N
 *   FSMC总线数据线D8（TFTLCD/IS62WV51216共用） */
#define PIN_PE11  64, 11, 4, 0   /* FSMC_D8, N */

/** PE12 - PIN65 - FSMC_D9 / 独立: N
 *   FSMC总线数据线D9（TFTLCD/IS62WV51216共用） */
#define PIN_PE12  65, 12, 4, 0  /* FSMC_D9, N */

/** PE13 - PIN66 - FSMC_D10 / 独立: N
 *   FSMC总线数据线D10（TFTLCD/IS62WV51216共用） */
#define PIN_PE13  66, 13, 4, 0  /* FSMC_D10, N */

/** PE14 - PIN67 - FSMC_D11 / 独立: N
 *   FSMC总线数据线D11（TFTLCD/IS62WV51216共用） */
#define PIN_PE14  67, 14, 4, 0  /* FSMC_D11, N */

/** PE15 - PIN68 - FSMC_D12 / 独立: N
 *   FSMC总线数据线D12（TFTLCD/IS62WV51216共用） */
#define PIN_PE15  68, 15, 4, 0  /* FSMC_D12, N */

/* ================================================================
 * PF 端口（Port F）
 * ================================================================ */

/** PF0 - PIN10 - FSMC_A0 / 独立: N
 *   FSMC总线地址线A0（IS62WV51216专用）
 *   禁止IS62WV51216片选后可做普通IO */
#define PIN_PF0   10, 0, 5, 0    /* FSMC_A0, N */

/** PF1 - PIN11 - FSMC_A1 / 独立: N
 *   FSMC总线地址线A1（IS62WV51216专用） */
#define PIN_PF1   11, 1, 5, 0    /* FSMC_A1, N */

/** PF2 - PIN12 - FSMC_A2 / 独立: N
 *   FSMC总线地址线A2（IS62WV51216专用） */
#define PIN_PF2   12, 2, 5, 0    /* FSMC_A2, N */

/** PF3 - PIN13 - FSMC_A3 / 独立: N
 *   FSMC总线地址线A3（IS62WV51216专用） */
#define PIN_PF3   13, 3, 5, 0    /* FSMC_A3, N */

/** PF4 - PIN14 - FSMC_A4 / 独立: N
 *   FSMC总线地址线A4（IS62WV51216专用） */
#define PIN_PF4   14, 4, 5, 0    /* FSMC_A4, N */

/** PF5 - PIN15 - FSMC_A5 / 独立: N
 *   FSMC总线地址线A5（IS62WV51216专用） */
#define PIN_PF5   15, 5, 5, 0    /* FSMC_A5, N */

/** PF6 - PIN18 - GBC_KEY / 独立: Y
 *   MODULE接口的KEY脚
 *   不插MODULE接口时完全独立 */
#define PIN_PF6   18, 6, 5, 1    /* GBC_KEY, Y */

/** PF7 - PIN19 - LIGHT_SENSOR / 独立: N
 *   接光敏传感器(LS1)
 *   可做普通IO，建议仅做输出 */
#define PIN_PF7   19, 7, 5, 0    /* LIGHT_SENSOR, N */

/** PF8 - PIN20 - BEEP / 独立: N
 *   接蜂鸣器(BEEP)
 *   不建议作为普通IO */
#define PIN_PF8   20, 8, 5, 0    /* BEEP, N */

/** PF9 - PIN21 - LED0 / 独立: N
 *   接DS0 LED灯（红色）
 *   如做普通IO用则DS0也受控制，建议仅做输出 */
#define PIN_PF9   21, 9, 5, 0    /* LED0, N */

/** PF10 - PIN22 - LED1 / 独立: N
 *   接DS1 LED灯（绿色）
 *   如做普通IO用则DS1也受控制，建议仅做输出 */
#define PIN_PF10  22, 10, 5, 0   /* LED1, N */

/** PF11 - PIN49 - T_MOSI / 独立: Y
 *   TFTLCD接口触摸屏MOSI信号
 *   不插TFTLCD模块时完全独立 */
#define PIN_PF11  49, 11, 5, 1   /* T_MOSI, Y */

/** PF12 - PIN50 - FSMC_A6 / 独立: N
 *   FSMC总线地址线A10（IS62WV51216/TFTLCD共用）
 *   禁止外设片选后可做普通IO */
#define PIN_PF12  50, 12, 5, 0   /* FSMC_A6, N */

/** PF13 - PIN53 - FSMC_A7 / 独立: N
 *   FSMC总线地址线A7（IS62WV51216专用） */
#define PIN_PF13  53, 13, 5, 0   /* FSMC_A7, N */

/** PF14 - PIN54 - FSMC_A8 / 独立: N
 *   FSMC总线地址线A8（IS62WV51216专用） */
#define PIN_PF14  54, 14, 5, 0   /* FSMC_A8, N */

/** PF15 - PIN55 - FSMC_A9 / 独立: N
 *   FSMC总线地址线A9（IS62WV51216专用） */
#define PIN_PF15  55, 15, 5, 0   /* FSMC_A9, N */

/* ================================================================
 * PG 端口（Port G）
 * ================================================================ */

/** PG0 - PIN56 - FSMC_A10 / 独立: N
 *   FSMC总线地址线A10（IS62WV51216专用）
 *   禁止IS62WV51216片选后可做普通IO */
#define PIN_PG0   56, 0, 6, 0    /* FSMC_A10, N */

/** PG1 - PIN57 - FSMC_A11 / 独立: N
 *   FSMC总线地址线A11（IS62WV51216专用） */
#define PIN_PG1   57, 1, 6, 0    /* FSMC_A11, N */

/** PG2 - PIN87 - FSMC_A12 / 独立: N
 *   FSMC总线地址线A12（IS62WV51216专用） */
#define PIN_PG2   87, 2, 6, 0    /* FSMC_A12, N */

/** PG3 - PIN88 - FSMC_A13 / 独立: N
 *   FSMC总线地址线A13（IS62WV51216专用） */
#define PIN_PG3   88, 3, 6, 0    /* FSMC_A13, N */

/** PG4 - PIN89 - FSMC_A14 / 独立: N
 *   FSMC总线地址线A14（IS62WV51216专用） */
#define PIN_PG4   89, 4, 6, 0    /* FSMC_A14, N */

/** PG5 - PIN90 - FSMC_A15 / 独立: N
 *   FSMC总线地址线A15（IS62WV51216专用） */
#define PIN_PG5   90, 5, 6, 0    /* FSMC_A15, N */

/** PG6 - PIN91 - NRF_CE / 独立: Y
 *   WIRELESS接口的CE脚
 *   不插WIRELESS模块时完全独立 */
#define PIN_PG6   91, 6, 6, 1    /* NRF_CE, Y */

/** PG7 - PIN92 - NRF_CS / 独立: Y
 *   WIRELESS接口的CS脚
 *   不插WIRELESS模块时完全独立 */
#define PIN_PG7   92, 7, 6, 1    /* NRF_CS, Y */

/** PG8 - PIN93 - NRF_IRQ / RS485_RE / 独立: N
 *   1，WIRELESS接口IRQ信号
 *   2，RS485 RE引脚
 *   不用WIRELESS和RS485时可做普通IO */
#define PIN_PG8   93, 8, 6, 0    /* NRF_IRQ/RS485_RE, N */

/** PG9 - PIN124 - DCMI_PWDN / 1WIRE_DQ / 独立: N
 *   1，OLED/CAMERA接口的PWDN脚
 *   2，接单总线接口(U12) DHT11/DS18B20（有4.7K上拉）
 *   不用OLED/CAMERA和单总线接口时可做普通IO */
#define PIN_PG9   124, 9, 6, 0   /* DCMI_PWDN/1WIRE_DQ, N */

/** PG10 - PIN125 - FSMC_NE3 / 独立: N
 *   FSMC总线的片选信号3，为外部SRAM（IS62WV51216）片选信号
 *   不建议作为普通IO */
#define PIN_PG10  125, 10, 6, 0  /* FSMC_NE3, N */

/** PG11 - PIN126 - RMII_TX_EN / 独立: N
 *   接LAN8720的TXEN脚
 *   LAN8720复位时可做普通IO */
#define PIN_PG11  126, 11, 6, 0  /* RMII_TX_EN, N */

/** PG12 - PIN127 - FSMC_NE4 / 独立: Y
 *   FSMC总线的片选信号4，为LCD片选信号
 *   不插TFTLCD接口时完全独立 */
#define PIN_PG12  127, 12, 6, 1  /* FSMC_NE4, Y */

/** PG13 - PIN128 - RMII_TXD0 / 独立: N
 *   接LAN8720的TXD0脚
 *   LAN8720复位时可做普通IO */
#define PIN_PG13  128, 13, 6, 0  /* RMII_TXD0, N */

/** PG14 - PIN129 - RMII_TXD1 / 独立: N
 *   接LAN8720的TXD1脚
 *   LAN8720复位时可做普通IO */
#define PIN_PG14  129, 14, 6, 0  /* RMII_TXD1, N */

/** PG15 - PIN132 - DCMI_RESET / 独立: Y
 *   OLED/CAMERA接口的RESET脚
 *   不插OLED/CAMERA时完全独立 */
#define PIN_PG15  132, 15, 6, 1  /* DCMI_RESET, Y */

/* ================================================================
 * 辅助宏
 * ================================================================ */

/* 将 GPIO 端口名转为 RCC 使能位
 * 使用方法：RCC->AHB1ENR |= RCC_AHB1ENR_GPIOX(GPIOX); */
#define GPIO_PORT_TO_BIT(PORT)  (1U << (PORT))

/* 判断引脚是否可完全独立（IND = Y）
 * 使用方法：if (PIN_IS_INDEP(PC4)) { ... }
 * 注意：PC14/PC15 RTC晶振恒返回0 */
#define PIN_IS_INDEP(pin)       (((pin) & 0x100) == 0)

/* GPIO 端口号转 RCC AHB1 使能寄存器位域偏移
 * A=0, B=1, C=2, D=3, E=4, F=5, G=6 */
#define GPIO_PORT_NUM(GPIO)     ((GPIO) - 'A')

/* 常用快速引脚别名（可直接替换使用） */
#define LED_RED    PIN_PF9    /* PF9  - DS0 红色LED */
#define LED_GREEN  PIN_PF10   /* PF10 - DS1 绿色LED */
#define BEEP       PIN_PF8    /* PF8  - 蜂鸣器 */
#define KEY0       PIN_PE4     /* PE4  - 按键KEY0 */
#define KEY1       PIN_PE3     /* PE3  - 按键KEY1 */
#define KEY2       PIN_PE2     /* PE2  - 按键KEY2 */
#define KEY_UP     PIN_PA0     /* PA0  - 按键KEY_UP */

#endif /* __PINDET_H__ */
