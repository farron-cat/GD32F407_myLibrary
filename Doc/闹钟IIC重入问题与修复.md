# 闹钟触发后 IIC 报错：原因分析与修复记录

> **现象**：PCF8563 闹钟每次触发后，串口都会打印
> `IIC device not acknowledged!` / `IIC device not found!`，闹钟回调虽能执行但 IIC 通信被破坏。
>
> | 项目 | 内容 |
> |---|---|
> | 记录时间 | 2026-10-10 |
> | 分支 / 基线 | `main`（工作区改动，未提交） |
> | 构建方式 | EIDE + ARM Compiler 6（AC6），target `GD32F407` |
> | 涉及文件 | `Hardware/msp_exti/msp_exti.c`、`Hardware/msp_exti/msp_exti.h`、`Driver/bsp_pcf8563/bsp_pcf8563.c`、`Driver/bsp_pcf8563/bsp_pcf8563.h`、`User/main.c` |
> | 修复方案 | 方案 A：中断只置标志，IIC 处理移到主循环 |

---

## 1. 现象

- 闹钟未到点时：RTC 时间读取、OLED 刷新、串口打印一切正常。
- 闹钟到点触发后，串口立刻出现：

  ```
  IIC device not acknowledged!
  IIC device not found!
  ```

  （两条通常连在一起出现，之后系统继续运行）
- **每个闹钟周期必现**，不是偶发。

---

## 2. 触发链路

```
                    ┌──────────────────── 主循环（while(1)）────────────────────┐
                    │  bsp_pcf8563_read_time(&time);   ← 软件 IIC 读 7 字节      │
                    │  oled_show_rtc(&time);           ← 软件 IIC 整屏刷新(~38ms)│
                    └───────────────────────────────────────────────────────────┘
                                        ▲
                                        │ 任意时刻被打断
                                        │
PCF8563 闹钟到点 → INT(PB5) 拉低（低有效）
   └─ EXTI5 下降沿
        └─ EXTI5_9_IRQHandler()                        [Hardware/msp_exti/msp_exti.c]
             ├─ exti_interrupt_flag_clear(EXTI_5)
             └─ on_rtc_int()                           [Driver/bsp_pcf8563/bsp_pcf8563.c]
                  ├─ msp_iic_read_nbyte(0x51, 0x01, ...)   ← 在这里发起 IIC
                  ├─ bsp_pcf8563_alarm_clear()             ← 又一次 IIC（读+写）
                  └─ on_rtc_alarm() → printf("alarm")      ← 还做了串口打印
```

**关键**：主循环和 EXTI5 中断访问的是**同一条软件 IIC 总线（PB6/PB7）**，而中断会在**任意时刻**打断主循环正在进行的 IIC 字节序列。

---

## 3. 根因分析

### 3.1 用的是「软件 IIC」

`Hardware/msp_iic/msp_iic.h`：

```c
#define IIC_HARD_SOFT_SWITCH 0   // 0 = 软件 IIC
```

软实现的每一位都靠 GPIO 翻转 + 阻塞延时：

```c
#define IIC_DELAY   delay_1us(2)     // 一个 SCL 半周期约 2µs
```

而 `delay_1us()` 依赖 **SysTick 1µs 中断** 递减计数（`User/systick.c`）：

```c
SysTick_Config(SystemCoreClock / 1000000U);   // 1µs
void delay_1us(uint32_t count){ delay = count; while (0U != delay) {} }
void delay_decrement(void){ us_cnt++; if (0U != delay) delay--; }
```

→ 软 IIC 的**时序完全由 CPU 逐位控制**，一旦被打断，位与位之间的间隔立刻错乱。

### 3.2 OLED 与 PCF8563 共用同一条总线、同一份代码，且没有互斥

`Driver/bsp_iic_oled/bsp_iic_oled.c` 里的 OLED 写字节最终也是走 `msp_iic`：

```c
void bsp_iic_oled_write_byte(uint8_t dat, uint8_t mode)
{
    if (mode) msp_iic_write_nbyte(0x78 >> 1, 0x40, &dat, 1);
    else      msp_iic_write_nbyte(0x78 >> 1, 0x00, &dat, 1);
}
```

（文件里旧的那套手写 `I2C_Start/Send_Byte` 已被注释，未参与编译。）

所以：**PCF8563(0x51) 与 SSD1306(0x3C) 共用 PB6/PB7，共用 `msp_iic`，但没有任何"总线忙"保护。**

### 3.3 中断里直接发起完整 IIC 事务

`on_rtc_int()` 是在 **EXTI5 中断上下文**里被调用的，它做了三件"重活"：

1. 读 `0x01` 状态寄存器（完整 IIC 事务）；
2. 调 `bsp_pcf8563_alarm_clear()`（又一次 IIC 读 + 写）；
3. 调 `on_rtc_alarm()` → `printf`（USART 阻塞发送）。

当这个中断落进主循环正在传输的 IIC 时序中间时，总线上会**突然插入一次新的 START 序列**，从机无法解析当前帧 → 应答位保持高 → `msp_iic_wait_ack()` 返回非 0。

### 3.4 为什么「每次都报」

主循环每一轮都要 `oled_show_rtc()` → 整屏 128×64 刷新，全是软件 IIC，耗时约 **38ms**（见 `note/18-OLED显示屏.md`）。
也就是说主循环**几乎 100% 的时间都在 IIC 传输中**，闹钟中断撞上"传输进行中"的概率接近 100%，所以必现。

### 3.5 为什么「连报两条」

旧代码把状态缓存放在静态变量 `sta_c2` 里，且**不检查读取是否成功**：

```c
msp_iic_read_nbyte(PCF8563_ADDR, 0x01, &sta_c2, 1);   // 失败时 sta_c2 仍是旧值
if ((sta_c2 >> 3) & 0x01) { bsp_pcf8563_alarm_clear(); on_rtc_alarm(); }  // 继续执行 → 再发一次 IIC → 再报错
```

读失败后仍用旧值判断，于是"错上加错"，打印出第二条。

---

## 4. 方案对比与选型

| 方案 | 做法 | 结论 |
|---|---|---|
| **A. 中断只置标志（采用）** | `EXTI5_9_IRQHandler` 只清 EXTI 标志 + 置 `volatile` 全局标志；主循环检测到标志后再调 `on_rtc_int()`。让 IIC 永远只在主循环（单线程）访问 | ✅ 彻底消除重入；改动小；符合 `README.md`「中断里只做标志位处理」 |
| B. IIC 加「忙」互斥 | 在 `msp_iic` 软实现里加 `volatile uint8_t iic_busy`，中断里发现忙就只置标志 | 可选加固；但需保证 OLED/PCF8563 都过同一把锁 |
| C. 关中断临界区 | 传输期间 `__disable_irq()` | ❌ **不可行**：`delay_1us()` 依赖 SysTick 中断递减，关中断会让 `while(delay)` 死锁 |

**选择 A**，并按需在第 7 节预留 B 作为后续加固。

---

## 5. 改动记录

共改 5 个文件（其中 4 处逻辑/声明，1 处为注释）。

### 5.1 `Hardware/msp_exti/msp_exti.c` —— 中断只置标志

```diff
-extern void on_rtc_int();
-
+// RTC(PCF8563) 中断标志：中断只置位，IIC 处理交给主循环，避免打断主循环正在进行的软件 IIC 时序
+volatile uint8_t g_rtc_int_flag = 0;
+
 void EXTI5_9_IRQHandler()
 {
     if (exti_interrupt_flag_get(EXTI_5) == SET)
     {
         exti_interrupt_flag_clear(EXTI_5);
 
-        // RTC回调
-        on_rtc_int();
+        // 只置标志，不在中断里做 IIC / printf
+        g_rtc_int_flag = 1;
     }
 }
```

### 5.2 `Hardware/msp_exti/msp_exti.h` —— 导出标志

```diff
+// RTC(PCF8563) 中断标志：EXTI5 中断只置位，主循环清零并处理（避免软件 IIC 重入）
+extern volatile uint8_t g_rtc_int_flag;
+
 void msp_exti_init();
```

### 5.3 `Driver/bsp_pcf8563/bsp_pcf8563.c` —— `on_rtc_int` 加读失败检查 + 改用局部变量

```diff
-// EXTI 中断服务函数
-// 由外部 PCF8563 INT 引脚触发的 EXTI 中断调用
+// PCF8563 中断事件处理
+// 不再在 EXTI5 中断里执行，改由主循环检测到 g_rtc_int_flag 后调用（避免软件 IIC 重入）
 void on_rtc_int(void)
 {
-    // printf("exti5\n");
-
-    msp_iic_read_nbyte(PCF8563_ADDR, 0x01, &sta_c2, 1);
+    uint8_t sta = 0;
+
+    // 读状态控制寄存器 0x01；读取失败直接返回，避免用旧缓存值误判
+    if (msp_iic_read_nbyte(PCF8563_ADDR, 0x01, &sta, 1) != IIC_SUC)
+        return;
 
     // 判断 AF 标志位
-    if ((sta_c2 >> 3) & 0x01)
+    if ((sta >> 3) & 0x01)
     {
         bsp_pcf8563_alarm_clear();
         on_rtc_alarm();
     }
 
     // 判断 TF 标志位
-    if ((sta_c2 >> 2) & 0x01)
+    if ((sta >> 2) & 0x01)
     {
         bsp_pcf8563_timer_clear();
         on_rtc_timer();
     }
 }
```

> 说明：静态缓存 `sta_c2` 在其他函数（`alarm_enable` / `alarm_clear` / `timer_*`）中仍在使用，未删除；这里只是不再用它做中断里的判断。
> `IIC_SUC` 来自 `msp_iic.h`（`bsp_pcf8563.c` 已 include）。

### 5.4 `Driver/bsp_pcf8563/bsp_pcf8563.h` —— 更新函数说明注释

```diff
-void on_rtc_int(void); // EXTI5 中断入口调用
+void on_rtc_int(void); // 主循环检测到 g_rtc_int_flag 后调用
```

### 5.5 `User/main.c` —— 主循环处理标志

```diff
     while (1)
     {
+        // PCF8563 中断事件处理：原在 EXTI5 中断中执行，移到主循环以避免软件 IIC 重入
+        if (g_rtc_int_flag)
+        {
+            g_rtc_int_flag = 0;
+            on_rtc_int();
+        }
+
         // 188数码管显示NTC温度
         bsp_188_leds_set_num(bsp_ntc_get_tem());
```

> `main.c` 原本为 **GBK** 编码，本次改动**保持编码不变**（无 BOM），插入的中文注释已按 GBK 写入，不会引入乱码。

**改动后的数据流**

```
EXTI5 中断：  clear flag → g_rtc_int_flag = 1            （微秒级，不碰总线）
主循环：      if(flag){ flag=0; on_rtc_int(); }          （IIC 单线程访问，安全）
```

---

## 6. 验证

### 6.1 编译（已通过）

用 EIDE 的构建器 `unify_builder` 复现构建，增量编译命中本次改动的 3 个 `.c` 文件：

```
[2026-10-10 21:19:44] incremental build: 3 source files changed
  Driver\bsp_pcf8563\bsp_pcf8563.c : source file has been changed.
  Hardware\msp_exti\msp_exti.c     : source file has been changed.
  User\main.c                      : source file has been changed.
[2026-10-10 21:19:46] [done]
        build successfully !
```

产物更新：`build/GD32F407/.obj/.../bsp_pcf8563.o`、`msp_exti.o`、`main.o` 时间戳均为 21:19。

`main.c` 编码检查：文件头字节 `35,105,110`（`#in`），**无 BOM**，编码未变。

### 6.2 上板验证（待执行）

| 步骤 | 期望结果 |
|---|---|
| 烧录后正常启动 | 串口打印 `============ start ============` |
| 让闹钟触发 | 串口**只**出现 `alarm`，**不再**出现 `IIC device not found!` / `IIC device not acknowledged!` |
| 连续等待 ≥2 个闹钟周期 | 每个周期都能正常触发（说明 AF 被及时清除、下降沿未丢） |
| 观察 OLED | 显示正常、时间走秒不乱 |

> 说明：本文档记录的验证到「编译通过」为止；上板现象请按上表实测确认。

---

## 7. 遗留与后续建议

1. **约定：中断里不碰软件 IIC**
   本次通过「中断只置标志」绕开了重入。请把这条写进规范：**任何中断服务函数里都不要发起软件 IIC 传输**（也不要 `printf`）。将来若新增触摸屏 INT、其他传感中断，同样遵守。

2. **可选加固：给软 IIC 加「忙」互斥（方案 B）**
   在 `msp_iic` 软实现入口/出口维护 `volatile uint8_t iic_busy`；中断里若检测到 `iic_busy` 就只置标志、不发起 IIC。适合将来"确实需要在中断里访问 IIC"的场景。

3. **注意 `bsp_pcf8563_alarm_clear()` / `timer_clear()` 的健壮性**
   它们内部仍是「读 → 改位 → 写回」，读失败时会用旧缓存写回。当前因只在主循环调用、总线不再被打断，风险很低；如追求严谨，可同样加返回值检查。

4. **总线层面的根因仍在**
   PCF8563 与 OLED 共用 PB6/PB7 且共用 `msp_iic`、无锁。本次修复的是"并发访问"这一触发条件，并未改变"共总线无互斥"的事实。若后续把 OLED 换成 SPI 版，则 IIC 上只剩 PCF8563，冲突面进一步缩小。

5. **相关笔记**：`note/17-PCF8563外部RTC.md`（INT→EXTI5→on_rtc_int 链路）、`note/18-OLED显示屏.md`（软 I2C 刷新耗时 ~38ms）、`note/06-外部中断EXTI.md`（EXTI 处理约定）。

---

### 附：改动文件索引

| 文件 | 改动性质 |
|---|---|
| `Hardware/msp_exti/msp_exti.c` | 中断只置 `g_rtc_int_flag`，移除 `on_rtc_int()` 调用 |
| `Hardware/msp_exti/msp_exti.h` | `extern volatile uint8_t g_rtc_int_flag;` |
| `Driver/bsp_pcf8563/bsp_pcf8563.c` | `on_rtc_int()` 改局部变量 + 检查 `IIC_SUC` + 更新注释 |
| `Driver/bsp_pcf8563/bsp_pcf8563.h` | `on_rtc_int()` 说明注释 |
| `User/main.c` | 主循环新增标志处理（GBK 编码保持） |
