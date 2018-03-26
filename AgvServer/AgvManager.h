#pragma once

#include <boost/unordered_map.hpp>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>

#include "Agv.h"
#include "Protocol.h"

class AgvManager :private boost::noncopyable, public boost::enable_shared_from_this<AgvManager>
{
public:
	typedef boost::shared_ptr<AgvManager> Pointer;

	//通过用户ID，查找AGV
	typedef boost::unordered_map<int, Agv::Pointer> MapIdAgv;
	typedef boost::shared_ptr<MapIdAgv> MapIdAgvPoint;

	void init();

	virtual ~AgvManager();

	static Pointer Instance()
	{
		static Pointer m_inst = Pointer(new AgvManager());
		return m_inst;
	}

	void SaveAgv(int id, Agv::Pointer agv)
	{
		std::unique_lock<std::mutex> lck(mtx);
		if (m_mapIdAgvs->find(id) != m_mapIdAgvs->end()) {
			auto itr = m_mapIdAgvs->find(id);
			if (itr != m_mapIdAgvs->end())
				m_mapIdAgvs->erase(itr);
		}
		(*m_mapIdAgvs)[id] = agv;
	}

	void RemoveAgv(int id)
	{
		std::unique_lock<std::mutex> lck(mtx);
		auto itr = m_mapIdAgvs->find(id);
		if (itr != m_mapIdAgvs->end())
			m_mapIdAgvs->erase(itr);
	}

	MapIdAgvPoint getAgvs()
	{
		std::unique_lock<std::mutex> lck(mtx);
		return m_mapIdAgvs;
	}

	Agv::Pointer getAgv(int id) {
		std::unique_lock<std::mutex> lck(mtx);
		return (*m_mapIdAgvs)[id];
	}

	std::list<Client_Response_Msg> getPositions();

	std::list<Client_Response_Msg> getstatuss();

private:
	AgvManager();

	std::mutex mtx;
	MapIdAgvPoint m_mapIdAgvs;
};

