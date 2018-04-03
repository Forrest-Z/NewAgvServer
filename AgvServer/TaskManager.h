#pragma once

#include <boost/noncopyable.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <mutex>
#include <map>
#include "Task.h"
#include "Common.h"

class TaskManager :private boost::noncopyable, public boost::enable_shared_from_this<TaskManager>
{
public:

	typedef boost::shared_ptr<TaskManager> Pointer;

	typedef std::function<void(int taskId, int agvId)> TASK_START_CALLBACK;
	typedef std::function<void(int taskId)> TASK_FINISH_CALLBACK;
	typedef std::function<void(int taskId,int errno,std::string errinfo)> TASK_ERROR_CALLBACK;

	virtual ~TaskManager();

	static Pointer getInstance()
	{
		static Pointer m_inst = Pointer(new TaskManager());
		return m_inst;
	}

	void init();

	//产生一个固定某辆车去到目的地的任务，车辆是agvId，目的地是aimStation
	int makeAgvAimTask(int agvId, int aimStation, int priority = Task::PRIORITY_NORMAL);

	//产生一个直接去到目的地的任务[车辆随意]，目的地是aimStation
	int makeAimTask(int aimStation, int priority = Task::PRIORITY_NORMAL);

	//产生一个指定车辆 取货送货的任务,pickupStation是取货点，aimStation是送货点
	int makeAgvPickupTask(int agvId, int pickupStation, int aimStation, int standByStation, int pickupLMR, int putLMR, int priority = Task::PRIORITY_NORMAL);

	//产生一个取货送货的任务,pickupStation是取货点，aimStation是送货点
	int makePickupTask(int pickupStation, int aimStation, int standByStation,int pickupLMR,int putLMR, int priority = Task::PRIORITY_NORMAL);

	int queryTaskStatus(int taskId);//返回task的状态。

	int cancelTask(int taskId);//取消一个任务

	TASK_INFO queryTaskDetail(int taskId);//返回task的具体信息

private:
	TaskManager();

	std::map<int, Task *> unassignedTasks;
	std::mutex uTaskMtx;

	std::map<int, Task *> doingTasks;                //正在执行的任务
	std::mutex dTaskMtx;

	TASK_START_CALLBACK taskStart;
	TASK_FINISH_CALLBACK taskFinish;
	TASK_ERROR_CALLBACK taskError;
};

