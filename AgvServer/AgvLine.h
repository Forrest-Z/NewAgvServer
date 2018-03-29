#pragma once
#include <list>
#include "Common.h"

enum {
	AGV_LINE_COLOR_WHITE = 0,  //未算出路径最小值
	AGV_LINE_COLOR_GRAY,       //已经计算出一定的值，在Q队列中，但是尚未计算出最小值
	AGV_LINE_COLOR_BLACK,      //已算出路径最小值
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

	//直线弧线共有部分
	bool line;//true为直线 false为曲线
	int id;
	bool draw;
	double length;
	int startStation;
	int endStation;

	//这条线路的颜色RGB，显示用的
	int color_r;
	int color_g;
	int color_b;
	////////////////////////////下面这部分是计算路径用的
public:
	//计算路径用的
	int father;
	int distance;//起点到这条线的终点 的距离
	int color;

	bool operator == (const AgvLine &b) {
		return this->startStation == b.startStation&& this->endStation == b.endStation;
	}

	std::list<int> occuAgvs;
};

