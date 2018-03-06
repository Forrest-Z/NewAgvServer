#pragma once

#include <functional>
#include <vector>
#include <mutex>
#include <boost/atomic.hpp>

#define AGV_SEND_PACK_MAX_LENGTH  64

//��ͷ
#define AGV_PACK_HEAD       0x55
//��β
#define AGV_PACK_END        0xAA

//������: �ֶ�ģʽ
#define AGV_PACK_SEND_CODE_CONFIG_MODE      0x70
//������: �ֶ�ģʽ
#define AGV_PACK_SEND_CODE_HAND_MODE        0x71
//������: �ֶ�ģʽ
#define AGV_PACK_SEND_CODE_AUTO_MODE        0x72
//�����룺�Զ�ģʽ
#define AGV_PACK_SEND_CODE_DISPATCH_MODE    0x73

const char CHAR_NULL = '\0';

class AgvOrder {
public:
	AgvOrder() {
		rfid = RFID_CODE_IMMEDIATELY;
		order = ORDER_STOP;
		param = 0x00;
	}
	AgvOrder(const AgvOrder &b) {
		rfid = b.rfid;
		order = b.order;
		param = b.param;
	}

	//��ID(Ҳ����RFID)
	enum {
		RFID_CODE_IMMEDIATELY = 0x00000000,//����ִ������
		RFID_CODE_EMPTY = 0xFFFFFFFF,//�տ�
	};
	int rfid = RFID_CODE_EMPTY;

	//����
	enum {
		ORDER_STOP = 0x00,//ֹͣ param��ʱʱ�� [0x0,0xf]
		ORDER_FORWARD = 0x01,//ǰ�� param�ٶȴ���[0,10]
		ORDER_BACKWARD = 0x02,//���� param�ٶȴ���[0,10]
		ORDER_TURN_LEFT = 0x03,//��ת paramת��Ƕ�[0,180] 0ΪѰ�ŷ���
		ORDER_TURN_RIGHT = 0x04,//��ת paramת��Ƕ�[0,180] 0ΪѰ�ŷ���
		ORDER_MP3_FILE = 0x05,//MP3�ļ�ID
		ORDER_MP3_VOLUME = 0x06,//MP3������1-10��
		ORDER_FORWARD_STRIPE = 0x08,//ǰ����Ѱ�� paramΪ��Զ����(cm)
		ORDER_BACKWARD_PLATE = 0x09,//���˵�ջ�崥�� paramΪ��Զ����(cm)
		ORDER_UP_DOWN = 0xA0,//������� paramΪ���� [0,255] cm
	};
	int order = ORDER_STOP;
	//����
	int param = 0x00;
};


class AgvCmdQueue
{
public:

	typedef struct _send_package_one_order{
		int32_t rfid;
		int8_t order;
		int8_t param;
	}SEND_PACKAGE_ONE_ORDER;

	typedef struct _send_package
	{
		int8_t pack_head;							//AGV_PACK_HEAD
		int8_t pack_len;							//���������˰�ͷ����ĳ���
		////////////////////////���ݲ���
		int8_t mode;								//������ʽ ����ģʽ AGV_PACK_SEND_CODE_DISPATCH_MODE
		int8_t queueNumber;							//������ţ�1������255ѭ��
		SEND_PACKAGE_ONE_ORDER orders[4];
		////////////////////////���ݲ���
		int8_t sum;
		int8_t pack_tail;							//AGV_PACK_END
	}SEND_PACKAGE;


	typedef std::function<void(const char *data, int len)> ToSendCallback;
	typedef std::function<void()> FinishCallback;

	enum {
		PICK_PUT_HEIGHT = 30,//������߷�����Ҫ�������ĸ߶�
	};

	AgvCmdQueue();
	virtual ~AgvCmdQueue();

	void init(ToSendCallback _toSend, FinishCallback _finish);

	void clear();

	void setQueue(const std::vector<AgvOrder>& ord);

	void onOrderQueueChanged(int queueNumber, int orderQueueNumber);

private:
	void threadProcess();

	void sendOrder();
	std::vector<AgvOrder> orders;
	std::mutex mtx;

	volatile bool quit;
	boost::atomic_int32_t sendQueueNumber;
	boost::atomic_int32_t recvQueueNumber;//��ǰ��������б��
	boost::atomic_int32_t orderExcuteIndex;//��ǰִ�е���������(0,4)
	boost::atomic_int32_t ordersSendIndex;//��ǰ����ָ����±�
	boost::atomic_int32_t lastSendOrderAmount;//��һ�η��͵�ָ�������

	ToSendCallback toSend;
	FinishCallback finish;
};

