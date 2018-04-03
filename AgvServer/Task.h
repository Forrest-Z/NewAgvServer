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
		AGV_TASK_STATUS_UNEXIST = -3,//不存在
		AGV_TASK_STATUS_UNEXCUTE = -2,//未执行
		AGV_TASK_STATUS_EXCUTING = -1,//正在执行
		AGV_TASK_STATUS_DONE = 0,//完成
		AGV_TASK_STATUS_FAIL = 1,//失败
		AGV_TASK_STATSU_CANCEL = 2//取消
	};
	int status = AGV_TASK_STATUS_UNEXCUTE;

	enum {
		PRIORITY_VERY_LOW = 0,//最低的优先级
		PRIORITY_LOW = 1,//低优先级
		PRIORITY_NORMAL = 2,//普通优先级
		PRIORITY_HIGH = 3,//高优先级
		PRIORITY_VERY_HIGH = 4,//最高优先级
	};
	int priority = PRIORITY_NORMAL;//优先级

	enum {
		INDEX_GETTING_GOOD = 0,//去取货
		INDEX_PUTTING_GOOD = 1,//去放货
		INDEX_GOING_STANDBY = 2,//去待命地点
	};
	int currentDoIndex = INDEX_GETTING_GOOD;

	//[车辆 先取货 再送货 最后完成后去到停留点]
	enum {
		GET_PUT_DIRECT_LEFT = 0,//左转取货、放货
		GET_PUT_DIRECT_RIGHT = 1,//右转取货放货
		GET_PUT_DIRECT_FORWARD = 2,//前进取货放货
	};

	enum {
		GET_PUT_DEFAULT_DISTANCE = 300,//取货、放货前进的距离
	};

	//取货
	int getGoodDirect = GET_PUT_DIRECT_LEFT;
	int getGoodDistance = GET_PUT_DEFAULT_DISTANCE;//取货前进的距离
	int getGoodStation = 0;//取货点
	int getGoodHeight = 0;//取货的高度
	std::string getStartTime;//开始取货时间
	std::string getFinishTime;//完成取货时间

	//放货
	int putGoodDirect = GET_PUT_DIRECT_LEFT;
	int putGoodDistance = GET_PUT_DEFAULT_DISTANCE;//放货前进的距离
	int putGoodStation = 0;//送货点
	int putGoodHeight = 0;//放货的高度
	std::string putStartTime;//开始放货时间
	std::string putFinishTime;//完成返货时间

	//停留
	int standByStation = 0;//停留点
	std::string standByStartTime;//开始去停留点时间
	std::string standByFinishTime;//到达停留点时间
};

