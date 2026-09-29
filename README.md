# GD32F407 固件工程

基于 GD32F407 的嵌入式项目，使用 EIDE 构建。包含 GD32 官方标准库、板级支持包（BSP）以及应用层逻辑。

## 目录结构

- **App/CMSIS**：启动文件（startup_gd32f407_427.s）和系统时钟配置（system_gd32f4xx.c）。
- **Driver/BSP**：具体硬件模块的驱动。
  - `bsp_ic_oled.c`：I2C OLED 屏幕
  - `bsp_pcf8563.c`：PCF8563 外部 RTC
  - `bsp_keys.c` / `bsp_leds.c` / `bsp_buzzer.c`：按键、LED、蜂鸣器
  - `bsp_battery_flows.c`：电池相关检测
- **Driver/Firmware**：GD32 官方标准外设库（GPIO、I2C、USART、RTC、PMU 等）。
- **Driver/Hardware**：外设底层初始化，负责引脚、时钟和中断配置（msp_exti.c、msp_i2c.c、msp_rtc.c、msp_uart.c）。
- **Middleware**：中间件层（目前为空，预留）。
- **User**：应用层代码。`main.c` 写业务逻辑，`gd32f4xx_it.c` 处理中断，`systick.c` 提供延时。
- **Output Files**：编译产物（hex/bin）。
- **芯片支持包**：GD32F4xx Pack。

## 硬件外设

根据现有驱动，开发板包含以下资源：
- MCU：GD32F407
- 显示：I2C 接口 OLED
- 时钟：PCF8563（外部 RTC）
- 交互：按键、LED、蜂鸣器
- 电源：电池检测/管理
- 通信：USART、I2C

## 开发环境

- IDE：VS Code + EIDE 插件
- 编译器：ARM Compiler 6 (AC6)
- 依赖：GD32F4xx 芯片支持包

## 编译与烧录

1. 用 VS Code 打开工程。
2. 确认 EIDE 的构建配置中已选择 AC6 编译器和正确的芯片包。
3. 点击构建，生成的固件在 `Output Files` 目录下。
4. 使用 J-Link / ST-Link / GD-Link 烧录。

## 开发注意事项

- 引脚和时钟配置统一放在 `Driver/Hardware` 的 `msp_*.c` 里，不要散落在业务代码中。
- 新增硬件驱动写在 `bsp_*.c`，不要直接在 `main.c` 里操作寄存器。
- 中断服务函数（`gd32f4xx_it.c`）里尽量只做标志位处理，耗时逻辑放到主循环。
- 换板子或者改引脚时，重点检查 `msp_*.c` 里的宏定义。