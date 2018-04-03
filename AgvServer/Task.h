#pragma once

#include <string>

class Task
{
public:
	Task();
	virtual ~Task();

public:
	int id = 0;
	std::string produceTime;
	std::string doTime;
	std::string doneTime;
	int excuteAgv = 0;

	enum {
		AGV_TASK_STATUS_UNEXIST = -3,//������
		AGV_TASK_STATUS_UNEXCUTE = -2,//δִ��
		AGV_TASK_STATUS_EXCUTING = -1,//����ִ��
		AGV_TASK_STATUS_DONE = 0,//���
		AGV_TASK_STATUS_FAIL = 1,//ʧ��
		AGV_TASK_STATSU_CANCEL = 2//ȡ��
	};
	int status = AGV_TASK_STATUS_UNEXCUTE;

	enum {
		PRIORITY_VERY_LOW = 0,//��͵����ȼ�
		PRIORITY_LOW = 1,//�����ȼ�
		PRIORITY_NORMAL = 2,//��ͨ���ȼ�
		PRIORITY_HIGH = 3,//�����ȼ�
		PRIORITY_VERY_HIGH = 4,//������ȼ�
	};
	int priority = PRIORITY_NORMAL;//���ȼ�

	enum {
		INDEX_GETTING_GOOD = 0,//ȥȡ��
		INDEX_PUTTING_GOOD = 1,//ȥ�Ż�
		INDEX_GOING_STANDBY = 2,//ȥ�����ص�
	};
	int currentDoIndex = INDEX_GETTING_GOOD;

	//[���� ��ȡ�� ���ͻ� �����ɺ�ȥ��ͣ����]
	enum {
		GET_PUT_DIRECT_LEFT = 0,//��תȡ�����Ż�
		GET_PUT_DIRECT_RIGHT = 1,//��תȡ���Ż�
		GET_PUT_DIRECT_FORWARD = 2,//ǰ��ȡ���Ż�
	};

	enum {
		GET_PUT_DEFAULT_DISTANCE = 300,//ȡ�����Ż�ǰ���ľ���
	};

	//ȡ��
	int getGoodDirect = GET_PUT_DIRECT_LEFT;
	int getGoodDistance = GET_PUT_DEFAULT_DISTANCE;//ȡ��ǰ���ľ���
	int getGoodStation = 0;//ȡ����
	int getGoodHeight = 0;//ȡ���ĸ߶�
	std::string getStartTime;//��ʼȡ��ʱ��
	std::string getFinishTime;//���ȡ��ʱ��

	//�Ż�
	int putGoodDirect = GET_PUT_DIRECT_LEFT;
	int putGoodDistance = GET_PUT_DEFAULT_DISTANCE;//�Ż�ǰ���ľ���
	int putGoodStation = 0;//�ͻ���
	int putGoodHeight = 0;//�Ż��ĸ߶�
	std::string putStartTime;//��ʼ�Ż�ʱ��
	std::string putFinishTime;//��ɷ���ʱ��

	//ͣ��
	int standByStation = 0;//ͣ����
	std::string standByStartTime;//��ʼȥͣ����ʱ��
	std::string standByFinishTime;//����ͣ����ʱ��
};

