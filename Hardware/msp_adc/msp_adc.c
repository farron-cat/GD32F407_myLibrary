#include "msp_adc.h"

static uint16_t encodes[ADC_LEN];

// ADC的DMA配置  外设到内存  DMA1_CH0
static void msp_adc_dma_config()
{
    // 重置DMA
    dma_deinit(DMA1, DMA_CH0);
    // 1.开启外设时钟
    rcu_periph_clock_enable(RCU_DMA1);
    // 2.配置DMA
    dma_single_data_parameter_struct init_struct;
    dma_single_data_para_struct_init(&init_struct);

    init_struct.direction = DMA_PERIPH_TO_MEMORY;           // 外设到内存
    init_struct.periph_addr = (uint32_t)(&ADC_RDATA(ADC0)); // 源地址  uint8_t*
    init_struct.memory0_addr = (uint32_t)encodes;           // 目的地址

    init_struct.periph_memory_width = DMA_PERIPH_WIDTH_16BIT; // 每次搬运大小宽度  需要根据源确定
    init_struct.number = ADC_LEN;                             // 搬运次数

    init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE; // 外设是否增长
    init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;  // 内存是否增长
    init_struct.circular_mode = DMA_CIRCULAR_MODE_ENABLE; // 搬运完成就结束  循环
    init_struct.priority = DMA_PRIORITY_LOW;              // 优先级

    dma_single_data_mode_init(DMA1, DMA_CH0, &init_struct);

    // 配置DMA通道的子外设 查表
    dma_channel_subperipheral_select(DMA1, DMA_CH0, DMA_SUBPERI0);
    // 清理DMA搬运完成标志位
    dma_flag_clear(DMA1, DMA_CH0, DMA_FLAG_FTF);
    // 3.启动DMA传输
    dma_channel_enable(DMA1, DMA_CH0);
}

// 电位器 PC4  ADC0_IN14
// ADC0_IN16  采集CPU内部温度
static void msp_adc_config()
{
    /*********************** 引脚配置 ***********************/
    // 电位器
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_4);

    // NTC热敏电阻
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_1);
    /*********************** ADC配置 ***********************/

    /* 重置 */
    adc_deinit();
    /* 使能时钟 */
    rcu_periph_clock_enable(RCU_ADC0);
    /* 设置分频系数 APB2/4 = 21M*/
    adc_clock_config(ADC_ADCCK_PCLK2_DIV4);
    /* 设置同步模式(独立模式) */
    adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
    /* 设置单次模式还是连续转换(单次转换) */
    adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, ENABLE);
    /* 设置扫描还是非扫描模式(非扫描模式) */
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
    /* 设置是否打开插入通道(不打开) */
    adc_special_function_config(ADC0, ADC_INSERTED_CHANNEL_AUTO, DISABLE);
    /* 设置分辨率 */
    adc_resolution_config(ADC0, ADC_RESOLUTION_12B);
    /* 设置数据对齐 */
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    /* 设置转换通道个数(包括常规通道组和插入通道组) */
    adc_channel_length_config(ADC0, ADC_ROUTINE_CHANNEL, ADC_LEN);
    /* 设置转换哪一个通道以及所处序列位置 1个ADC周期 1/21us*/
    adc_routine_channel_config(ADC0, 0, ADC_CHANNEL_14, ADC_SAMPLETIME_15);  // 内部温度传感器 17.1us
    adc_routine_channel_config(ADC0, 1, ADC_CHANNEL_16, ADC_SAMPLETIME_480); // 内部温度传感器 17.1us
    adc_routine_channel_config(ADC0, 2, ADC_CHANNEL_1, ADC_SAMPLETIME_15);
    // 内部通道需要单独打开
    adc_channel_16_to_18(ADC_TEMP_VREF_CHANNEL_SWITCH, ENABLE);
    adc_flag_clear(ADC0, ADC_FLAG_EOC);
    // 配置常规通道每一个通道转换完成都会产生EOC标志位
    //	adc_end_of_conversion_config(ADC0,ADC_EOC_SET_CONVERSION);

    /* 配置DMA */
    // 每个通道转换完成都会产生DMA搬运请求
    adc_dma_request_after_last_enable(ADC0);
    adc_dma_mode_enable(ADC0);

    /* 使能ADC */
    adc_enable(ADC0);
    /* 内部校准(需要delay等待) */
    delay_1ms(1);
    // 校准
    adc_calibration_enable(ADC0);

    // 将采集放入转换通道
    adc_software_trigger_enable(ADC0, ADC_ROUTINE_CHANNEL);
}

// adc初始化
void msp_adc_init()
{
    msp_adc_dma_config();
    msp_adc_config();
}
// 获取特定通道数据
uint16_t msp_adc_get(uint8_t i)
{
    return encodes[i];
}