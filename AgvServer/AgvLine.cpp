#include "AgvLine.h"
#include "Common.h"

AgvLine::AgvLine() :
	line(true),
	id(0),
	draw(false),
	length(0),
	color_r(0),
	color_g(255),
	color_b(0),

	father(0),
	distance(0),
	color(0)
{
}


AgvLine::~AgvLine()
{
}

AgvLine::AgvLine(const AgvLine &b)
{
	line = b.line;
	id = b.id;
	draw = b.draw;
	length = b.length;
	startStation = b.startStation;
	endStation = b.endStation;
	color_r = b.color_r;
	color_g = b.color_g;
	color_b = b.color_b;
}

AgvLine::AgvLine(AGV_LINE b)
{
	line = true;
	id = b.id;
	draw = b.draw;
	length = b.length;
	startStation = b.startStation;
	endStation = b.endStation;
	color_r = b.r;
	color_g = b.g;
	color_b = b.b;
}

AGV_LINE AgvLine::getSendData()
{
	AGV_LINE b;
	b.id = id;
	b.draw = draw;
	b.startStation = startStation;
	b.endStation = endStation;
	b.r = color_r;
	b.g = color_g;
	b.b = color_b;
	b.length = length;

	return b;
}