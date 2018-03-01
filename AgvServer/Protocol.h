#pragma once

#include <cstdint>
////����client��server����Ϣ��ʽ
#define	TCP_ONE_MSG_MAX_SIZE           (1024)  //һ����Ϣ����󳤶� 

#define TCP_ONE_MSG_HEAD_HEAD		0x88
#define TCP_ONE_MSG_HEAD_TAIL		0xAA

//head
typedef struct _Client_Msg_Head
{
	int8_t head;//�̶�Ϊ0x88
	int8_t complete;//�������Ƿ���һ�������İ�
	int32_t body_length;//body�ĳ��� ���ΪTCP_MSG_MAX_SIZE
	int8_t type;//Ҫ��������Ĵ���
	int8_t todo;//Ҫ����������������С��
	int32_t queuenumber;//��Ϣ��ţ�����ʱҪ����
	int8_t tail;//�̶�Ϊ0xAA
}Client_Msg_Head;

//body
typedef struct _Client_Msg_Body
{
	char data[TCP_ONE_MSG_MAX_SIZE];
	int length;
}Client_Msg_Body;

//һ����������Ϣ
typedef struct _Client_Msg {
	Client_Msg_Head header;
	Client_Msg_Body body;
	int user_id;
}Client_Msg;

//////////////////////////���涨��������ϢЭ��
//�ְ����������һ�������İ���ALL
//�����Ҫ����������HEAD + TAIL
//�����Ҫ����������HEAD + BODY + TAIL
//���Ҫ���������ϰ�: HEAD + BODY + ... + BODY + TAIL
enum Client_Msg_Complete
{
	CLIENT_MSG_COMPLTE_ALL = 0,//һ�������İ�
	CLIENT_MSG_COMPLETE_HEAD,//ͷ��
	CLIENT_MSG_COMPLTE_BODY,//�м䲿��
	CLIENT_MSG_COMPLTE_TAIL//��β����
};

enum Client_Msg_Type
{
	CLIENT_MSG_TYPE_USER = 0,//�û�����
	CLIENT_MSG_TYPE_MAP,//��ͼ����
	CLIENT_MSG_TYPE_HAND,//�ֶ�����
	CLIENT_MSG_TYPE_AGV_MANAGE,//AGV����
	CLIENT_MSG_TYPE_TASK,//�������
	CLIENT_MSG_TYPE_LOG//��־
};

enum Client_Msg_Todo_User
{
	CLIENT_MSG_TODO_USER_LOGIN = 0,//��¼//username[64]+password[64]
	CLIENT_MSG_TODO_USER_LOGOUT,//�ǳ�//none
	CLIENT_MSG_TODO_USER_CHANGED_PASSWORD,//�޸�����//newpassword[64]
	CLIENT_MSG_TODO_USER_LIST,//�б�//none
	CLIENT_MSG_TODO_USER_DELTE,//ɾ���û�//userid
	CLIENT_MSG_TODO_USER_ADD,//����û�//username[64] password[64] role[1]
};

enum Client_Msg_Todo_Map
{
	CLIENT_MSG_TODO_MAP_CREATE = 0,//������ͼ
								   //station_amount[4]+line_amount[4]+arc_amount[4]
								   //+station[id[4]+x[4]+y[4]+name[64]+rfid[4]+r[2]+g[2]+b[2]]{station_amount��}  
								   //+line[id[4] + startstation[4] + endstation[64] +length[4] + draw[1] +r[2]+g[2]+b[2]]{ line_amount�� }
								   //+arc[id[4] + startstation[4] + endstation[64] +length[4] + draw[1] +r[2]+g[2]+b[2]+p1x[4]+p1y[4]+p2x[4]+p2y[4]]{ arc_amount�� }
	CLIENT_MSG_TODO_MAP_SET_IMAGE,//���õ�ͼ����//length[8] data[lengthת��long]//���length==0����ô��ձ���
	CLIENT_MSG_TODO_MAP_LIST_STATION,//��������վ��//none
	CLIENT_MSG_TODO_MAP_LIST_LINE,//��������ֱ��//none
	CLIENT_MSG_TODO_MAP_LIST_ARC,//������������//none
	CLIENT_MSG_TODO_MAP_SUB_AGV_POSITION,//����AGV��λ�ù㲥//none
	CLIENT_MSG_TODO_MAP_CANCEL_SUB_AGV_POSITION,//ȡ��AGV��λ�ù㲥//none
};

enum Client_Msg_Todo_Hand
{
	CLIENT_MSG_TODO_HAND_REQUEST = 0,//�������Ȩ//none
	CLIENT_MSG_TODO_HAND_RELEASE,//�ͷſ���Ȩ//none
	CLIENT_MSG_TODO_HAND_FORWARD,//ǰ��//none
	CLIENT_MSG_TODO_HAND_BACKWARD,//����//none
	CLIENT_MSG_TODO_HAND_TURN_LEFT,//��ת//none
	CLIENT_MSG_TODO_HAND_TURN_RIGHT,//��ת//none
	CLIENT_MSG_TODO_HAND_LIGHT,//�ƹ�//[1]
	CLIENT_MSG_TODO_HAND_SUB_AGV_STATUS,//����״̬//none
	CLIENT_MSG_TODO_HAND_CANCEL_SUB_AGV_STATUS,//ȡ������//none
};

enum Client_Msg_Todo_Agv_Manage
{
	CLIENT_MSG_TODO_AGV_MANAGE_LIST = 0,//�����б�//none
	CLIENT_MSG_TODO_AGV_MANAGE_ADD,//����//name[64]+ip[64]
	CLIENT_MSG_TODO_AGV_MANAGE_DELETE,//ɾ��//id[4]
	CLIENT_MSG_TODO_AGV_MANAGE_MODIFY//�޸�//id[4]+name[64]+ip[64]
};

enum Client_Msg_Todo_Task
{
	CLIENT_MSG_TODO_TASK_CREATE_TO_X = 0,//��X��λ������//x[4]
	CLIENT_MSG_TODO_TASK_CREATE_AGV_TO_X,//�ƶ�Agv��X��λ������//agvid[4]+x[4]
	CLIENT_MSG_TODO_TASK_CREATE_PASS_Y_TO_X,//ȥYȡ���ŵ�X������//x[4]+y[4]
	CLIENT_MSG_TODO_TASK_CREATE_AGV_PASS_Y_TO_X,//ָ��AGV��Yȡ���ŵ�X������//agvid[4]+x[4]+y[4]
	CLIENT_MSG_TODO_TASK_QUERY_STATUS,//��ѯ����״̬//taskid[4]
	CLIENT_MSG_TODO_TASK_CANCEL,//ȡ������//taskid[4]
	CLIENT_MSG_TODO_TASK_LIST_UNDO,//δ��ɵ��б�//none
	CLIENT_MSG_TODO_TASK_LIST_DOING,//����ִ�е������б�//none
	CLIENT_MSG_TODO_TASK_LIST_DONE_TODAY,//������ɵ�����//none
	CLIENT_MSG_TODO_TASK_LIST_DURING,//��ʷ��ɵ�����//ʱ���ʽ��ʽyyyy-MM-dd hh-mm-ss��from_time[24] to_time[24]
};

enum Client_Msg_Todo_Log
{
	CLIENT_MSG_TODO_LOG_LIST_DURING = 0,//��ѯ��ʷ��־//ʱ���ʽ��ʽyyyy-MM-dd hh-mm-ss��from_time[24] to_time[24]
	CLIENT_MSG_TODO_LOG_SUB,//������־�㲥//none
	CLIENT_MSG_TODO_LOG_CANCEL_SUB,//ȡ����־�㲥//none
};