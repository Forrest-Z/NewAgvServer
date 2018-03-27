#include "Agv.h"

Agv::Agv():
	cmdQueue(NULL),
	connection(NULL)
{
}



Agv::~Agv()
{
	if (cmdQueue)delete cmdQueue;
	if (connection)delete connection;
}

//开始任务
void Agv::startTask(std::vector<AgvOrder>& ord)
{
	if (cmdQueue)
	{
		cmdQueue->setQueue(ord);
	}
}

//停止、取消任务
void Agv::stopTask()
{
	if (cmdQueue)
	{
		cmdQueue->clear();
	}
}


void Agv::onQueueFinish()
{
	auto self(shared_from_this());
	if (taskFinish != nullptr) {
		taskFinish(self);
	}
}

void Agv::onSend(const char *data, int len)
{
	if (connection)
	{
		connection->doSend((char *)data, len);
	}
}

bool Agv::init(int _id,std::string _ip, int _port, TaskFinishCallback _taskFinish, TaskErrorCallback _taskError, TaskInteruptCallback _taskInteruput, UpdateMCallback _updateM, UpdateMRCallback _updateMR)
{
	baseinfo.id = _id;
	sprintf_s(baseinfo.ip, _ip.c_str(), _ip.length(), 64);
	baseinfo.port = _port;
	taskFinish = _taskFinish;
	taskError = _taskError;
	taskInteruput = _taskInteruput;
	updateM = _updateM;
	updateMR = _updateMR;
	if (cmdQueue) {
		delete cmdQueue;
		cmdQueue = NULL;
	}
	if (connection) {
		delete connection;
		connection = NULL;
	}

	//创建连接
	AgvConnectionOnReadPackage onread = std::bind(&Agv::processOnePack, this, std::placeholders::_1, std::placeholders::_2);

	connection = new AgvConnection(_id,_ip,_port, onread);

	//创建队列处理
	AgvCmdQueue::ToSendCallback s = std::bind(&Agv::onSend, this, std::placeholders::_1, std::placeholders::_2);
	AgvCmdQueue::FinishCallback f = std::bind(&Agv::onQueueFinish, this);
	cmdQueue = new AgvCmdQueue;
	cmdQueue->init(s, f);

	return true;
}

void Agv::processOnePack(char *qba, int len)
{
	auto self(shared_from_this());
	int kk = (int)(qba[1] & 0xFF);

	if (kk != len - 1) return;//除去包头
	assert(kk >= 30);

	char *str = qba;
	str += 2;//跳过包头和包长

	statusinfo.mileage = getInt32FromByte(str);
	str += 4;

	statusinfo.currentRfid = getInt32FromByte(str);
	//qDebug()<<"current rfid="<<currentRfid;
	str += 4;

	statusinfo.nextRfid = getInt32FromByte(str);
	str += 4;

	statusinfo.current = getInt16FromByte(str);
	str += 2;

	statusinfo.voltage = getInt16FromByte(str);
	str += 2;

	statusinfo.positionMagneticStripe = getInt16FromByte(str);
	str += 2;

	statusinfo.pcbTemperature = getInt8FromByte(str);
	str += 1;

	statusinfo.motorTemperature = getInt8FromByte(str);
	str += 1;

	statusinfo.cpu = getInt8FromByte(str);
	str += 1;

	statusinfo.speed = getInt8FromByte(str);
	str += 1;

	statusinfo.angle = getInt8FromByte(str);
	str += 1;

	statusinfo.height = getInt8FromByte(str);
	str += 1;

	statusinfo.error_no = getInt8FromByte(str);
	str += 1;

	statusinfo.mode = getInt8FromByte(str);
	str += 1;

	statusinfo.recvQueueNumber = getInt8FromByte(str);
	str += 1;

	statusinfo.orderCount = getInt8FromByte(str);
	str += 1;


	statusinfo.CRC = getInt8FromByte(str);
	str += 1;

	//校验CRC
	str = qba;
	unsigned char c = checkSum((unsigned char *)(str + 2), kk - 4);
	if (c != statusinfo.CRC)
	{
		//TODO:
		//校验不合格
	}

	//更新小车状态
	if (statusinfo.mode == AGV_MODE_HAND) {
		if (currentTaskId > 0)
		{

			//1.通知cmdqueue，取消任务
			cmdQueue->clear();

			//2.通知上面的，取消任务了
			if (taskInteruput != nullptr) 
			{
				taskInteruput(self);
			}

			//3.任务置空? 交给上面完成吧

			//emit taskCancel(currentTaskId);
		}
		status = AGV_STATUS_HANDING;
	}

	//更新错误状态
	if (statusinfo.error_no != 0 && currentTaskId > 0)
	{
		//1.通知cmdqueue，取消任务
		cmdQueue->clear();
		//任务过程中发生错误
		if (taskError != nullptr) {
			taskError(statusinfo.error_no, self);
		}
	}

	//更新rfid       //更新里程计
	if (statusinfo.currentRfid != lastRfid)
	{
		lastRfid = statusinfo.currentRfid;
		lastStationOdometer = statusinfo.mileage;

		if (updateMR != nullptr) {
			updateMR(statusinfo.currentRfid, statusinfo.mileage, self);
		}
	}
	else {
		if (updateM != nullptr) {
			updateM(statusinfo.mileage, self);
		}
	}

	if (cmdQueue) {
		cmdQueue->onOrderQueueChanged(statusinfo.recvQueueNumber, statusinfo.orderCount);
	}


}
