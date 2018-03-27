#include "AgvArc.h"
#include "Common.h"

AgvArc::AgvArc()
{
	line = true;
}


AgvArc::~AgvArc()
{
}

AgvArc::AgvArc(const AgvArc &b)
{
	line = b.line;
	p1x = b.p1x;
	p2x = b.p2x;
	p1y = b.p1y;
	p2y = b.p2y;
	id = b.id;
	draw = b.draw;
	length = b.length;
	startStation = b.startStation;
	endStation = b.endStation;
	color_r = b.color_r;
	color_g = b.color_g;
	color_b = b.color_b;
}

AGV_ARC AgvArc::getSendData()
{
	AGV_ARC b;
	b.id = id;
	b.draw = draw;
	b.startStation = startStation;
	b.endStation = endStation;
	b.r = color_r;
	b.g = color_g;
	b.b = color_b;
	b.length = length;
	b.p1x = p1x;
	b.p1y = p1y;
	b.p2x = p2x;
	b.p2y = p2y;

	return b;
}