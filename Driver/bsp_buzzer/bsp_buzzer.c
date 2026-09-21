#include "bsp_buzzer.h"

// ---------- 音名频率（C 大调，单位 Hz） ---------- */
#define NOTE_REST 0
#define NOTE_DO 524
#define NOTE_RE 588
#define NOTE_MI 660
#define NOTE_FA 698
#define NOTE_SOL 784
#define NOTE_LA 880
#define NOTE_SI 988
#define NOTE_DO_H 1048 // 高音 do

/* ---------- 节拍 ---------- */
// 120 BPM：一拍 = 60/120 = 0.5s = 500000us
#define BEAT_US 500000u
// 音符之间留的间隔，避免连音
#define NOTE_GAP_US 20000u

/* ---------- 音符结构 ---------- */
typedef struct
{
    uint16_t freq; // 频率，NOTE_REST 表示休止
    uint8_t beats; // 拍数：1=四分音符，2=二分音符
} note_t;

/* ---------- 《小星星》简谱 ----------
   1 1 5 5 | 6 6 5 - | 4 4 3 3 | 2 2 1 - |
   5 5 4 4 | 3 3 2 - | 5 5 4 4 | 3 3 2 - |
   1 1 5 5 | 6 6 5 - | 4 4 3 3 | 2 2 1 - |
------------------------------------------ */
static const note_t little_star[] = {
    // 1 1 5 5 6 6 5 -
    {NOTE_DO, 1},
    {NOTE_DO, 1},
    {NOTE_SOL, 1},
    {NOTE_SOL, 1},
    {NOTE_LA, 1},
    {NOTE_LA, 1},
    {NOTE_SOL, 2},

    // 4 4 3 3 2 2 1 -
    {NOTE_FA, 1},
    {NOTE_FA, 1},
    {NOTE_MI, 1},
    {NOTE_MI, 1},
    {NOTE_RE, 1},
    {NOTE_RE, 1},
    {NOTE_DO, 2},

    // 5 5 4 4 3 3 2 -
    {NOTE_SOL, 1},
    {NOTE_SOL, 1},
    {NOTE_FA, 1},
    {NOTE_FA, 1},
    {NOTE_MI, 1},
    {NOTE_MI, 1},
    {NOTE_RE, 2},

    // 5 5 4 4 3 3 2 -
    {NOTE_SOL, 1},
    {NOTE_SOL, 1},
    {NOTE_FA, 1},
    {NOTE_FA, 1},
    {NOTE_MI, 1},
    {NOTE_MI, 1},
    {NOTE_RE, 2},

    // 1 1 5 5 6 6 5 -
    {NOTE_DO, 1},
    {NOTE_DO, 1},
    {NOTE_SOL, 1},
    {NOTE_SOL, 1},
    {NOTE_LA, 1},
    {NOTE_LA, 1},
    {NOTE_SOL, 2},

    // 4 4 3 3 2 2 1 -
    {NOTE_FA, 1},
    {NOTE_FA, 1},
    {NOTE_MI, 1},
    {NOTE_MI, 1},
    {NOTE_RE, 1},
    {NOTE_RE, 1},
    {NOTE_DO, 2},
};

#define LITTLE_STAR_LEN (sizeof(little_star) / sizeof(little_star[0]))

// TIMER1 全局4倍频  168000000hz <=> 1s
// 预分频器 168 => 1000000hz <=> 1s
// PWM 周期1ms 占空比20% 0.2ms
// BUZZER 频率20hz 周期50ms 占空比固定50% 25ms
// 1000000hz <=> 1s
// 1000hz <=> 1ms
// 50000hz <=> 50ms  period = 50000-1 duty = 25000

#define PRESCALER (168 - 1)
timer_oc_parameter_struct ocpara;
timer_parameter_struct timer_init_struct;

// PB09  TIMER1_CH1  AF1
void Buzzer_GPIO_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);
    gpio_af_set(GPIOB, GPIO_AF_1, GPIO_PIN_9);
}

static void Timer1_config(void)
{
    // 1.打开外设时钟
    rcu_periph_clock_enable(RCU_TIMER1);

    // 2.初始化定时器
    timer_struct_para_init(&timer_init_struct);
    timer_init_struct.prescaler = PRESCALER; // 定时器时钟预分频
    timer_init_struct.period = 0;            // 定时器周期

    timer_init(TIMER1, &timer_init_struct);

    // 3.配置PWM输出通道
    timer_channel_output_struct_para_init(&ocpara);
    // 通道P
    ocpara.outputstate = (uint16_t)TIMER_CCX_ENABLE; // 打开通道输出
    ocpara.ocpolarity = TIMER_OC_POLARITY_HIGH;
    ocpara.ocidlestate = TIMER_OC_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER1, TIMER_CH_1, &ocpara);
    // 4.输出模式配置
    timer_channel_output_mode_config(TIMER1, TIMER_CH_1, TIMER_OC_MODE_PWM0);
    // 5.设置占空比
    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_1, 0);

    // 6.使能定时器
    timer_enable(TIMER1);
}

// 初始化BUZZER
void bsp_buzzer_init(void)
{
    Buzzer_GPIO_config();
    Timer1_config();
}

// 播放特定频率声音 20~20000hz
void bsp_buzzer_play(uint16_t hz)
{
    // 周期
    uint16_t period = (SystemCoreClock / (PRESCALER + 1) / hz) - 1;
    // 占空比
    uint16_t duty = (period + 1) * 0.5;

    // 设置周期
    timer_init_struct.period = period;
    timer_init(TIMER1, &timer_init_struct);

    // 打开通道输出
    ocpara.outputstate = (uint16_t)TIMER_CCX_ENABLE;
    timer_channel_output_config(TIMER1, TIMER_CH_1, &ocpara);

    // 设置占空比
    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_1, duty);
}

// 停止播放
void bsp_buzzer_stop(void)
{
    // 关闭通道输出
    ocpara.outputstate = (uint16_t)TIMER_CCX_DISABLE;
    timer_channel_output_config(TIMER1, TIMER_CH_1, &ocpara);
}

/* ---------- 全局停止标志 ---------- */
static volatile uint8_t song_stop_flag = 0;

static void wait_us(uint64_t us)
{
    uint64_t start = get_us_cnt();
    while ((get_us_cnt() - start) < us)
    {
        if (song_stop_flag)
            return; // 支持中途停止
    }
}

/* ---------- 播放整首歌 ---------- */
void bsp_buzzer_play_little_star(void)
{
    song_stop_flag = 0;

    for (uint32_t i = 0; i < LITTLE_STAR_LEN; i++)
    {
        if (song_stop_flag)
            break;

        uint16_t freq = little_star[i].freq;
        uint32_t beats = little_star[i].beats;
        uint64_t dur_us = (uint64_t)BEAT_US * beats;

        if (freq == NOTE_REST)
        {
            bsp_buzzer_stop();
            wait_us(dur_us);
        }
        else
        {
            bsp_buzzer_play(freq);
            wait_us(dur_us);
            bsp_buzzer_stop();
            wait_us(NOTE_GAP_US); // 音符间隙
        }
    }

    bsp_buzzer_stop();
}

/* ---------- 中途停止 ---------- */
void bsp_buzzer_stop_song(void)
{
    song_stop_flag = 1;
    bsp_buzzer_stop();
}