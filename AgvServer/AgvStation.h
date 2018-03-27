#pragma once
#include <string>

class AgvStation
{
public:
	AgvStation();
	AgvStation(const AgvStation &b);
	~AgvStation();

	bool operator <(const AgvStation &b) {
		return id<b.id;
	}

	double x;
	double y;
	std::string name;
	int id;
	int rfid;

	int color_r;
	int color_g;
	int color_b;

	bool operator == (const AgvStation &b) {
		return this->id == b.id;
	}

	////用于计算线路的，不存库
	int occuAgv;
};

