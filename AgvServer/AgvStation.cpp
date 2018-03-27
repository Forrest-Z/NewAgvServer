#include "AgvStation.h"



AgvStation::AgvStation():
	x(0),
	y(0),
	name(""),
	id(0),
	rfid(0),
	occuAgv(0),
	color_r(255),
	color_g(0),
	color_b(0)
{
}

AgvStation::AgvStation(const AgvStation &b)
{
	x = b.x;
	y = b.y;
	name = b.name;
	id = b.id;
	rfid = b.rfid;
	occuAgv = b.occuAgv;
	color_r = b.color_r;
	color_g = b.color_g;
	color_b = b.color_b;
}

AgvStation::~AgvStation()
{
}
