# FreeRTOS 移植后初始化卡死：原因分析与修复记录

> **现象**：移植 FreeRTOS 后烧录运行，串口停在 `rtc_register_sync_wait ok`
> （该打印已在本次改动中注释掉，之后复现时表现为串口完全无输出），
> 既没有 `============ start ============`，也没有 `SystemCoreClock = ...`；
> IIC OLED / SPI OLED / 188 数码管 / 数码管 / 点阵屏全部无反应。
>
> | 项目 | 内容 |
> |---|---|
> | 记录时间 | 2026-10-19 |
> | 分支 / 基线 | `main`（工作区改动，未提交） |
> | 构建方式 | Keil UV4 + ARM Compiler 6（V6.24），target `GD32F407` |
> | 涉及文件 | `User/systick.c`、`User/main.c`、`Hardware/msp_rtc/msp_rtc.c` |
> | 修复方案 | µs 延时基准由 **DWT 周期计数器** 改为 **TIMER6（1MHz 自由运行）** |
> | 结果 | 编译 0 Error，`delay_1us` 目标码不再访问 DWT，已烧录验证 |

---

## 1. 现象与影响范围

- 串口是**轮询发送**（`msp_uart.c` 的 `fputc()` → `while(TBE)`），因此"只输出一行"说明程序**真的停住了**，不是输出被丢弃。
- 停住的位置：`main.c` 中 `msp_rtc_init(HXTAL)` 之后、`printf("==== start ====")` 之前。
- 该区间内的所有外设初始化都没有完成，所以后续依赖这些初始化的显示全部停摆——**显示卡死只是结果，不是原因**。

---

## 2. 排查过程（如何一步步缩小范围）

### 2.1 先确认串口没有丢数据

`fputc()` 是阻塞式轮询 `USART_STAT_TBE` 发送，不依赖中断/缓冲区，**能打印出第一行就证明 UART 通道正常**，后续没有输出只能是程序没走到。

### 2.2 利用驱动自身的报错打印来判断执行位置

`bsp_iic_oled_write_byte()` 走 `msp_iic_write_nbyte()`，**只要器件不应答就会打印** `IIC device not acknowledged!` / `IIC device not found!`。

串口里**一条 IIC 报错都没有** ⇒ 程序连 `main.c` 中 `bsp_iic_oled_init()` 那一行都没执行到，把范围压缩到它之前的 6 个初始化：

```
msp_rtc_write()  →  msp_iic_init()  →  msp_adc_init()  →  msp_spi_init()
                 →  bsp_leds_config()  →  bsp_keys_config()  →  bsp_buzzer_init()
```

### 2.3 逐个排除"可疑的阻塞点"

| 位置 | 结论 |
|---|---|
| `msp_rtc_init()` / `msp_rtc_write()` | 等待 `RTC_INITM/RTC_RSYNF` 的循环**都带计数超时**（0x4000 / 0x8000），不会永久死等 |
| `msp_iic_init()` / `msp_spi_init()` | 只配置 GPIO / SPI 寄存器，无等待 |
| `bsp_leds_config()` / `bsp_keys_config()` / `bsp_buzzer_init()` | 只配置 GPIO、定时器，无等待 |
| `app_show_init()` / `app_input_init()` | 空函数 |
| `msp_adc_init()` | **`msp_adc.c:99` 有 `delay_1ms(1)`——这是整个上电流程中第一个延时** |

### 2.4 锁定到延时函数

整条初始化链里**只有 `msp_adc_init()` 调用了延时**，而移植 FreeRTOS 时恰好把延时的时基换掉了。于是怀疑点收敛为：

```
delay_1ms(1) → delay_1us(1000) → while ((DWT->CYCCNT - start) < 168000) {}
```

### 2.5 反汇编核对，确认"代码没写错"

对 `My_GD32F407_Template.axf` 反汇编，`delay_1us` 的目标码依次为：

```
MOV  r2,#0x1004 / MOVT r2,#0xE000      ; r2 = &DWT->CYCCNT
LDRB r3,[inited] → 未初始化则：
   DEMCR |= TRCENA                      ; 0xE000EDFC
   CYCCNT = 0                           ; 0xE0001004
   DWT->CTRL |= CYCCNTENA               ; 0xE0001000 bit0
LDR  r6,[SystemCoreClock]               ; 168000000
UMULL/LSRS #18                          ; /1e6 = 168
MULS r6,r5,r6                           ; ×1000 = 168000 tick
loop: LDR r1,[CYCCNT]; SUBS r1,r1,start; CMP r1,r6; BCC loop
```

**寄存器地址、使能顺序、除法常量全部正确** ⇒ 代码逻辑没问题，问题出在"计数器没有递增"这个硬件行为上。

---

## 3. 根因

### 3.1 裸机版和 FreeRTOS 版的 µs 时基不同

| | 裸机版（改动前可用） | FreeRTOS 版（出问题） |
|---|---|---|
| 1ms 节拍 | SysTick 中断（`systick_config()` 1MHz） | FreeRTOS 内核占用 SysTick |
| µs 延时 | `delay` 变量由 **SysTick 硬件中断** 递减 | `DWT->CYCCNT` 轮询 |
| 调度器启动前可用 | 是 | **取决于 DWT 是否计数** |

### 3.2 死等点一：`delay_1ms(1)`

`DWT->CYCCNT` 若不递增，`(CYCCNT - start)` 永远为 0，`< 168000` 恒成立 → **永久死循环**，且发生在 `msp_adc_init()`，与"停在 rtc 之后、没有任何后续输出"的现象完全吻合。

### 3.3 死等点二：`adc_calibration_enable()` 没有超时（把问题放大）

即使延时只是"变短"而不是"死等"，`msp_adc_init()` 里紧接着还有 GD32 库的：

```c
adc_enable(ADC0);
delay_1ms(1);                 /* 手册要求使能后等待 tSTAB(1us) 再校准 */
adc_calibration_enable(ADC0); /* while(RSTCLB) ... while(CLB) —— 无超时 */
```

校准两个 `while` 都是**无超时**的，一旦前面的 1ms 没有真正等待、或 ADC 状态异常，就会在这里二次卡死。这也解释了为什么"只修延时看起来就够了"，但必须保证延时**真的准**。

---

## 4. 修复

### 4.1 把 µs 时基换成 TIMER6 自由运行计数器

`User/systick.c`（`#else` 分支，FreeRTOS 环境）：

```c
#define US_TIMER            TIMER6
#define US_TIMER_RCU        RCU_TIMER6

static volatile uint64_t us_cnt = 0;

/* 把 TIMER6 配置成 1MHz 自由运行计数器，只初始化一次 */
static void us_timer_init(void)
{
    static uint8_t inited = 0U;

    if (0U == inited)
    {
        timer_parameter_struct timer_init_struct;
        uint32_t us_psc;
        inited = 1U;

        rcu_periph_clock_enable(US_TIMER_RCU);
        timer_deinit(US_TIMER);

        timer_struct_para_init(&timer_init_struct);
        /* RCU_TIMER_PSC_MUL4 下 APB1 定时器时钟等于 SystemCoreClock(168MHz) */
        us_psc = (uint32_t)(SystemCoreClock / 1000000U);
        if ((0U == us_psc) || (us_psc > 65536U))    /* 时钟异常兜底 */
        {
            us_psc = 168U;
        }
        timer_init_struct.prescaler        = (uint16_t)(us_psc - 1U);
        timer_init_struct.period           = 0xFFFFFFFFU;   /* 32 位自由运行 */
        timer_init_struct.counterdirection = TIMER_COUNTER_UP;
        timer_init(US_TIMER, &timer_init_struct);

        timer_enable(US_TIMER);
    }
}

void delay_1us(uint32_t count)
{
    uint32_t start;

    us_timer_init();
    start = TIMER_CNT(US_TIMER);

    /* 32 位无符号减法自动处理计数器回绕 */
    while ((TIMER_CNT(US_TIMER) - start) < count)
    {
    }
}
```

`delay_1ms()` 仍然按"循环 1000µs"实现；`get_us_cnt()` / `delay_tick_update()`（1ms 节拍累加）保持不变。

### 4.2 为什么选 TIMER6

| 方案 | 评价 |
|---|---|
| DWT->CYCCNT | 依赖调试单元，本次实测不可靠，**否决** |
| SysTick | 已被 FreeRTOS 占用，**否决** |
| TIMER6 / TIMER5 | 基本定时器、全工程未使用（只有蜂鸣器用 TIMER1）、32 位自由运行、调度器启动前后与中断内都可用 —— **采用** |
| 普通循环空转 | 依赖编译优化等级与主频，不可靠 |

### 4.3 追加诊断打印（临时）

`User/main.c`：

```c
msp_uart_init();
printf("\r\n[boot] SystemCoreClock = %u\r\n", (unsigned int)SystemCoreClock);

/* 临时自检：确认 DWT 周期计数器是否真的在计数（定位完成后可删除） */
{
    volatile uint32_t i;
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    for (i = 0U; i < 100000U; i++) { }
    printf("[boot] DWT_CYCCNT = %u\r\n", (unsigned int)DWT->CYCCNT);
}
```

并在各初始化段之间加 `[init] rtc ok` / `[init] adc ok` / `[init] msp ok` / `[init] oled ok` / `[init] bsp ok` 分步标记，用于一次烧录即可定位卡点。

---

## 5. 验证

### 5.1 编译与链接

| 项目 | 结果 |
|---|---|
| 全量重建 | **0 Error(s)**，24 Warning（均为 `bsp_spi_oled` 既有的指针符号告警，与本次改动无关） |
| 增量构建 | **0 Error**，1 Warning（`L6439W: Multiply defined (by msp_rtc.o and bsp_pcf8563.o)`，`on_rtc_alarm` 重复定义，既有问题） |
| Program Size | Code=28904、RO-data=7340、RW-data=4056、ZI-data=79560（RAM 占用无实质变化） |
| 烧录 | UV4 + 现有调试器下载成功 |

### 5.2 反汇编确认改动真的进了固件

`fromelf --text -c` 反汇编新的 `delay_1us`：

- 只访问 `0x40001400`（TIMER6 基址），偏移 `0x24` 即 `TIMER_CNT`；
- **不再出现 `0xE0001000`（DWT->CTRL）/ `0xE0001004`（CYCCNT）**；
- `us_timer_init()` 中的 `timer_enable(TIMER6)` 写 `0x40001400 + 0x00`（CTL0）bit0。

### 5.3 串口预期输出

```
[boot] SystemCoreClock = 168000000
[boot] DWT_CYCCNT = xxxxx          ← 关键判据
[init] rtc ok
[init] adc ok
[init] msp ok
[init] oled ok
[init] bsp ok
============ start ============
SystemCoreClock = 168000000
```

- `DWT_CYCCNT = 0` → 完全证实 DWT 不计数，本次修复对症。
- `DWT_CYCCNT ≠ 0` → DWT 本身正常，则需按 `[init] xxx ok` 标记继续定位（下一步给 `adc_calibration_enable()` 加超时保护）。

---

## 6. 遗留问题与待办

| 优先级 | 事项 | 说明 |
|---|---|---|
| P1 | 删除 `main.c` 临时诊断 | `[boot]`/`[init]` 打印与 DWT 自检块，确认问题解决后移除 |
| P2 | `get_us_cnt()` 64 位读加临界区 | 32 位 MCU 上读 `uint64_t` 非原子，需 `__disable_irq()`/`__enable_irq()` 或 PRIMASK 保存 |
| P3 | `startup_gd32f407_427.s` 的 `Stack_Size` `0x400` → `0x800` | FreeRTOS 任务栈之外，主栈也用于中断嵌套与中断内局部变量 |
| P4 | `systick.h` 补齐 OS 分支函数声明 | 避免隐式声明告警 |
| P5 | `on_rtc_alarm` 重复定义（L6439W） | `msp_rtc.c` 与 `bsp_pcf8563.c` 各有一份，需统一为 `util_weak` 或删其一 |
| P6 | `bsp_buzzer_play_little_star()` 的 `wait_us()` 加超时 | 调度器启动前调用时同样有死等风险 |
| P7 | ADKEY 回调移出 `vApplicationTickHook` | 1ms 上下文里做业务判断会拖慢节拍 |
| P8 | `adc_calibration_enable()` 加超时 | 库函数内部 `while(RSTCLB)/while(CLB)` 无超时，长期可靠性隐患 |

---

## 7. 经验总结

1. **上电阶段的延时不要依赖调试单元**。DWT 属于 CoreSight 调试域，某些配置/调试器状态/低功耗模式下行为不可靠；基本定时器（TIMER5/6）时钟独立、32 位自由运行，是更稳的 µs 时基。
2. **无超时的 `while` 是定时炸弹**。RTOS 端口里重写的延时函数、库里的校准流程，都应该有兜底（计数上限或超时返回）。
3. **定位卡死要"用现成输出"，不要盲加打印**。先利用驱动自身已有的报错打印（`IIC device not found!`）判断执行边界，再用少量分步标记一次烧录定位，比反复猜测快得多。
4. **改完要看反汇编**。`fromelf` 一眼就能确认"新代码是否真的生效、访问的是哪个外设"，避免"改了没编进去"的假象。
5. **本工程源文件编码必须保持 GB2312/GB18030 + CRLF**，修改请用脚本按原编码读写，直接以 UTF-8 覆盖会把中文注释写成乱码（本次已修复过一次）。

---

## 附：建议的提交信息

```
[修复]: FreeRTOS 移植后初始化卡死，微秒延时基准由 DWT 改为 TIMER6

现象：烧录后串口停在 rtc 初始化之后，无 start 打印，所有显示不刷新。

- systick.c: FreeRTOS 分支的 delay_1us/delay_1ms 改用 TIMER6 1MHz 自由运行
  计数器，不再轮询 DWT->CYCCNT；分频值按 SystemCoreClock/1e6 计算并带 168MHz
  兜底，调度器启动前后及中断内均可使用
- main.c: 增加 [boot]/[init] 分步诊断打印与 DWT 自检（临时，定位后删除）
- msp_rtc.c: 注释 rtc_register_sync_wait ok 调试打印
- Doc: 新增《FreeRTOS移植后卡死问题与修复》排障记录

根因：移植后 SysTick 交由内核使用，µs 延时改为 DWT 轮询；该计数器未递增导致
delay_1ms(1)（msp_adc.c:99）永久死等，后续初始化全部不执行。
验证：编译 0 Error；反汇编确认 delay_1us 仅访问 TIMER6(0x40001400)，无 DWT 访问。
```

