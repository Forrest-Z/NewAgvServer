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
		//���û��Ҫ���͵����ݣ��ȴ�
		mtx.lock();
		if (orders.size() <= 0)
		{
			mtx.unlock();
			std::chrono::milliseconds dura(50);
			std::this_thread::sleep_for(dura);
			continue;
		}
		mtx.unlock();

		//��һ�η�������
		if (ordersSendIndex == 0) {
			sendOrder();
		}
		else {
			//�����ǰִ�е�����>=1��ʾ�Ѿ�ִ����1���򼸸�����ȥ��ָ���ˣ���ôҪ����µ�ָ�����
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
		//TODO:����˶��еķ���:
		if (finish != nullptr)
		{
			finish();
		}
		return;
	}


	//����orders��װһ�����͵�����
	SEND_PACKAGE send_package;
	send_package.pack_head = AGV_PACK_HEAD;

	send_package.pack_len = sizeof(SEND_PACKAGE) - 1;//һ������ȥͷ���Ĳ���

	//������ʽ: ����
	send_package.mode = AGV_PACK_SEND_CODE_DISPATCH_MODE;
	
	//������
	send_package.queueNumber = ++sendQueueNumber & 0xFF;

	lastSendOrderAmount = 0;

	for (int i = 0;i<3;++i)
	{
		if (i + ordersSendIndex >= orders.size()) {
			//����һ����
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
		int sendTime = 3;//ʧ���ظ���������
		int interval = 200;//200ms��һ�η���ʧ��
		int per = 50;//ÿ50ms�鿴һ�ν��
		int times = interval / per;//���ٴβ鿴���ʧ�ܺ� ��һ�η���ʧ��
		int percount = 0;//��¼�鿴��� ʧ�ܵĴ���
		bool sendResult = false;//���ͽ�� Ĭ����ʧ�ܵ�

		for (int i = 0;i<sendTime;++i)
		{
			toSend((char *)&send_package, sizeof(send_package));

			while (true)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(200));

				if (recvQueueNumber == sendQueueNumber)//���ͳɹ�
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
				ordersSendIndex += lastSendOrderAmount;//���ͳɹ���index�ı�
				break;
			}
		}

		if (!sendResult)++sendQueueNumber;//���һֱ����ʧ�ܣ�������queueNumber��ͬ��ɵģ���������1���´ξͲ��ỹ����ͬ��
	}
}

void AgvCmdQueue::init(ToSendCallback _toSend, FinishCallback _finish)
{
	toSend = _toSend;
	finish = _finish;
	//����һ���̣߳�����socket�����ӣ�Ȼ���ͺͶ�ȡ
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
