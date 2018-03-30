#pragma once

#include <boost/unordered_map.hpp>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>

#include "Agv.h"
#include "Protocol.h"
#include "TcpConnection.h"

class AgvManager :private boost::noncopyable, public boost::enable_shared_from_this<AgvManager>
{
public:
	typedef boost::shared_ptr<AgvManager> Pointer;

	//Õ®π˝ID£¨≤È’“AGV
	typedef boost::unordered_map<int, Agv::Pointer> MapIdAgv;
	typedef boost::shared_ptr<MapIdAgv> MapIdAgvPoint;

	void init();

	virtual ~AgvManager();

	static Pointer getInstance()
	{
		static Pointer m_inst = Pointer(new AgvManager());
		return m_inst;
	}

	void interHandRequest(TcpConnection::Pointer conn, Client_Request_Msg msg);
	void interHandRelease(TcpConnection::Pointer conn, Client_Request_Msg msg);
	void interHandForward(TcpConnection::Pointer conn, Client_Request_Msg msg);
	void interHandBackward(TcpConnection::Pointer conn, Client_Request_Msg msg);
	void interHandTurnLeft(TcpConnection::Pointer conn, Client_Request_Msg msg);
	void interHandTurnRight(TcpConnection::Pointer conn, Client_Request_Msg msg);
	void interList(TcpConnection::Pointer conn, Client_Request_Msg msg);
	void interAdd(TcpConnection::Pointer conn, Client_Request_Msg msg);
	void interDelete(TcpConnection::Pointer conn, Client_Request_Msg msg);
	void interModify(TcpConnection::Pointer conn, Client_Request_Msg msg);


	void onFinish(Agv::Pointer agv);

	void onError(int code, Agv::Pointer agv);
	
	void onInterupt(Agv::Pointer agv);
	
	void updateOdometer(int odometer, Agv::Pointer agv);

	void updateStationOdometer(int rfid, int odometer, Agv::Pointer agv);
	
	void SaveAgv(int id, Agv::Pointer agv)
	{
		UNIQUE_LCK lck(mtx);
		if (m_mapIdAgvs->find(id) != m_mapIdAgvs->end()) {
			auto itr = m_mapIdAgvs->find(id);
			if (itr != m_mapIdAgvs->end())
				m_mapIdAgvs->erase(itr);
		}
		(*m_mapIdAgvs)[id] = agv;
	}

	void RemoveAgv(int id)
	{
		UNIQUE_LCK lck(mtx);
		auto itr = m_mapIdAgvs->find(id);
		if (itr != m_mapIdAgvs->end())
			m_mapIdAgvs->erase(itr);
	}

	/*MapIdAgvPoint getAgvs()
	{
		UNIQUE_LCK lck(mtx);
		return m_mapIdAgvs;
	}

	Agv::Pointer getAgv(int id) {
		UNIQUE_LCK lck(mtx);
		return (*m_mapIdAgvs)[id];
	}*/

	std::list<Client_Response_Msg> getPositions();

	std::list<Client_Response_Msg> getstatuss();

private:
	AgvManager();

	std::mutex mtx;
	MapIdAgvPoint m_mapIdAgvs;
};

