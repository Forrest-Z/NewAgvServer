#pragma once
#include <cstdint>
#include <mutex>

#define M_PI 3.14159265358979323846

#define  DISTANCE_INFINITY INT_MAX

//�û���Ϣ�ṹ��[��¼�ɹ�ʱ������һ�����û���userinfo.�û��б��ض���û�userinfo]
typedef struct _USER_INFO
{
	uint32_t id;//id��
	uint32_t role;//��ɫ
	char username[64];//�û���
	char password[64];//����
	uint8_t status;//��¼״̬
}USER_INFO;

//AGV������Ϣ
typedef struct _AGV_BASE_INFO
{
	uint32_t id;
	char name[64];
	char ip[64];
	uint32_t port;
}AGV_BASE_INFO;

//AGVλ����Ϣ
typedef struct _AGV_POSITION_INFO
{
	uint32_t id;
	uint32_t x;
	uint32_t y;
	int32_t rotation;
}AGV_POSITION_INFO;

//AGV״̬��Ϣ
enum {
	AGV_MODE_AUTO = 0,//�Զ�ģʽ
	AGV_MODE_HAND = 1//�ֶ�ģʽ
};
typedef struct _AGV_STATUS_INFO
{
	int32_t mileage;
	int32_t currentRfid;
	int32_t nextRfid;
	int16_t current;
	int16_t voltage;
	int16_t positionMagneticStripe;
	int8_t pcbTemperature;
	int8_t motorTemperature;
	int8_t cpu;
	int8_t speed;
	int8_t angle;
	int8_t height;
	int8_t error_no;
	int8_t mode;
	int8_t recvQueueNumber;
	int8_t orderCount;
	int8_t CRC;//�Ƿ�ȥ����
}AGV_STATUS_INFO;

typedef struct _STATION_INFO
{
	int32_t id;
	int32_t x;
	int32_t y;
	int32_t rfid;
	int8_t r;
	int8_t g;
	int8_t b;
	int32_t occuagv;
	char name[64];
}STATION_INFO;

typedef struct _AGV_LINE
{
	int32_t id;
	int32_t startStation;
	int32_t endStation;
	int32_t length;
	int8_t r;
	int8_t g;
	int8_t b;
	int8_t draw;
}AGV_LINE;

typedef struct _AGV_ARC
{
	int32_t id;
	int32_t startStation;
	int32_t endStation;
	int32_t length;
	int8_t r;
	int8_t g;
	int8_t b;
	int8_t draw;
	double p1x;
	double p1y;
	double p2x;
	double p2y;
}AGV_ARC;

typedef struct _PATH_LEFT_MIDDLE_RIGHT {
	int lastLine;
	int nextLine;

	bool operator == (const _PATH_LEFT_MIDDLE_RIGHT &r) {
		return lastLine == r.lastLine && nextLine == r.nextLine;
	}

	bool operator < (const _PATH_LEFT_MIDDLE_RIGHT &r) const
	{
		if (lastLine != r.lastLine) {
			return lastLine<r.lastLine;
		}

		return nextLine<r.nextLine;
	}
}PATH_LEFT_MIDDLE_RIGHT;

enum {
	PATH_LMF_NOWAY = -2,//�������Ҫ��ͷ��ʻ
	PATH_LMR_LEFT = -1,
	PATH_LMR_MIDDLE = 0,
	PATH_LMR_RIGHT = 1,
};

typedef std::unique_lock<std::mutex>  UNIQUE_LCK;

unsigned char checkSum(unsigned char *data, int len);

int getInt32FromByte(char *data);

uint16_t getInt16FromByte(char *data);

uint8_t getInt8FromByte(char *data);
