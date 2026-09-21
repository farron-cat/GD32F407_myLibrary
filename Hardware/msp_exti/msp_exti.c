#include "msp_exti.h"

static void EXTI_config(rcu_periph_enum rcu, uint32_t port, uint32_t pin, uint32_t pupd,
                        uint8_t exti_port, uint8_t exti_pin, exti_line_enum linex,
                        exti_trig_type_enum trig_type, uint8_t nvic_irq)
{
    // 1. 配置引脚GPIO模式
    rcu_periph_clock_enable(rcu);
    gpio_mode_set(port, GPIO_MODE_INPUT, pupd, pin);

    // 2. 选择EXTI检测线路连接引脚(SYSCFG)
    rcu_periph_clock_enable(RCU_SYSCFG);
    syscfg_exti_line_config(exti_port, exti_pin);
    // 3. 配置EXTI检测极性(上升沿/下降沿/BOTH触发)
    exti_init(linex, EXTI_INTERRUPT, trig_type);
    // 4. 告诉NVIC要处理EXTI中断
    nvic_irq_enable(nvic_irq, 2, 2);

    // 清理中断标志位
    exti_interrupt_flag_clear(linex);
    // 5. 打开EXTI中断
    exti_interrupt_enable(linex);
}

void msp_exti_init()
{
// 初始化PA0
#if USE_EXTI0

    EXTI_config(EXTI0_PORT_RCU, EXTI0_PORT, EXTI0_PIN, EXTI0_PUPD, EXTI0_EXTI_PORT,
                EXTI0_EXTI_PIN, EXTI0_NUM, EXTI0_TRIG_TYPE, EXTI0_IRQ);
#endif

#if USE_EXTI3
    // 初始化PC3
    EXTI_config(EXTI3_PORT_RCU, EXTI3_PORT, EXTI3_PIN, EXTI3_PUPD, EXTI3_EXTI_PORT,
                EXTI3_EXTI_PIN, EXTI3_NUM, EXTI3_TRIG_TYPE, EXTI3_IRQ);
#endif
}

// EXTI0中断处理函数
#if USE_EXTI0

void EXTI0_IRQHandler()
{

    static FlagStatus pre_sta = RESET;
    static uint64_t pre_cnt = 0;

    if (exti_interrupt_flag_get(EXTI0_NUM) == SET)
    {
        exti_interrupt_flag_clear(EXTI0_NUM);

        // 消抖
        uint64_t cur_cnt = get_us_cnt();
        if (cur_cnt - pre_cnt <= 30000)
        {
            return;
        }
        pre_cnt = cur_cnt;
        FlagStatus cur_sta = gpio_input_bit_get(EXTI0_PORT, EXTI0_PIN);
        if (cur_sta == pre_sta)
        {
            return;
        }
        pre_sta = cur_sta;
        // 按下和松开判断
        if (cur_sta == SET)
        { // 上升沿  按下
            printf("KEYC_D\n");
        }
        else
        { // 下降沿  松开
            printf("KEYC_U\n");
        }
    }
}

#endif

// EXTI3中断处理函数
#if USE_EXTI3

void EXTI3_IRQHandler()
{
    static FlagStatus pre_sta = SET;
    static uint64_t pre_cnt = 0;

    if (exti_interrupt_flag_get(EXTI_3) == SET)
    {
        exti_interrupt_flag_clear(EXTI_3);

        // 获取当前时间
        uint64_t cur_cnt = get_us_cnt();
        if (cur_cnt - pre_cnt <= 30000)
        {
            return;
        }
        pre_cnt = cur_cnt;
        FlagStatus cur_sta = gpio_input_bit_get(GPIOC, GPIO_PIN_3);
        if (cur_sta == pre_sta)
        {
            return;
        }
        pre_sta = cur_sta;
        // 按下和松开判断
        if (cur_sta == RESET)
        { // 下降沿  按下
            printf("KEY4_D\n");
        }
        else
        { // 上升沿  松开
            printf("KEY4_U\n");
        }
    }
}

#endif
