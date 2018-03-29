#pragma once
#include <list>
#include "Common.h"

enum {
	AGV_LINE_COLOR_WHITE = 0,  //δ���·����Сֵ
	AGV_LINE_COLOR_GRAY,       //�Ѿ������һ����ֵ����Q�����У�������δ�������Сֵ
	AGV_LINE_COLOR_BLACK,      //�����·����Сֵ
};

class AgvLine
{
public:
	AgvLine();
	AgvLine(const AgvLine &b);
	virtual ~AgvLine();

	bool operator <(const AgvLine &b) {
		return id<b.id;
	}

	AGV_LINE getSendData();

	//ֱ�߻��߹��в���
	bool line;//trueΪֱ�� falseΪ����
	int id;
	bool draw;
	double length;
	int startStation;
	int endStation;

	//������·����ɫRGB����ʾ�õ�
	int color_r;
	int color_g;
	int color_b;
	////////////////////////////�����ⲿ���Ǽ���·���õ�
public:
	//����·���õ�
	int father;
	int distance;//��㵽�����ߵ��յ� �ľ���
	int color;

	bool operator == (const AgvLine &b) {
		return this->startStation == b.startStation&& this->endStation == b.endStation;
	}

	std::list<int> occuAgvs;
};

