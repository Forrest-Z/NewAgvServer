#pragma once

#include <functional>
#include <vector>
#include <mutex>
#include <boost/atomic.hpp>

#define AGV_SEND_PACK_MAX_LENGTH  64

//包头
#define AGV_PACK_HEAD       0x55
//包尾
#define AGV_PACK_END        0xAA

//功能码: 手动模式
#define AGV_PACK_SEND_CODE_CONFIG_MODE      0x70
//功能码: 手动模式
#define AGV_PACK_SEND_CODE_HAND_MODE        0x71
//功能码: 手动模式
#define AGV_PACK_SEND_CODE_AUTO_MODE        0x72
//功能码：自动模式
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

	//卡ID(也就是RFID)
	enum {
		RFID_CODE_IMMEDIATELY = 0x00000000,//立即执行命令
		RFID_CODE_EMPTY = 0xFFFFFFFF,//空卡
	};
	int rfid = RFID_CODE_EMPTY;

	//命令
	enum {
		ORDER_STOP = 0x00,//停止 param延时时间 [0x0,0xf]
		ORDER_FORWARD = 0x01,//前进 param速度代码[0,10]
		ORDER_BACKWARD = 0x02,//后退 param速度代码[0,10]
		ORDER_TURN_LEFT = 0x03,//左转 param转向角度[0,180] 0为寻磁方向
		ORDER_TURN_RIGHT = 0x04,//右转 param转向角度[0,180] 0为寻磁方向
		ORDER_MP3_FILE = 0x05,//MP3文件ID
		ORDER_MP3_VOLUME = 0x06,//MP3音量（1-10）
		ORDER_FORWARD_STRIPE = 0x08,//前进到寻磁 param为最远距离(cm)
		ORDER_BACKWARD_PLATE = 0x09,//后退到栈板触发 param为最远距离(cm)
		ORDER_UP_DOWN = 0xA0,//升降插齿 param为距离 [0,255] cm
	};
	int order = ORDER_STOP;
	//参数
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
		int8_t pack_len;							//包长，除了包头以外的长度
		////////////////////////内容部分
		int8_t mode;								//工作方式 调度模式 AGV_PACK_SEND_CODE_DISPATCH_MODE
		int8_t queueNumber;							//发送序号，1递增至255循环
		SEND_PACKAGE_ONE_ORDER orders[4];
		////////////////////////内容部分
		int8_t sum;
		int8_t pack_tail;							//AGV_PACK_END
	}SEND_PACKAGE;


	typedef std::function<void(const char *data, int len)> ToSendCallback;
	typedef std::function<void()> FinishCallback;

	enum {
		PICK_PUT_HEIGHT = 30,//叉起或者放下需要的升降的高度
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
	boost::atomic_int32_t recvQueueNumber;//当前命令的序列编号
	boost::atomic_int32_t orderExcuteIndex;//当前执行到命令条数(0,4)
	boost::atomic_int32_t ordersSendIndex;//当前发送指令的下标
	boost::atomic_int32_t lastSendOrderAmount;//上一次发送的指令的数量

	ToSendCallback toSend;
	FinishCallback finish;
};

