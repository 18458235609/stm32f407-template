# 神舟号开发板 IO 引脚分配图

> 芯片：STM32F407ZGT6（LQFP144）  
> 整理自：神舟号IO资源分配表.xlsx

---

## 引脚定义速查

### PA 端口（Port A）

| GPIO | PIN | 功能 | 独立 | 说明 |
|------|-----|------|------|------|
| PA0 | 34 | WK_UP | ✅ | 按键KEY_UP，可待机唤醒 |
| PA1 | 35 | RMII_REF_CLK | ❌ | 接LAN8720 REFCLKO（50M），不建议做IO |
| PA2 | 36 | USART2_TX/RS485_RX/ETH_MDIO | ❌ | 通过P9选择RS232/RS485，同时接LAN8720 |
| PA3 | 37 | USART2_RX/RS485_TX/PWM_DAC | ❌ | 通过P9选择，同时接PWM_DAC |
| PA4 | 40 | STM_DAC/DCMI_HREF | ✅ | DAC输出，不插外设可独立 |
| PA5 | 41 | STM_ADC/TPAD | ✅ | ADC输入，拔P12可独立 |
| PA6 | 42 | DCMI_PCLK | ✅ | OLED/CAMERA PCLK，不插可独立 |
| PA7 | 43 | RMII_CRS_DV | ❌ | 接LAN8720 CRS_DV |
| PA8 | 100 | DCMI_XCLK/REMOTE_IN | ❌ | OLED/CAMERA XCLK + HS0038红外（有上拉） |
| PA9 | 101 | USART1_TX | ✅ | 串口1 TX，拔P6可独立 |
| PA10 | 102 | USART1_RX | ✅ | 串口1 RX，拔P6可独立 |
| PA11 | 103 | USB_D-/CAN_RX | ✅ | 通过P11选择 |
| PA12 | 104 | USB_D+/CAN_TX | ✅ | 通过P11选择 |
| PA13 | 105 | JTMS/SWDIO | ❌ | SWD仿真接口（有上/下拉） |
| PA14 | 109 | JTCK/SWDCLK | ❌ | SWD仿真接口 |
| PA15 | 110 | JTDI/USB_PWR | ❌ | JTAG + USB_HOST电源（有上拉） |

### PB 端口（Port B）

| GPIO | PIN | 功能 | 独立 | 说明 |
|------|-----|------|------|------|
| PB0 | 46 | T_SCK | ✅ | TFTLCD触摸屏SCK，不插屏可独立 |
| PB1 | 47 | T_PEN | ✅ | TFTLCD触摸屏PEN，不插屏可独立 |
| PB2 | 48 | BOOT1/T_MISO | ❌ | BOOT配置脚+TFTLCD触摸屏MISO |
| PB3 | 133 | JTDO/SPI1_SCK | ❌ | JTAG + W25Q128/WIRELESS SCK |
| PB4 | 134 | JTRST/SPI1_MISO | ❌ | JTAG + W25Q128/WIRELESS MISO |
| PB5 | 135 | SPI1_MOSI | ❌ | W25Q128/WIRELESS MOSI |
| PB6 | 136 | DCMI_D5 | ✅ | OLED/CAMERA D5，不插可独立 |
| PB7 | 137 | DCMI_VSYNC | ✅ | OLED/CAMERA VSYNC，不插可独立 |
| PB8 | 139 | IIC_SCL | ❌ | 24C02/MPU6050/WM8978 SCL（有上拉） |
| PB9 | 140 | IIC_SDA | ❌ | 24C02/MPU6050/WM8978 SDA（有上拉） |
| PB10 | 69 | USART3_TX | ✅ | COM3 RX / ATK-MOD RXD，拔P10可独立 |
| PB11 | 70 | USART3_RX | ✅ | COM3 TX / ATK-MOD TXD，拔P10可独立 |
| PB12 | 73 | I2S_LRCK | ❌ | WM8978 LRCK |
| PB13 | 74 | I2S_SCLK | ❌ | WM8978 BCLK |
| PB14 | 75 | F_CS | ❌ | W25Q128片选，不建议做IO |
| PB15 | 76 | LCD_BL | ✅ | TFTLCD背光，不插屏可独立 |

### PC 端口（Port C）

| GPIO | PIN | 功能 | 独立 | 说明 |
|------|-----|------|------|------|
| PC0 | 26 | GBC_LED/3D_INT | ❌ | ATK-MODULE LED + MPU6050 INT |
| PC1 | 27 | ETH_MDC | ❌ | LAN8720 MDC |
| PC2 | 28 | I2S_SDOUT | ❌ | WM8978 ADCDAT |
| PC3 | 29 | I2S_SDIN | ❌ | WM8978 DACDAT |
| PC4 | 44 | RMII_RXD0 | ❌ | LAN8720 RXD0 |
| PC5 | 45 | RMII_RXD1 | ❌ | LAN8720 RXD1 |
| PC6 | 96 | I2S_MCLK/DCMI_D0 | ❌ | WM8978 MCLK + OLED/CAMERA D0 |
| PC7 | 97 | DCMI_D1 | ✅ | OLED/CAMERA D1，不插可独立 |
| PC8 | 98 | SDIO_D0/DCMI_D2 | ❌ | SD卡D0 + OLED/CAMERA D2（有上拉） |
| PC9 | 99 | SDIO_D1/DCMI_D3 | ❌ | SD卡D1 + OLED/CAMERA D3（有上拉） |
| PC10 | 111 | SDIO_D2 | ❌ | SD卡D2（有上拉） |
| PC11 | 112 | SDIO_D3 | ❌ | SD卡D3（有上拉） |
| PC12 | 113 | SDIO_SCK/DCMI_D4 | ✅ | SD卡SCK + OLED/CAMERA D4 |
| PC13 | 7 | T_CS | ✅ | TFTLCD触摸屏CS，不插屏可独立 |
| PC14 | 8 | RTC晶振 | ❌ | **32.768K晶振，不可做IO** |
| PC15 | 9 | RTC晶振 | ❌ | **32.768K晶振，不可做IO** |

### PD 端口（Port D）

| GPIO | PIN | 功能 | 独立 | 说明 |
|------|-----|------|------|------|
| PD0 | 114 | FSMC_D2 | ❌ | TFTLCD/IS62WV51216共用 |
| PD1 | 115 | FSMC_D3 | ❌ | TFTLCD/IS62WV51216共用 |
| PD2 | 116 | SDIO_CMD | ❌ | SD卡CMD（有上拉） |
| PD3 | 117 | ETH_RESET | ❌ | LAN8720复位（有下拉） |
| PD4 | 118 | FSMC_NOE | ❌ | FSMC RD，TFTLCD/IS62共用 |
| PD5 | 119 | FSMC_NWE | ❌ | FSMC WR，TFTLCD/IS62共用 |
| PD6 | 122 | DCMI_SCL | ✅ | OLED/CAMERA SCL，不插可独立 |
| PD7 | 123 | DCMI_SDA | ✅ | OLED/CAMERA SDA，不插可独立 |
| PD8 | 77 | FSMC_D13 | ❌ | TFTLCD/IS62WV51216共用 |
| PD9 | 78 | FSMC_D14 | ❌ | TFTLCD/IS62WV51216共用 |
| PD10 | 79 | FSMC_D15 | ❌ | TFTLCD/IS62WV51216共用 |
| PD11 | 80 | FSMC_A16 | ❌ | IS62WV51216专用 |
| PD12 | 81 | FSMC_A17 | ❌ | IS62WV51216专用 |
| PD13 | 82 | FSMC_A18 | ❌ | IS62WV51216专用 |
| PD14 | 85 | FSMC_D0 | ❌ | TFTLCD/IS62WV51216共用 |
| PD15 | 86 | FSMC_D1 | ❌ | TFTLCD/IS62WV51216共用 |

### PE 端口（Port E）

| GPIO | PIN | 功能 | 独立 | 说明 |
|------|-----|------|------|------|
| PE0 | 141 | FSMC_NBL0 | ❌ | IS62WV51216专用 |
| PE1 | 142 | FSMC_NBL1 | ❌ | IS62WV51216专用 |
| PE2 | 1 | KEY2 | ✅ | 按键KEY2，不按可独立 |
| PE3 | 2 | KEY1 | ✅ | 按键KEY1，不按可独立 |
| PE4 | 3 | KEY0 | ✅ | 按键KEY0，不按可独立 |
| PE5 | 4 | DCMI_D6 | ✅ | OLED/CAMERA D6，不插可独立 |
| PE6 | 5 | DCMI_D7 | ✅ | OLED/CAMERA D7，不插可独立 |
| PE7 | 58 | FSMC_D4 | ❌ | TFTLCD/IS62WV51216共用 |
| PE8 | 59 | FSMC_D5 | ❌ | TFTLCD/IS62WV51216共用 |
| PE9 | 60 | FSMC_D6 | ❌ | TFTLCD/IS62WV51216共用 |
| PE10 | 63 | FSMC_D7 | ❌ | TFTLCD/IS62WV51216共用 |
| PE11 | 64 | FSMC_D8 | ❌ | TFTLCD/IS62WV51216共用 |
| PE12 | 65 | FSMC_D9 | ❌ | TFTLCD/IS62WV51216共用 |
| PE13 | 66 | FSMC_D10 | ❌ | TFTLCD/IS62WV51216共用 |
| PE14 | 67 | FSMC_D11 | ❌ | TFTLCD/IS62WV51216共用 |
| PE15 | 68 | FSMC_D12 | ❌ | TFTLCD/IS62WV51216共用 |

### PF 端口（Port F）

| GPIO | PIN | 功能 | 独立 | 说明 |
|------|-----|------|------|------|
| PF0 | 10 | FSMC_A0 | ❌ | IS62WV51216专用 |
| PF1 | 11 | FSMC_A1 | ❌ | IS62WV51216专用 |
| PF2 | 12 | FSMC_A2 | ❌ | IS62WV51216专用 |
| PF3 | 13 | FSMC_A3 | ❌ | IS62WV51216专用 |
| PF4 | 14 | FSMC_A4 | ❌ | IS62WV51216专用 |
| PF5 | 15 | FSMC_A5 | ❌ | IS62WV51216专用 |
| PF6 | 18 | GBC_KEY | ✅ | MODULE接口KEY，不插可独立 |
| PF7 | 19 | LIGHT_SENSOR | ❌ | 光敏传感器，建议仅做输出 |
| PF8 | 20 | BEEP | ❌ | 蜂鸣器，不建议做IO |
| PF9 | 21 | LED0 | ❌ | DS0红色LED，建议仅做输出 |
| PF10 | 22 | LED1 | ❌ | DS1绿色LED，建议仅做输出 |
| PF11 | 49 | T_MOSI | ✅ | TFTLCD触摸屏MOSI，不插屏可独立 |
| PF12 | 50 | FSMC_A6 | ❌ | IS62WV51216/TFTLCD共用 |
| PF13 | 53 | FSMC_A7 | ❌ | IS62WV51216专用 |
| PF14 | 54 | FSMC_A8 | ❌ | IS62WV51216专用 |
| PF15 | 55 | FSMC_A9 | ❌ | IS62WV51216专用 |

### PG 端口（Port G）

| GPIO | PIN | 功能 | 独立 | 说明 |
|------|-----|------|------|------|
| PG0 | 56 | FSMC_A10 | ❌ | IS62WV51216专用 |
| PG1 | 57 | FSMC_A11 | ❌ | IS62WV51216专用 |
| PG2 | 87 | FSMC_A12 | ❌ | IS62WV51216专用 |
| PG3 | 88 | FSMC_A13 | ❌ | IS62WV51216专用 |
| PG4 | 89 | FSMC_A14 | ❌ | IS62WV51216专用 |
| PG5 | 90 | FSMC_A15 | ❌ | IS62WV51216专用 |
| PG6 | 91 | NRF_CE | ✅ | WIRELESS接口CE，不插可独立 |
| PG7 | 92 | NRF_CS | ✅ | WIRELESS接口CS，不插可独立 |
| PG8 | 93 | NRF_IRQ/RS485_RE | ❌ | WIRELESS IRQ + RS485 RE |
| PG9 | 124 | DCMI_PWDN/1WIRE_DQ | ❌ | OLED/CAMERA PWDN + 单总线DHT11/DS18B20 |
| PG10 | 125 | FSMC_NE3 | ❌ | IS62WV51216片选，不建议做IO |
| PG11 | 126 | RMII_TX_EN | ❌ | LAN8720 TXEN |
| PG12 | 127 | FSMC_NE4 | ✅ | TFTLCD片选，不插屏可独立 |
| PG13 | 128 | RMII_TXD0 | ❌ | LAN8720 TXD0 |
| PG14 | 129 | RMII_TXD1 | ❌ | LAN8720 TXD1 |
| PG15 | 132 | DCMI_RESET | ✅ | OLED/CAMERA RESET，不插可独立 |

---

## 跳线选择器说明

| 跳线 | 功能 | 影响引脚 |
|------|------|---------|
| P6 | USART1 ↔ CH340 | PA9, PA10 |
| P9 | USART2 RS232 ↔ RS485 | PA2, PA3 |
| P10 | USART3 ↔ ATK-MODULE | PB10, PB11 |
| P11 | USB ↔ CAN | PA11, PA12 |
| P12 | ADC ↔ TPAD | PA5 |

---

## 快速别名（可直接在代码中使用）

| 别名 | 对应GPIO | 说明 |
|------|---------|------|
| `LED_RED` | PF9 | 红色LED DS0 |
| `LED_GREEN` | PF10 | 绿色LED DS1 |
| `BEEP` | PF8 | 蜂鸣器 |
| `KEY0` | PE4 | 按键KEY0 |
| `KEY1` | PE3 | 按键KEY1 |
| `KEY2` | PE2 | 按键KEY2 |
| `KEY_UP` | PA0 | 按键KEY_UP（唤醒） |
