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

	//����һ���̶�ĳ����ȥ��Ŀ�ĵص����񣬳�����agvId��Ŀ�ĵ���aimStation
	int makeAgvAimTask(int agvId, int aimStation, int priority = Task::PRIORITY_NORMAL);

	//����һ��ֱ��ȥ��Ŀ�ĵص�����[��������]��Ŀ�ĵ���aimStation
	int makeAimTask(int aimStation, int priority = Task::PRIORITY_NORMAL);

	//����һ��ָ������ ȡ���ͻ�������,pickupStation��ȡ���㣬aimStation���ͻ���
	int makeAgvPickupTask(int agvId, int pickupStation, int aimStation, int standByStation, int pickupLMR, int putLMR, int priority = Task::PRIORITY_NORMAL);

	//����һ��ȡ���ͻ�������,pickupStation��ȡ���㣬aimStation���ͻ���
	int makePickupTask(int pickupStation, int aimStation, int standByStation,int pickupLMR,int putLMR, int priority = Task::PRIORITY_NORMAL);

	int queryTaskStatus(int taskId);//����task��״̬��

	int cancelTask(int taskId);//ȡ��һ������

	TASK_INFO queryTaskDetail(int taskId);//����task�ľ�����Ϣ

private:
	TaskManager();

	std::map<int, Task *> unassignedTasks;
	std::mutex uTaskMtx;

	std::map<int, Task *> doingTasks;                //����ִ�е�����
	std::mutex dTaskMtx;

	TASK_START_CALLBACK taskStart;
	TASK_FINISH_CALLBACK taskFinish;
	TASK_ERROR_CALLBACK taskError;
};

