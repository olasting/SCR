#include "HeaderInclude.h"



#define LOW_STARTING    0
#define LOW_STARTED     1
typedef struct
{
    unsigned char SCR_duty; /// 可控硅占空比
    unsigned char SCR_tick;/// 可控硅计时
    unsigned char SCR_port;/// 可控硅端口
    unsigned char SCR_isOpen: 1;/// 是否开启可控硅
    unsigned char SCR_OpenLevel: 1; /// 可控硅开启电平
    unsigned char SCR_isDeal: 1; /// 当前过零周期是否处理
#ifdef SCR_LOWSTART_ENABLE
    /// 缓启动部分

    unsigned char lowStartPerDuty: 7; /// 每次缓启动增加占空比
    unsigned char lowStartStartDuty: 7; /// 缓启动初始占空比
    unsigned char isLowStart: 1;/// 是否缓启动
    unsigned char lowStartState: 1;/// 缓启动标志
    unsigned char lowStartEveryCycle;/// 运行多少个半周期增加占空比
    unsigned char cycleCnt;/// 运行周期
    unsigned char lowStartDuty;/// 当前占空比
#endif
} SCR_object;

static SCR_object SCRobj[SCR_NUM];

/**
 * @brief 可控硅初始化
 *
 * @param id 可控硅编号
 * @param port 可控硅输出端口
 */
void f_SCR_init(unsigned char id, unsigned char port, unsigned char openLevel)
{

    SCRobj[id].SCR_duty = 50; /// 可控硅占空比
    SCRobj[id].SCR_tick = 0;/// 可控硅计时
    SCRobj[id].SCR_isOpen = 0;/// 是否开启可控硅
    SCRobj[id].SCR_port = port;/// 可控硅端口
    SCRobj[id].SCR_OpenLevel = openLevel;/// 可控硅端口
    SCRobj[id].SCR_isDeal = 1;/// 过零周期已处理
#ifdef SCR_LOWSTART_ENABLE
    /// 缓启动部分
    SCRobj[id].lowStartPerDuty = 2; /// 每次缓启动增加占空比
    SCRobj[id].lowStartEveryCycle = 2;/// 运行多少个半周期增加占空比
    SCRobj[id].isLowStart = 0;/// 是否缓启动
    SCRobj[id].lowStartState = 0;/// 缓启动标志
    SCRobj[id].cycleCnt = 0;/// 运行周期
    SCRobj[id].lowStartDuty = 0;/// 当前占空比
    SCRobj[id].isLowStart = 0;
    SCRobj[id].lowStartStartDuty = 0;
#endif
}

/**
 * @brief 打开可控硅缓启动
 *
 * @param id 可控硅编号
 * @param startDuty 起始占空比
 * @param perDuty 每次增加多少占空比
 * @param perCycle 每多少周期增加一次占空比
 */
void f_SCR_SetLowStartEnable(unsigned char id, unsigned char startDuty, unsigned char perDuty, unsigned char perCycle)
{
#ifdef SCR_LOWSTART_ENABLE
    SCRobj[id].lowStartPerDuty = perDuty; /// 每次缓启动增加占空比
    SCRobj[id].lowStartEveryCycle = perCycle;/// 运行多少个半周期增加占空比
    SCRobj[id].isLowStart = 1;/// 是否缓启动
    SCRobj[id].lowStartState = 0;/// 缓启动标志
    SCRobj[id].cycleCnt = 0;/// 运行周期
    SCRobj[id].lowStartDuty = startDuty;/// 当前占空比
    SCRobj[id].lowStartStartDuty = startDuty;
#endif
}

/**
 * @brief 关闭可控硅软起动
 *
 * @param id 可控硅编号
 */
void f_SCR_SetLowStartDisable(unsigned char id)
{
#ifdef SCR_LOWSTART_ENABLE
    SCRobj[id].isLowStart = 0;/// 是否缓启动
#endif
}

/**
 * @brief 过零检测，发生过零时调用该函数
 *
 */
void zero_detection(void)
{
    unsigned char i;

    for (i = 0; i < SCR_NUM; ++i)
    {
        SCRobj[i].SCR_tick = 0;
        pin_write(SCRobj[i].SCR_port, !SCRobj[i].SCR_OpenLevel);
        SCRobj[i].SCR_isDeal = 0;
#ifdef SCR_LOWSTART_ENABLE

        if (SCRobj[i].isLowStart && SCRobj[i].lowStartState == LOW_STARTING)
        {
            ++SCRobj[i].cycleCnt;

            if (SCRobj[i].lowStartEveryCycle <= SCRobj[i].cycleCnt)
            {
                SCRobj[i].cycleCnt = 0;

                if (SCRobj[i].lowStartDuty < SCRobj[i].SCR_duty)
                {
                    SCRobj[i].lowStartDuty += SCRobj[i].lowStartPerDuty;

                    if (SCRobj[i].lowStartDuty >= SCRobj[i].SCR_duty)
                    {
                        SCRobj[i].lowStartState = LOW_STARTED;
                        SCRobj[i].lowStartDuty = SCRobj[i].SCR_duty;
                    }
                }
                else
                {
                    SCRobj[i].lowStartDuty -= SCRobj[i].lowStartPerDuty;

                    if (SCRobj[i].lowStartDuty <= SCRobj[i].SCR_duty)
                    {
                        SCRobj[i].lowStartState = LOW_STARTED;
                        SCRobj[i].lowStartDuty = SCRobj[i].SCR_duty;
                    }
                }


            }
        }

#endif
    }
}
/**
 * @brief 使能可控硅输出
 *
 * @param enable 是否输出
 */
void f_SCR_enable(unsigned char id, unsigned char enable)
{

#ifdef SCR_LOWSTART_ENABLE

    if (enable != 0)
    {
        if (enable != SCRobj[id].SCR_isOpen)
        {

            SCRobj[id].lowStartState = LOW_STARTING;/// 缓启动标志
            SCRobj[id].cycleCnt = 0;/// 运行周期
            SCRobj[id].lowStartDuty = SCRobj[id].lowStartStartDuty;/// 当前占空比

        }

#endif
    }

    SCRobj[id].SCR_isOpen = enable;
}
/**
 * @brief Set the SCR duty object
 *
 * @param id 可控硅编号
 * @param duty 占空比，具体输出为比例 (1-cos(duty*pi/100))/2
 */
void f_set_SCR_duty(unsigned char id, unsigned char duty)
{

    if (duty != SCRobj[id].SCR_duty)
    {
#ifdef SCR_TRANSITION_ENABLE
        //SCRobj[id].lowStartDuty = SCRobj[id].SCR_duty;/// 当前占空比
        SCRobj[id].lowStartState = LOW_STARTING;
#endif
        SCRobj[id].SCR_duty = duty;
    }
}
/**
 * @brief 时间片函数，将该函数放置于时间片内，100us
 *
 */
void f_SCR_control_tick(void)
{
    unsigned char i = 0;

    for (i = 0 ; i < SCR_NUM; ++i)
    {
        if (SCRobj[i].SCR_isOpen && SCRobj[i].SCR_isDeal == 0)
        {
            unsigned char temp;
#ifdef SCR_LOWSTART_ENABLE

            if (SCRobj[i].isLowStart)
            {
                temp = SCRobj[i].lowStartState == LOW_STARTED ? SCRobj[i].SCR_duty : SCRobj[i].lowStartDuty;
                temp = 100 - temp;
            }
            else
#endif
            {
                temp = 100 - SCRobj[i].SCR_duty;
            }

            ++SCRobj[i].SCR_tick;

            if (SCRobj[i].SCR_tick >= temp + SCR_HOLD_TICK)
            {
                pin_write(SCRobj[i].SCR_port, !SCRobj[i].SCR_OpenLevel);
                SCRobj[i].SCR_isDeal = 1;
            }
            else if (SCRobj[i].SCR_tick >= temp)
            {
                pin_write(SCRobj[i].SCR_port, SCRobj[i].SCR_OpenLevel);
            }
            else
            {
                pin_write(SCRobj[i].SCR_port, !SCRobj[i].SCR_OpenLevel);
            }
        }
        else
        {
            pin_write(SCRobj[i].SCR_port, !SCRobj[i].SCR_OpenLevel);
        }
    }
}
