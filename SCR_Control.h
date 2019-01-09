#ifndef SCR_CONTROL_H
#define SCR_CONTROL_H

#define SCR_NUM     1          /// �ɿع�����
#define SCR_HOLD_TICK   5      /// �ɿع�ʹ�ܵ�ƽ����ʱ��
#define SCR_LOWSTART_ENABLE    /// �ɿع軺����ʹ��,����ù�����ע��
#define SCR_TRANSITION_ENABLE	 /// ռ�ձȸı�ʱ���ݻ��������ٶȹ��ȣ����ȿ�������


void f_SCR_init(unsigned char id, unsigned char port, unsigned char openLevel);
void f_SCR_enable(unsigned char id, unsigned char enable);
void f_set_SCR_duty(unsigned char id, unsigned char duty);

///ѭ������
void zero_detection(void);
void f_SCR_control_tick(void);

/// ������
void f_SCR_SetLowStartEnable(unsigned char id, unsigned char startDuty,unsigned char perDuty, unsigned char perCycle);
void f_SCR_SetLowStartDisable(unsigned char id);
#endif
