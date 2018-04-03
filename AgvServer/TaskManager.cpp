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
	//启动一个线程，每半秒检查一次未执行的任务，是否可以开始执行
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

				if (ttask->excuteAgv > 0) {//固定车辆去执行该任务
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
					//寻找最优车辆去执行任务
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
							//一个可用线路的结果//当然并不一定是最优的线路
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
					//这个任务要派给这个车了！接下来的事情是这些：
					//TODO:!!!要求一下操作可以回滚，因为车辆可能不接受该任务！！！！！！！！！！！！！！
					MapManger::Pointer g_agvMapCenter = MapManger::getInstance();
					//将终点，占领
					MapManger::getInstance()->occuStation(aimStation, bestCar->baseinfo.id);

					//将起点，释放 这里释放起点其实是不合适的，应该在小车启动的时候，释放这个位置,虽然这里就差几行
					g_agvMapCenter->freeStation(bestCar->nowStation, bestCar->baseinfo.id);

					//if(g_m_stations[bestCar->nowStation]->occuAgv == bestCar->id)g_m_stations[bestCar->nowStation]->occuAgv = (0);

					//对任务属性进行赋值
					ttask->doTime = getTimeStrNow();
					ttask->status = (Task::AGV_TASK_STATUS_EXCUTING);
					ttask->excuteAgv = (bestCar->baseinfo.id);

					//对线路属性进行赋值         //4.把线路的反方向线路定为占用
					for (auto p:path) {
						g_agvMapCenter->addOccuLine(g_agvMapCenter->getReverseLine(p), (bestCar->baseinfo.id));
					}
					//对车子属性进行赋值        //5.把这个车辆置为 非空闲,对车辆的其他信息进行更新
					bestCar->status = (Agv::AGV_STATUS_TASKING);
					bestCar->task = (ttask->id);
					bestCar->currentPath = (path);
					//将任务移动到正在执行的任务//6.把这个任务定为doing。
					itr = unassignedTasks.erase(itr);
					dTaskMtx.lock();
					doingTasks.insert(std::make_pair(ttask->id,ttask));
					dTaskMtx.unlock();
					//要看返回的结果的！！！！！！ 如果失败了，要回滚上述所有操作！太难了
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

//产生一个固定某辆车去到目的地的任务，车辆是agvId，目的地是aimStation
int TaskManager::makeAgvAimTask(int agvId, int aimStation, int priority)
{
	//TODO
	return 0;
}

//产生一个直接去到目的地的任务[车辆随意]，目的地是aimStation
int TaskManager::makeAimTask(int aimStation, int priority)
{
	//TODO
	return 0;
}

//产生一个指定车辆 取货送货的任务,pickupStation是取货点，aimStation是送货点
int TaskManager::makeAgvPickupTask(int agvId, int pickupStation, int aimStation, int standByStation, int pickupLMR, int putLMR, int priority)
{
	//TODO
	return 0;
}

//产生一个取货送货的任务,pickupStation是取货点，aimStation是送货点
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
