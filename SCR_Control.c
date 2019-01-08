#include "HeaderInclude.h"



#define LOW_STARTINT    0
#define LOW_STARTED     1
typedef struct
{
    unsigned char SCR_duty; /// �ɿع�ռ�ձ�
    unsigned char SCR_tick;/// �ɿع��ʱ
    unsigned char SCR_port;/// �ɿع�˿�
    unsigned char SCR_isOpen: 1;/// �Ƿ����ɿع�
    unsigned char SCR_OpenLevel: 1; /// �ɿع迪����ƽ
#ifdef SCR_LOWSTART_ENABLE
    /// ����������

    unsigned char lowStartPerDuty: 7; /// ÿ�λ���������ռ�ձ�
    unsigned char isLowStart: 1;/// �Ƿ�����
    unsigned char lowStartState: 1;/// ��������־
    unsigned char lowStartEveryCycle;/// ���ж��ٸ�����������ռ�ձ�
    unsigned char cycleCnt;/// ��������
    unsigned char lowStartDuty;/// ��ǰռ�ձ�
#endif
} SCR_object;

static SCR_object SCRobj[SCR_NUM];

/**
 * @brief �ɿع��ʼ��
 *
 * @param id �ɿع���
 * @param port �ɿع�����˿�
 */
void f_SCR_init(unsigned char id, unsigned char port, unsigned char openLevel)
{
    SCRobj[id].SCR_duty = 50; /// �ɿع�ռ�ձ�
    SCRobj[id].SCR_tick = 0;/// �ɿع��ʱ
    SCRobj[id].SCR_isOpen = 0;/// �Ƿ����ɿع�
    SCRobj[id].SCR_port = port;/// �ɿع�˿�
    SCRobj[id].SCR_OpenLevel = openLevel;/// �ɿع�˿�
#ifdef SCR_LOWSTART_ENABLE
    /// ����������
    SCRobj[id].lowStartPerDuty = 2; /// ÿ�λ���������ռ�ձ�
    SCRobj[id].lowStartEveryCycle = 2;/// ���ж��ٸ�����������ռ�ձ�
    SCRobj[id].isLowStart = 0;/// �Ƿ�����
    SCRobj[id].lowStartState = 0;/// ��������־
    SCRobj[id].cycleCnt = 0;/// ��������
    SCRobj[id].lowStartDuty = 0;/// ��ǰռ�ձ�
    SCRobj[id].isLowStart = 0;
#endif
}

/**
 * @brief �򿪿ɿع軺����
 *
 * @param id �ɿع���
 * @param perDuty ÿ�����Ӷ���ռ�ձ�
 * @param perCycle ÿ������������һ��ռ�ձ�
 */
void f_SCR_SetLowStartEnable(unsigned char id, unsigned char perDuty, unsigned char perCycle)
{
#ifdef SCR_LOWSTART_ENABLE
    SCRobj[id].lowStartPerDuty = perDuty; /// ÿ�λ���������ռ�ձ�
    SCRobj[id].lowStartEveryCycle = perCycle;/// ���ж��ٸ�����������ռ�ձ�
    SCRobj[id].isLowStart = 1;/// �Ƿ�����
    SCRobj[id].lowStartState = 0;/// ��������־
    SCRobj[id].cycleCnt = 0;/// ��������
    SCRobj[id].lowStartDuty = 0;/// ��ǰռ�ձ�
#endif
}

/**
 * @brief �رտɿع�����
 *
 * @param id �ɿع���
 */
void f_SCR_SetLowStartDisable(unsigned char id)
{
#ifdef SCR_LOWSTART_ENABLE
    SCRobj[id].isLowStart = 0;/// �Ƿ�����
#endif
}

/**
 * @brief �����⣬��������ʱ���øú���
 *
 */
void zero_detection(void)
{
    unsigned char i;

    for (i = 0; i < SCR_NUM; i++)
    {
        SCRobj[i].SCR_tick = 0;
        pin_write(SCRobj[i].SCR_port, !SCRobj[i].SCR_OpenLevel);
#ifdef SCR_LOWSTART_ENABLE

        if (SCRobj[i].isLowStart && SCRobj[i].lowStartState == LOW_STARTINT)
        {
            ++SCRobj[i].cycleCnt;

            if (SCRobj[i].lowStartEveryCycle <= SCRobj[i].cycleCnt)
            {
                SCRobj[i].cycleCnt = 0;
                SCRobj[i].lowStartDuty += SCRobj[i].lowStartPerDuty;

                if (SCRobj[i].lowStartDuty >= SCRobj[i].SCR_duty)
                {
                    SCRobj[i].lowStartState = LOW_STARTED;
                }
            }
        }

#endif
    }
}
/**
 * @brief ʹ�ܿɿع����
 *
 * @param enable �Ƿ����
 */
void f_SCR_enable(unsigned char id, unsigned char enable)
{
    SCRobj[id].SCR_isOpen = enable;
#ifdef SCR_LOWSTART_ENABLE

    if (enable != 0)
    {
        if (enable != SCRobj[id].SCR_isOpen)
        {

            SCRobj[id].lowStartState = 0;/// ��������־
            SCRobj[id].cycleCnt = 0;/// ��������
            SCRobj[id].lowStartDuty = 0;/// ��ǰռ�ձ�

        }
#endif
    }
}
/**
 * @brief Set the SCR duty object
 *
 * @param id �ɿع���
 * @param duty ռ�ձȣ��������Ϊ���� (1-cos(duty*pi/100))/2
 */
void f_set_SCR_duty(unsigned char id, unsigned char duty)
{
    SCRobj[id].SCR_duty = duty;
}
/**
 * @brief ʱ��Ƭ���������ú���������ʱ��Ƭ�ڣ�100us
 *
 */
void f_SCR_control_tick(void)
{
    unsigned char i = 0;

    for (i = 0 ; i < SCR_NUM; ++i)
    {
        if (SCRobj[i].SCR_isOpen)
        {
            unsigned char temp;
#ifdef SCR_LOWSTART_ENABLE

            if (SCRobj[i].isLowStart)
            {
                temp = 100 - SCRobj[i].lowStartState == LOW_STARTED ? SCR[i].SCR_duty : SCRobj[i].lowStartDuty;
            }
            else
#endif
            {
                temp = 100 - SCR[i].SCR_duty;
            }

            ++SCRobj[i].SCR_tick;

            if (SCRobj[i].SCR_tick >= temp + SCR_HOLD_TICK)
            {
                pin_write(SCRobj[i].SCR_port, !SCRobj[i].SCR_OpenLevel);
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
