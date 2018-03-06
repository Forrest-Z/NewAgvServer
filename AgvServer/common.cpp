#include "Common.h"

unsigned char checkSum(unsigned char *data, int len)
{
	int sum = 0;
	for (int i = 0;i < len;++i) {
		//int v = data[i] &0xFF;
		sum += data[i] & 0xFF;
		sum &= 0xFF;
	}
	return sum & 0xFF;
}


int getInt32FromByte(char *data)
{
	int j = data[0] & 0x000000ff;
	j |= ((data[1] << 8) & 0x0000ff00);
	j |= ((data[2] << 16) & 0x00ff0000);
	j |= ((data[3] << 24) & 0xff000000);
	return j;
}

uint16_t getInt16FromByte(char *data)
{
	uint16_t j = data[0] & 0x000000ff;
	j |= ((data[1] << 8) & 0x0000ff00);
	return j;
}

uint8_t getInt8FromByte(char *data)
{
	return (uint8_t)(data[0]);
}