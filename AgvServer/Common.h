#pragma once
#include <cstdint>

//�û���Ϣ�ṹ��[��¼�ɹ�ʱ������һ�����û���userinfo.�û��б��ض���û�userinfo]
typedef struct _USER_INFO
{
	int32_t id;//id��
	int32_t role;//��ɫ
	char username[64];//�û���
	char password[64];//����
	int8_t status;//��¼״̬
}USER_INFO;

//AGV������Ϣ
typedef struct _AGV_BASE_INFO
{
	int32_t id;
	char name[64];
	char ip[64];
	int32_t port;
}AGV_BASE_INFO;

//AGVλ����Ϣ
typedef struct _AGV_POSITION_INFO
{
	int32_t x;
	int32_t y;
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
	float p1x;
	float p1y;
	float p2x;
	float p2y;
}AGV_ARC;


unsigned char checkSum(unsigned char *data, int len);

int getInt32FromByte(char *data);

uint16_t getInt16FromByte(char *data);

uint8_t getInt8FromByte(char *data);
