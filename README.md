# GD32F407 固件工程

基于 **GD32F407VET6**的分层嵌入式工程模板：
片上外设抽象（`Hardware/msp_*`）、片外器件驱动（`Driver/bsp_*`）、应用层（`App`）、FreeRTOS 中间件，
同时提供 **Keil MDK** 与 **EIDE** 两套构建配置。

| 项目 | 内容 |
|---|---|
| 主控 | GD32F407VET6，Cortex-M4F @ 168MHz，512KB Flash / 128KB SRAM（另有 64KB CCM 未使用） |
| 晶振 | 外部 8MHz（`HXTAL_VALUE=8000000`），系统主频 168MHz |
| 实时内核 | FreeRTOS Kernel V202604.01-LTS（`portable/GCC/ARM_CM4F` + `heap_4`） |
| 编译器 | ARM Compiler 6（AC6），Keil MDK 5 / EIDE 插件 |
| 输出 | `My_GD32F407_Template.axf` / `.hex` |

---

## 目录结构

```
GD32F407_myLibrary
├─ User/                  程序入口与系统级代码
│  ├─ main.c              入口：时钟 → 外设初始化 → App 初始化 → 创建任务 → 启动调度器
│  ├─ gd32f4xx_it.c       中断服务：裸机/RTOS 两套（SYS_SUPPORT_OS 切换）
│  ├─ systick.c/.h        延时与时间戳（裸机 SysTick / RTOS TIMER6 + tick 累加）
│  ├─ main.h              全局公共头
│  └─ util_weak.h         __weak 断言/占位工具
├─ App/                   应用层（业务）
│  ├─ app.h               应用层公共接口与 extern 变量
│  ├─ app_input/          输入：按键、ADKEY、触摸按键、矩阵键盘任务
│  ├─ app_show/           显示：188 数码管、数码管、点阵、IIC/SPI OLED、RTC 时间
│  └─ app_test/           上电自测：Flash、OLED、数码管、PCF8563 设时/闹钟
├─ Middleware/            中间件
│  ├─ taskmanager.c/.h    周期任务表（裸机遗留，RTOS 下作为 app_task 的调度内核）
│  └─ FreeRTOS/           FreeRTOS Kernel 源码 + FreeRTOSConfig.h
├─ Hardware/              片上外设初始化（引脚/时钟/中断，msp_*）
│  ├─ msp_uart/           USART0 / USART2，printf 重定向、接收回调
│  ├─ msp_iic/            软件 IIC（PB6/PB7）；另有硬件 I2C0 实现 `msp_iic_hard.c`
│  ├─ msp_spi/            SPI0；`msp_spi.c` 软实现 / `msp_spi_hard.c` 硬实现
│  ├─ msp_adc/            ADC0（含校准）
│  ├─ msp_exti/           EXTI0/3/5/8/11/15（按键、PCF8563、触摸、编码器）
│  ├─ msp_rtc/            片内 RTC（HXTAL 时钟源）
│  ├─ msp_fwdgt/          独立看门狗
│  └─ msp_wwdgt/          窗口看门狗
├─ Driver/                片外器件驱动（bsp_*，见下表）
├─ Firmware/              GD32 官方库
│  ├─ CMSIS/              Cortex-M4 内核文件与 startup_gd32f407_427.s
│  ├─ GD32F4xx_standard_peripheral/   标准外设库
│  └─ GD32F4xx_usb_library/           USB 库（当前工程未使用）
├─ Project/               Keil 工程（.uvprojx），输出在 Objects/
├─ build/                 EIDE 构建输出
└─ Doc/                   设计/排障文档（见文末）
```

---

## 驱动与硬件资源

| 模块 | 驱动目录 | 说明 |
|---|---|---|
| LED | `Driver/bsp_leds` | LED1 / LED2 控制 |
| 独立按键 | `Driver/bsp_keys` | KEY1~KEY4，GPIO 输入 |
| 矩阵键盘 | `Driver/bsp_keypad` | 行/列扫描（当前 main 中未启用） |
| AD 按键 | `Driver/bsp_adkey` | ADC 分压采样，含单击/双击状态机，1ms 扫描 |
| 触摸按键 | `Driver/bsp_touch_key` | SC12B，IIC 接口，中断只置标志 |
| 旋转编码器 | `Driver/bsp_ec12d` | EC12D，A/B 相 + 按键，EXTI 判方向 |
| 蜂鸣器 | `Driver/bsp_buzzer` | TIMER1 PWM，支持音符播放 |
| 188 数码管 | `Driver/bsp_188_leds` | 5 路位选（PE15/PE13/PE11/PE9/PE7） |
| 数码管 | `Driver/bsp_digital_leds` | 共阳段码 + 动态扫描，1ms 扫描一次 |
| 点阵屏 | `Driver/bsp_dot_matrix` | 8×8 点阵，内置 0~9 取模表 |
| IIC OLED | `Driver/bsp_iic_oled` | 软件 IIC + 字库 + `bmp.c` 图片显示 |
| SPI OLED | `Driver/bsp_spi_oled` | 硬件 SPI，支持 GB2312 字符串显示 |
| 外部 RTC | `Driver/bsp_pcf8563` | PCF8563（IIC），闹钟中断 INT(PB5) → EXTI5 |
| 温度 | `Driver/bsp_ntc` | NTC + ADC 测温 |
| 片内 Flash | `Driver/bsp_flash` | 读写/擦除 |
| 电池充电流水灯 | `Driver/bsp_battery_flows` | 充电流水灯效果（状态机，不含电量计量） |

> 引脚级对照（含原理图与 PCB 差异、已发现的 3 个冲突）见 `Doc/硬件信息汇总.md`。

---

## FreeRTOS 集成

### 内核与配置

| 项 | 值 |
|---|---|
| 版本 | V202604.01-LTS（`Middleware/FreeRTOS`） |
| 端口 | `portable/GCC/ARM_CM4F/port.c`（浮点上下文保存，AC6 亦可编译） |
| 内存 | `portable/MemMang/heap_4.c`，`configTOTAL_HEAP_SIZE = 75KB` |
| 调度 | 抢占式，`configTICK_RATE_HZ = 1000`，`configMAX_PRIORITIES = 5` |
| 同步 | mutex / 递归 mutex / 计数信号量 / 软件定时器 均开启 |
| 中断优先级 | `configPRIO_BITS = 4`，可调用 API 的最高优先级 `5` |
| 异常映射 | `vPortSVCHandler → SVC_Handler`、`xPortPendSVHandler → PendSV_Handler`、`xPortSysTickHandler → SysTick_Handler` |

### 双模式开关

`SYS_SUPPORT_OS` 宏决定走裸机还是 RTOS 代码路径，**Keil 与 EIDE 两处都已定义**：

- `User/main.c`：`#ifndef SYS_SUPPORT_OS` → `systick_config()` 才初始化 SysTick；RTOS 下由内核接管。
- `User/gd32f4xx_it.c`：`#ifndef SYS_SUPPORT_OS` → 裸机版 `SVC/PendSV/SysTick_Handler`；否则编译 `vApplicationTickHook()`。

若要临时回到裸机版本，只需去掉该宏（同时确认二者不会重复定义中断向量）。

### 任务划分（当前）

| 任务 | 栈 | 优先级 | 职责 |
|---|---|---|---|
| `app_task` | 512 words | `tskIDLE_PRIORITY + 3` | 循环执行 `task_schedule()` + `vTaskDelay(1ms)`，等价于原裸机 `while(1)` |
| `task_led1` | 64 words | 2 | 1s 翻转 LED1（演示） |
| `task_led2` | 64 words | 2 | 100ms 翻转 LED2（演示） |

`Middleware/taskmanager.c` 的周期任务表（触摸 10ms、数码管 10ms、点阵 10ms、188 数码管 20ms、
IIC OLED 100ms、PCF8563 1000ms）目前仍由 `app_task` 驱动，**尚未拆分为独立任务**（代码内有 TODO）。

### 节拍与延时

- 1ms 节拍由内核 SysTick 提供；`vApplicationTickHook()` 内完成：
  `delay_tick_update()`（µs 时间戳累加）+ `bsp_188_leds_scan()` + `bsp_adkey_scan()` + `bsp_digital_leds_scan()` + `task_switch()`。
  **tick hook 必须保持轻量、不可阻塞、不可调用非 ISR 版 API。**
- µs 级延时使用 **TIMER6 1MHz 自由运行计数器**（`User/systick.c`），不依赖 DWT，
  调度器启动前后与中断内均可使用。改用 DWT 延时会死机，详见 `Doc/FreeRTOS移植后卡死问题与修复.md`。
- `get_us_cnt()` 由 tick hook 每 1ms 累加，精度为毫秒级，适用于超时判断与帧间隔。

---

## 开发环境与构建

### 方式一：Keil MDK（主）

1. 打开 `Project/GD32F407_myLibrary.uvprojx`（target 名：`GD32F407`）。
2. 需要已安装 `GigaDevice.GD32F4xx_DFP`（3.2.0）与 ARM Compiler 6。
3. Build（F7）→ 产物 `Project/Objects/My_GD32F407_Template.axf` / `.hex`（已开启 Create HEX File）。
4. 下载：在 `Options → Debug / Utilities` 里选好调试器（J-Link / ST-Link / GD-Link）后 Download。

工程内联选项：

| 项 | 值 |
|---|---|
| 器件 | GD32F407VE |
| Define | `USE_STDPERIPH_DRIVER, GD32F407, HXTAL_VALUE=8000000, SYS_SUPPORT_OS` |
| 分组 | `User` / `App` / `Middleware` / `Driver` / `Firmware` / `CMSIS` / `Hardware` / `FreeRTOS_core` / `FreeRTOS_port` |
| 链接 | 自动生成分散加载，ROM `0x08000000` / RAM `0x20000000` |

### 方式二：EIDE（VS Code）

`.eide/eide.yml` 与 Keil 工程保持同样的文件列表、宏与 include 路径，输出到 `build/GD32F407`。
用 VS Code 打开工程根目录，EIDE 面板选择目标后 Build 即可。

> **重要：新增/删除源文件、修改宏或 include 路径时，`Project/*.uvprojx` 与 `.eide/eide.yml` 必须同步修改**，
> 否则一套能编、另一套编不过。

---

## 启动流程

`User/main.c` 的执行顺序：

```
nvic_priority_group_set(PRE2_SUB2)
rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4)   // APB1 定时器时钟 = 168MHz
[裸机] systick_config()                                 // RTOS 下跳过，内核接管 SysTick
msp_uart_init()  → printf 可用
msp_exti_init() / msp_rtc_init(HXTAL) / msp_iic_init() / msp_adc_init() / msp_spi_init()
bsp_leds / keys / buzzer / iic_oled / spi_oled / pcf8563 / flash / ntc / 188_leds
    / adkey / ec12d / digital_leds / touch_key / dot_matrix  init()
app_input_init() / app_show_init()
app_test_*()                                            // 上电外设自测
xTaskCreate(app_task / task_led1 / task_led2)
vTaskStartScheduler()                                   // 正常不会返回
```

---

## 代码规范与注意事项

1. **分层职责**：`User` 只做入口编排；业务放 `App`；可复用中间件放 `Middleware`；
   片上外设（GPIO/时钟/中断）只在 `Hardware/msp_*.c` 配置；片外器件驱动写 `Driver/bsp_*.c`。
2. **不要在 `main.c` 或业务代码里直接操作寄存器**，也不要跨层直接改 `msp_*` 的静态变量。
3. **中断里只置标志/计数**，耗时逻辑（尤其是 IIC/OLED 刷屏）放到任务；共享的软件 IIC 总线不能被中断重入
   （参见 `Doc/闹钟IIC重入问题与修复.md`）。
4. **源文件编码必须保持 GB2312/GB18030 + CRLF**（Keil 中文注释依赖此编码）。
   用 VS Code 编辑时注意状态栏编码，不要以 UTF-8 直接保存覆盖。
5. **定时器时钟假设**：`RCU_TIMER_PSC_MUL4` 下 APB1 定时器时钟 = `SystemCoreClock`（168MHz）。
   该假设同时出现在 `User/systick.c`（TIMER6 分频）与 `Driver/bsp_buzzer/bsp_buzzer.c`（PWM 频率），
   若改为 `MUL2` 或修改主频，两处必须一起改。
6. **RAM 预算**：`heap_4` 独占 75KB，其余为静态变量、任务栈与主栈；新增任务前先评估。

---

## 已知遗留与待办

| 事项 | 说明 |
|---|---|
| `main.c` 临时诊断打印 | `[boot] SystemCoreClock`、`[boot] DWT_CYCCNT`、`[init] xxx ok` 为定位卡死问题所加，问题确认后可删除 |
| 周期任务未拆分 | `taskmanager` 的任务表仍由 `app_task` 轮询驱动，计划逐个迁移为独立 FreeRTOS 任务 |
| tick hook 负载偏重 | 1ms 内做 3 路扫描 + 任务计数，建议后续改为独立任务或软件定时器 |
| `get_us_cnt()` 64 位读非原子 | 建议加 PRIMASK 临界区 |
| 主栈偏小 | `startup_gd32f407_427.s` 的 `Stack_Size = 0x400`，建议提到 `0x800` |
| `on_rtc_alarm` 重复定义 | `msp_rtc.c` 与 `bsp_pcf8563.c` 各一份，链接告警 `L6439W` |
| FreeRTOS `portable/` 未精简 | 官方全部移植目录都在仓库内，实际只编译 `GCC/ARM_CM4F` + `MemMang/heap_4.c` |
| 未开启健壮性检查 | `configCHECK_FOR_STACK_OVERFLOW = 0`、`configUSE_MALLOC_FAILED_HOOK = 0` |
| 软件 IIC 无总线互斥 | IIC OLED、PCF8563、SC12B 共用 PB6/PB7，目前靠「中断只置标志、处理放任务」规避重入，未使用互斥量 |
| 引脚复用待核对 | PB10 配成 USART2_TX、PC3 兼作 KEY4/EXTI3、PB8 作触摸 INT，与硬件文档记录存在复用，详见 `Doc/硬件信息汇总.md` §5 |
| 软/硬两套实现并存 | `msp_iic_hard.c`、`msp_spi_hard.c` 与软实现同时存在，改动前先确认当前工程实际编译的是哪一套 |

---

## 文档索引

| 文档 | 内容 |
|---|---|
| `Doc/硬件信息汇总.md` | 扩展板 BOM、80 引脚对照、代码与硬件一致性校验结果 |
| `Doc/引脚检查测试.md` | 引脚检查记录 |
| `Doc/整理操作参考.md` | `main.c` 分层重构的操作指南与验证清单 |
| `Doc/闹钟IIC重入问题与修复.md` | PCF8563 闹钟中断重入软件 IIC 导致通信失败的分析 |
| `Doc/FreeRTOS移植后卡死问题与修复.md` | 移植后上电卡死的完整排障与 TIMER6 延时方案 |

---

## 常见问题

| 现象 | 排查方向 |
|---|---|
| 编译报找不到新加的 `.c` | `Project/*.uvprojx` 与 `.eide/eide.yml` 未同步添加 |
| 串口无任何输出 | `msp_uart_init()` 是否调用、`printf` 是否重定向到 USART、波特率 |
| 上电停在初始化、无 `start` 打印 | 见 `Doc/FreeRTOS移植后卡死问题与修复.md`；确认 `SYS_SUPPORT_OS` 与延时实现 |
| 任务创建失败 / 跑不起来 | `heap_4` 剩余空间不足（默认 75KB），或任务栈设置过大 |
| 显示刷新变慢、按键迟钝 | tick hook 或任务内阻塞过久（IIC 软件延时、OLED 整屏刷新约 38ms） |
| 烧录后必须按复位才运行 | 检查调试器 Download 设置里的 `Reset and Run` |

---

## 提交信息约定

沿用仓库历史格式：`[类型]: 模块 描述`，常用类型 `功能` / `修复` / `更新` / `优化` / `整理` / `合并` / `文档`。

```
[修复]: systick TIMER6 微秒延时在调度器启动前不可用的问题
[更新]: bsp_dot_matrix 点阵屏 数字取模
```

---

## 致谢

- GD32F4xx 标准外设库、CMSIS：GigaDevice Semiconductor
- FreeRTOS Kernel：Amazon.com, Inc.（MIT License）

