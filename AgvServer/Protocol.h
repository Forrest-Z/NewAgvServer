#pragma once

#include <cstdint>
////����client��server����Ϣ��ʽ
#define	CLIENT_MSG_REQUEST_BODY_MAX_SIZE           (1024)  //һ����Ϣ����󳤶� 

#define CLIENT_COMMON_HEAD_HEAD		0x88
#define CLIENT_COMMON_HEAD_TAIL		0xAA

#pragma pack(1)
//��Ϣ��head
typedef struct _Client_Common_Head
{
	uint8_t head;//�̶�Ϊ0x88
	uint32_t body_length;//body�ĳ��� ���ΪTCP_MSG_MAX_SIZE
	uint8_t todo;//Ҫ��������
	uint32_t queuenumber;//��Ϣ��ţ�����ʱҪ����
	uint8_t tail;//�̶�Ϊ0xAA [head ��tail �����������Ϣ�Ƿ��� ���Ƕ������Ϣ]
}Client_Common_Head;

//!!!������������Ϣ
typedef struct _Client_Request_Msg {
	Client_Common_Head head;//�ȶ�ȡ�̶����ȵ�head(11Byte) Ȼ�����head�е�body_length,��ȡ body_length�� body����
	char body[CLIENT_MSG_REQUEST_BODY_MAX_SIZE];
	int id;//���ڱ�������ĸ�conn//client�����ô�
}Client_Request_Msg;

//������Ϣ�Ľṹ�Ķ���ͷ
typedef struct _CLIENT_RETURN_MSG_HEAD
{
	uint8_t result;//CLIENT_RETURN_MSG_RESULT
	uint32_t error_code;//CLIENT_RETURN_MSG_ERROR_CODE_
	char error_info[256];
}CLIENT_RETURN_MSG_HEAD;

//!!!�����ķ�����Ϣ
typedef struct _Client_Response_Msg {
	Client_Common_Head head;//�ȶ�ȡ�̶����ȵ�head(11Byte) Ȼ���ֶ�ȡ�̶����ȵ�return_head(261Byte) ������head�е�body_length,��ȡ body_length�� body����
	CLIENT_RETURN_MSG_HEAD return_head;//�����жϷ��ؽ��
	char body[CLIENT_MSG_REQUEST_BODY_MAX_SIZE];
}Client_Response_Msg;
//////////////////////////���涨��������ϢЭ��

//������Ϣͷ�� todo
typedef enum Client_Msg_Todo
{
	CLIENT_MSG_TODO_USER_LOGIN = 0,//��¼//username[64]+password[64]
	CLIENT_MSG_TODO_USER_LOGOUT,//�ǳ�//none
	CLIENT_MSG_TODO_USER_CHANGED_PASSWORD,//�޸�����//newpassword[64]
	CLIENT_MSG_TODO_USER_LIST,//�б�//none
	CLIENT_MSG_TODO_USER_DELTE,//ɾ���û�//userid[32]
	CLIENT_MSG_TODO_USER_ADD,//����û�//username[64] password[64] role[1]
	CLIENT_MSG_TODO_USER_MODIFY,//����û�//username[64] password[64] role[1]
	CLIENT_MSG_TODO_MAP_CREATE_START,//������ͼ��ʼ
	CLIENT_MSG_TODO_MAP_CREATE_ADD_STATION,//���վ�� station[id[4]+x[4]+y[4]+name[64]+rfid[4]+r[2]+g[2]+b[2]]{��} //�������1024���ȣ����Էֳɶ���
	CLIENT_MSG_TODO_MAP_CREATE_ADD_LINE, //���ֱ�� line[id[4] + startstation[4] + endstation[64] + length[4] + draw[1] + r[2] + g[2] + b[2]]{�� }//�������1024���ȣ����Էֳɶ���
	CLIENT_MSG_TODO_MAP_CREATE_ADD_ARC,//������� arc[id[4] + startstation[4] + endstation[64] +length[4] + draw[1] +r[2]+g[2]+b[2]+p1x[4]+p1y[4]+p2x[4]+p2y[4]]{�� }//�������1024���ȣ����Էֳɶ���
	CLIENT_MSG_TODO_MAP_CREATE_FINISH,//������ͼ���
	CLIENT_MSG_TODO_MAP_LIST_STATION,//��������վ��//none
	CLIENT_MSG_TODO_MAP_LIST_LINE,//��������ֱ��//none
	CLIENT_MSG_TODO_MAP_LIST_ARC,//������������//none
	CLIENT_MSG_TODO_HAND_REQUEST,//�������Ȩ//none
	CLIENT_MSG_TODO_HAND_RELEASE,//�ͷſ���Ȩ//none
	CLIENT_MSG_TODO_HAND_FORWARD,//ǰ��//none
	CLIENT_MSG_TODO_HAND_BACKWARD,//����//none
	CLIENT_MSG_TODO_HAND_TURN_LEFT,//��ת//none
	CLIENT_MSG_TODO_HAND_TURN_RIGHT,//��ת//none
	CLIENT_MSG_TODO_AGV_MANAGE_LIST,//�����б�//none
	CLIENT_MSG_TODO_AGV_MANAGE_ADD,//����//name[64]+ip[64]
	CLIENT_MSG_TODO_AGV_MANAGE_DELETE,//ɾ��//id[4]
	CLIENT_MSG_TODO_AGV_MANAGE_MODIFY,//�޸�//id[4]+name[64]+ip[64]
	CLIENT_MSG_TODO_TASK_CREATE_TO_X,//��X��λ������//x[4]
	CLIENT_MSG_TODO_TASK_CREATE_AGV_TO_X,//�ƶ�Agv��X��λ������//agvid[4]+x[4]
	CLIENT_MSG_TODO_TASK_CREATE_PASS_Y_TO_X,//ȥYȡ���ŵ�X������//x[4]+y[4]
	CLIENT_MSG_TODO_TASK_CREATE_AGV_PASS_Y_TO_X,//ָ��AGV��Yȡ���ŵ�X������//agvid[4]+x[4]+y[4]
	CLIENT_MSG_TODO_TASK_QUERY_STATUS,//��ѯ����״̬//taskid[4]
	CLIENT_MSG_TODO_TASK_CANCEL,//ȡ������//taskid[4]
	CLIENT_MSG_TODO_TASK_LIST_UNDO,//δ��ɵ��б�//none
	CLIENT_MSG_TODO_TASK_LIST_DOING,//����ִ�е������б�//none
	CLIENT_MSG_TODO_TASK_LIST_DONE_TODAY,//������ɵ�����//none
	CLIENT_MSG_TODO_TASK_LIST_DURING,//��ʷ��ɵ�����//ʱ���ʽ��ʽyyyy-MM-dd hh-mm-ss��from_time[24] to_time[24]
	CLIENT_MSG_TODO_LOG_LIST_DURING,//��ѯ��ʷ��־//ʱ���ʽ��ʽyyyy-MM-dd hh-mm-ss��from_time[24] to_time[24]
	CLIENT_MSG_TODO_SUB_AGV_POSITION,//���ĳ���λ����Ϣ
	CLIENT_MSG_TODO_CANCEL_SUB_AGV_POSITION,//ȡ������λ����Ϣ����
	CLIENT_MSG_TODO_SUB_AGV_STATSU,//���ĳ���״̬��Ϣ
	CLIENT_MSG_TODO_CANCEL_SUB_AGV_STATSU,//ȡ������״̬��Ϣ����
	CLIENT_MSG_TODO_SUB_LOG,//������־
	CLIENT_MSG_TODO_CANCEL_SUB_LOG,//ȡ����־����
	CLIENT_MSG_TODO_SUB_TASK,//������
	CLIENT_MSG_TODO_CANCEL_SUB_TASK,//ȡ��������

	//
	CLIENT_MSG_TODO_PUB_AGV_POSITION,//������agvλ����Ϣ������Ϣ��queuebumber = 0
	CLIENT_MSG_TODO_PUB_AGV_STATUS,//������agv״̬��Ϣ������Ϣ��queuebumber = 0
	CLIENT_MSG_TODO_PUB_LOG,//��������־��Ϣ������Ϣ��queuebumber = 0
	CLIENT_MSG_TODO_PUB_TASK,//������������Ϣ������Ϣ��queuebumber = 0

	CLIENT_MSG_TODO_NOTIFY_ALL_MAP_UPDATE,//֪ͨ��Ϣ -- ��ͼ����
}CLIENT_MSG_TODO;

//������Ϣͷ�� todo//---------------------------------------------------------------------------------------------------------------------------------

//resultλ�Ķ���
enum
{
	CLIENT_RETURN_MSG_RESULT_SUCCESS = 0,//ȫ�ֵĳɹ�
	CLIENT_RETURN_MSG_RESULT_FAIL,//ȫ�ֵĴ���
};

//error_codeλ�Ķ���
enum {
	CLIENT_RETURN_MSG_ERROR_NO_ERROR = 0,
	CLIENT_RETURN_MSG_ERROR_UNKNOW,//δ֪����
	CLIENT_RETURN_MSG_ERROR_LENGTH,//���ݳ���������
	CLIENT_RETURN_MSG_ERROR_CODE_USERNAME_NOT_EXIST,//��½�û���������
	CLIENT_RETURN_MSG_ERROR_CODE_PASSWORD_ERROR,//��½�������
	CLIENT_RETURN_MSG_ERROR_CODE_NOT_LOGIN,//�û�δ��¼
	CLIENT_RETURN_MSG_ERROR_CODE_SAVE_SQL_FAIL,//�������ݿ�ʧ��
};

////////////////////////////////////////��������������ķ��ؽṹ��



#pragma pack ()