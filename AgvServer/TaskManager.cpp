#include "TaskManager.h"
#include "Agv.h"
#include "AgvManager.h"
#include "MapManger.h"

TaskManager::TaskManager()
{
}


TaskManager::~TaskManager()
{
}


void TaskManager::init()
{
	//����һ���̣߳�ÿ������һ��δִ�е������Ƿ���Կ�ʼִ��
	std::thread([&]() {
		while (true) {
			std::chrono::milliseconds dura(500);
			std::this_thread::sleep_for(dura);

			uTaskMtx.lock();
			for (auto itr = unassignedTasks.begin();itr != unassignedTasks.end();) {
				Task *ttask = itr->second;
				int aimStation = 0;
				if (ttask->currentDoIndex == Task::INDEX_GETTING_GOOD) {
					aimStation = ttask->getGoodStation;
				}
				else if (ttask->currentDoIndex == Task::INDEX_PUTTING_GOOD) {
					aimStation = ttask->putGoodStation;
				}
				else {
					aimStation = ttask->standByStation;
				}
				Agv::Pointer bestCar = NULL;
				int minDis = DISTANCE_INFINITY;
				std::list<int> path;
				int tempDis = DISTANCE_INFINITY;

				if (ttask->excuteAgv > 0) {//�̶�����ȥִ�и�����
					Agv::Pointer agvp = AgvManager::getInstance()->getAgv(ttask->excuteAgv);
					if (agvp == NULL || agvp->status != Agv::AGV_STATUS_IDLE) {
						++itr;
						continue;
					}
					std::list<int> result;
					if (agvp->nowStation>0) {
						result = MapManger::getInstance()->getBestPath(ttask->excuteAgv, agvp->lastStation, agvp->nowStation, aimStation, tempDis, false);
					}
					else {
						result = MapManger::getInstance()->getBestPath(ttask->excuteAgv, agvp->lastStation, agvp->nextStation, aimStation, tempDis, false);
					}
					if (result.size()>0 && tempDis != DISTANCE_INFINITY) {
						bestCar = agvp;
						minDis = tempDis;
						path = result;
					}
				}
				else {
					//Ѱ�����ų���ȥִ������
					AgvManager::getInstance()->agvForeach([&](Agv::Pointer agvpp) {
						if (agvpp == NULL)return;
						if (agvpp->status != Agv::AGV_STATUS_IDLE)return ;
						std::list<int> result;
						if (agvpp->nowStation>0) {
							result = MapManger::getInstance()->getBestPath(agvpp->baseinfo.id, agvpp->lastStation, agvpp->nowStation, aimStation, tempDis, false);
						}
						else {
							result = MapManger::getInstance()->getBestPath(agvpp->baseinfo.id, agvpp->lastStation, agvpp->nextStation, aimStation, tempDis, false);
						}
						if (result.size()>0 && tempDis != DISTANCE_INFINITY)
						{
							//һ��������·�Ľ��//��Ȼ����һ�������ŵ���·
							if (tempDis < minDis) {
								bestCar = agvpp;
								minDis = tempDis;
								path = result;
							}
						}
					});
				}

				if (bestCar != NULL && minDis != DISTANCE_INFINITY && path.size() > 0)
				{
					//�������Ҫ�ɸ�������ˣ�����������������Щ��
					//TODO:!!!Ҫ��һ�²������Իع�����Ϊ�������ܲ����ܸ����񣡣�������������������������
					MapManger::Pointer g_agvMapCenter = MapManger::getInstance();
					//���յ㣬ռ��
					MapManger::getInstance()->occuStation(aimStation, bestCar->baseinfo.id);

					//����㣬�ͷ� �����ͷ������ʵ�ǲ����ʵģ�Ӧ����С��������ʱ���ͷ����λ��,��Ȼ����Ͳ��
					g_agvMapCenter->freeStation(bestCar->nowStation, bestCar->baseinfo.id);

					//if(g_m_stations[bestCar->nowStation]->occuAgv == bestCar->id)g_m_stations[bestCar->nowStation]->occuAgv = (0);

					//���������Խ��и�ֵ
					ttask->doTime = getTimeStrNow();
					ttask->status = (Task::AGV_TASK_STATUS_EXCUTING);
					ttask->excuteAgv = (bestCar->baseinfo.id);

					//����·���Խ��и�ֵ         //4.����·�ķ�������·��Ϊռ��
					for (auto p:path) {
						g_agvMapCenter->addOccuLine(g_agvMapCenter->getReverseLine(p), (bestCar->baseinfo.id));
					}
					//�Գ������Խ��и�ֵ        //5.�����������Ϊ �ǿ���,�Գ�����������Ϣ���и���
					bestCar->status = (Agv::AGV_STATUS_TASKING);
					bestCar->task = (ttask->id);
					bestCar->currentPath = (path);
					//�������ƶ�������ִ�е�����//6.���������Ϊdoing��
					itr = unassignedTasks.erase(itr);
					dTaskMtx.lock();
					doingTasks.insert(std::make_pair(ttask->id,ttask));
					dTaskMtx.unlock();
					//Ҫ�����صĽ���ģ����������� ���ʧ���ˣ�Ҫ�ع��������в�����̫����
					//TODO:
					AgvManager::getInstance()->agvStartTask(bestCar->baseinfo.id, path);

					//TODO:
					//emit sigTaskStart(ttask->id, ttask->excuteCar);
				}

				++itr;
			}
		}
	}).detach();
}

//����һ���̶�ĳ����ȥ��Ŀ�ĵص����񣬳�����agvId��Ŀ�ĵ���aimStation
int TaskManager::makeAgvAimTask(int agvId, int aimStation, int priority)
{
	//TODO
	return 0;
}

//����һ��ֱ��ȥ��Ŀ�ĵص�����[��������]��Ŀ�ĵ���aimStation
int TaskManager::makeAimTask(int aimStation, int priority)
{
	//TODO
	return 0;
}

//����һ��ָ������ ȡ���ͻ�������,pickupStation��ȡ���㣬aimStation���ͻ���
int TaskManager::makeAgvPickupTask(int agvId, int pickupStation, int aimStation, int standByStation, int pickupLMR, int putLMR, int priority)
{
	//TODO
	return 0;
}

//����һ��ȡ���ͻ�������,pickupStation��ȡ���㣬aimStation���ͻ���
int TaskManager::makePickupTask(int pickupStation, int aimStation, int standByStation, int pickupLMR, int putLMR, int priority)
{
	
	//TODO
	return 0;
}

int TaskManager::queryTaskStatus(int taskId)
{
	//TODO
	return 0;
}

int TaskManager::cancelTask(int taskId)
{
	//TODO
	return 0;
}

TASK_INFO TaskManager::queryTaskDetail(int taskId) {
	//TODO
	TASK_INFO ti;
	memset(&ti, 0, sizeof(TASK_INFO));

	return ti;
}
