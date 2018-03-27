#pragma once

#include "AgvConnection.h"
#include "AgvCmdQueue.h"
#include "Common.h"

class Agv : public boost::enable_shared_from_this<Agv>
{
public:
	typedef boost::shared_ptr<Agv> Pointer;

	//��������ص�
	typedef std::function<void(Agv::Pointer)> TaskFinishCallback;
	TaskFinishCallback taskFinish;

	//�������ص�
	typedef std::function<void(int, Agv::Pointer)> TaskErrorCallback;
	TaskErrorCallback taskError;

	//���񱻴�ϻص�
	typedef std::function<void(Agv::Pointer)> TaskInteruptCallback;
	TaskInteruptCallback taskInteruput;

	//������̼�
	typedef std::function<void(int, Agv::Pointer)> UpdateMCallback;
	UpdateMCallback updateM;

	//������̼ƺ�վ��
	typedef std::function<void(int, int, Agv::Pointer)> UpdateMRCallback;
	UpdateMRCallback updateMR;

	Agv();
	virtual ~Agv();

	bool init(int _id,std::string _ip, int _port, TaskFinishCallback _taskFinish = nullptr, TaskErrorCallback _taskError = nullptr, TaskInteruptCallback _taskInteruput = nullptr, UpdateMCallback _updateM = nullptr, UpdateMRCallback _updateMR = nullptr);

	//��ʼ����
	void startTask(std::vector<AgvOrder> &ord);

	//ֹͣ��ȡ������
	void stopTask();

	void onQueueFinish();

	void onSend(const char *data, int len);

	void setTaskFinishCallback(TaskFinishCallback _taskFinish) {
		taskFinish = _taskFinish;
	}

	void setTaskErrorCallback(TaskErrorCallback _taskError) {
		taskError = _taskError;
	}

	void setTaskInteruptCallback(TaskInteruptCallback _taskInteruput) {
		taskInteruput = _taskInteruput;
	}

	//����·���õ�
	int task = 0;
	int lastStation = 0;
	int nowStation = 0;
	int nextStation = 0;
	//���ڼ��㵱ǰλ����Ϣ�õ�
	int lastStationOdometer = 0;//��һ����λ����̼�
	int lastRfid = 0;

	////������Ϣ:
	AGV_BASE_INFO baseinfo;
	////λ����Ϣ
	AGV_POSITION_INFO positioninfo;
	////״̬��Ϣ
	AGV_STATUS_INFO statusinfo;

	//״̬
	enum {
		AGV_STATUS_HANDING = -1,//�ֶ�ģʽ�У�������
		AGV_STATUS_IDLE = 0,//���п���
		AGV_STATUS_UNCONNECT = 1,//δ����
		AGV_STATUS_TASKING = 2,//����ִ������
		AGV_STATUS_POWER_LOW = 3,//������
		AGV_STATUS_ERROR = 4,//����
		AGV_STATUS_GO_CHARGING = 5,//���س����
		AGV_STATUS_CHARGING = 6,//���ڳ��
	};
	int status = AGV_STATUS_IDLE;
	
	//��ǰ·��
	std::vector<int> currentPath;
	//����ִ�������ID
	int currentTaskId = 0;
	
private:
	AgvConnection *connection;//��С��������
	AgvCmdQueue *cmdQueue;//���̶��е�ά��
	
	void processOnePack(char *qba,int len);
};

