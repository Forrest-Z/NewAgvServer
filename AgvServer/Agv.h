#pragma once

#include "AgvConnection.h"
#include "AgvCmdQueue.h"
#include "Common.h"

class Agv : public boost::enable_shared_from_this<AgvConnection>
{
public:
	typedef boost::shared_ptr<Agv> Pointer;

	//任务结束回调
	typedef std::function<void(Agv *)> TaskFinishCallback;
	TaskFinishCallback taskFinish;

	//任务错误回调
	typedef std::function<void(int, Agv *)> TaskErrorCallback;
	TaskErrorCallback taskError;

	//任务被打断回调
	typedef std::function<void(Agv *)> TaskInteruptCallback;
	TaskInteruptCallback taskInteruput;

	//更新里程计
	typedef std::function<void(int, Agv *)> UpdateMCallback;
	UpdateMCallback updateM;

	//更新里程计和站点
	typedef std::function<void(int, int, Agv *)> UpdateMRCallback;
	UpdateMRCallback updateMR;

	Agv();
	virtual ~Agv();

	bool init(int _id,std::string _ip, int _port, TaskFinishCallback _taskFinish = nullptr, TaskErrorCallback _taskError = nullptr, TaskInteruptCallback _taskInteruput = nullptr, UpdateMCallback _updateM = nullptr, UpdateMRCallback _updateMR = nullptr);

	//开始任务
	void startTask(std::vector<AgvOrder> &ord);

	//停止、取消任务
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

	//计算路径用的
	int task = 0;
	int lastStation = 0;
	int nowStation = 0;
	int nextStation = 0;
	//用于计算当前位置信息用的
	int lastStationOdometer = 0;//上一个点位的里程计
	int lastRfid = 0;

	////基础信息:
	AGV_BASE_INFO baseinfo;
	////位置信息
	AGV_POSITION_INFO positioninfo;
	////状态信息
	AGV_STATUS_INFO statusinfo;

	//状态
	enum {
		AGV_STATUS_HANDING = -1,//手动模式中，不可用
		AGV_STATUS_IDLE = 0,//空闲可用
		AGV_STATUS_UNCONNECT = 1,//未连接
		AGV_STATUS_TASKING = 2,//正在执行任务
		AGV_STATUS_POWER_LOW = 3,//电量低
		AGV_STATUS_ERROR = 4,//故障
		AGV_STATUS_GO_CHARGING = 5,//返回充电中
		AGV_STATUS_CHARGING = 6,//正在充电
	};
	int status = AGV_STATUS_IDLE;
	
	//当前路径
	std::list<int> currentPath;
	//正在执行任务的ID
	int currentTaskId = 0;
	
private:
	AgvConnection *connection;//和小车的连接
	AgvCmdQueue *cmdQueue;//长短队列的维护
	
	void processOnePack(char *qba,int len);
};

