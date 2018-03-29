#pragma once
#include "AgvLine.h"
#include "Common.h"

class AgvArc :
	public AgvLine
{
public:
	AgvArc();
	AgvArc(const AgvArc &b);
	AgvArc(AGV_ARC b);
	virtual ~AgvArc();

	AGV_ARC getSendData();

	//曲线只比直线多了两个点
	double p1x;
	double p1y;
	double p2x;
	double p2y;
};

