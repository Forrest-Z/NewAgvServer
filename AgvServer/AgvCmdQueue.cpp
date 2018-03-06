#include "AgvCmdQueue.h"
#include "Common.h"

AgvCmdQueue::AgvCmdQueue():
	sendQueueNumber(0),
	recvQueueNumber(0),
	orderExcuteIndex(0),
	ordersSendIndex(0),
	lastSendOrderAmount(0),
	quit(false)
{
}


AgvCmdQueue::~AgvCmdQueue()
{
	quit = true;
}

void AgvCmdQueue::threadProcess()
{
	while (!quit)
	{
		//如果没有要发送的内容，等待
		mtx.lock();
		if (orders.size() <= 0)
		{
			mtx.unlock();
			std::chrono::milliseconds dura(50);
			std::this_thread::sleep_for(dura);
			continue;
		}
		mtx.unlock();

		//第一次发送任务
		if (ordersSendIndex == 0) {
			sendOrder();
		}
		else {
			//如果当前执行的序列>=1表示已经执行了1个或几个发过去的指令了，那么要填充新的指令给它
			if (orderExcuteIndex >= 1)
			{
				if (lastSendOrderAmount >= orderExcuteIndex)
				{
					ordersSendIndex -= (lastSendOrderAmount - orderExcuteIndex);
				}
				sendOrder();
			}
		}
	}
}

void AgvCmdQueue::sendOrder()
{
	mtx.lock();
	if (ordersSendIndex >= orders.size()) {
		orders.clear();
		ordersSendIndex = 0;
		lastSendOrderAmount = 0;
		mtx.unlock();
		//TODO:完成了队列的发送:
		if (finish != nullptr)
		{
			finish();
		}
		return;
	}


	//根据orders封装一个发送的命令
	SEND_PACKAGE send_package;
	send_package.pack_head = AGV_PACK_HEAD;

	send_package.pack_len = sizeof(SEND_PACKAGE) - 1;//一个包出去头部的部分

	//工作方式: 调度
	send_package.mode = AGV_PACK_SEND_CODE_DISPATCH_MODE;
	
	//命令编号
	send_package.queueNumber = ++sendQueueNumber & 0xFF;

	lastSendOrderAmount = 0;

	for (int i = 0;i<3;++i)
	{
		if (i + ordersSendIndex >= orders.size()) {
			//放入一个空
			send_package.orders[i].rfid = AgvOrder::RFID_CODE_IMMEDIATELY;
			send_package.orders[i].order = (AgvOrder::ORDER_STOP);
			send_package.orders[i].param = 0x00;
		}
		else {
			AgvOrder order = orders[i + ordersSendIndex];
			send_package.orders[i].rfid = order.rfid;
			send_package.orders[i].order = order.order & 0xFF;
			send_package.orders[i].param = order.param & 0xFF;
			lastSendOrderAmount += 1;
		}
	}
	mtx.unlock();
	send_package.orders[3].rfid = AgvOrder::RFID_CODE_IMMEDIATELY;
	send_package.orders[3].order = (AgvOrder::ORDER_STOP);
	send_package.orders[3].param = 0x00;

	if (toSend != nullptr)
	{
		int sendTime = 3;//失败重复发送三次
		int interval = 200;//200ms记一次发送失败
		int per = 50;//每50ms查看一次结果
		int times = interval / per;//多少次查看结果失败后 记一次发送失败
		int percount = 0;//记录查看结果 失败的次数
		bool sendResult = false;//发送结果 默认是失败的

		for (int i = 0;i<sendTime;++i)
		{
			toSend((char *)&send_package, sizeof(send_package));

			while (true)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(200));

				if (recvQueueNumber == sendQueueNumber)//发送成功
				{
					sendResult = true;
					break;
				}

				++percount;
				if (percount >= times)
				{
					sendResult = false;
					break;
				}
			}

			if (sendResult)
			{
				ordersSendIndex += lastSendOrderAmount;//发送成功，index改变
				break;
			}
		}

		if (!sendResult)++sendQueueNumber;//如果一直发送失败，可能是queueNumber相同造成的，这里增加1，下次就不会还是相同的
	}
}

void AgvCmdQueue::init(ToSendCallback _toSend, FinishCallback _finish)
{
	toSend = _toSend;
	finish = _finish;
	//启动一个线程，创建socket并连接，然后发送和读取
	std::thread(&AgvCmdQueue::threadProcess, this).detach();
}

void AgvCmdQueue::onOrderQueueChanged(int queueNumber, int orderQueueNumber)
{
	recvQueueNumber = queueNumber;
	orderExcuteIndex = orderQueueNumber;
}


void AgvCmdQueue::clear()
{
	mtx.lock();
	ordersSendIndex = 0;
	orders.clear();
	sendOrder();
	mtx.unlock();
}

void AgvCmdQueue::setQueue(const std::vector<AgvOrder> &ord)
{
	mtx.lock();
	ordersSendIndex = 0;
	orders = ord;
	mtx.unlock();
}
