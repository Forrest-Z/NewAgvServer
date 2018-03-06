#pragma once

#include <boost/unordered_map.hpp>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>

#include "Agv.h"

class AgvManager :private boost::noncopyable, public boost::enable_shared_from_this<AgvManager>
{
public:
	typedef boost::shared_ptr<AgvManager> Pointer;

	//ͨ���û�ID������AGV
	typedef boost::unordered_map<int, Agv::Pointer> MapIdAgv;
	typedef boost::shared_ptr<MapIdAgv> MapIdAgvPoint;

	void init();

	virtual ~AgvManager();

	static Pointer Instance()
	{
		static Pointer m_inst = Pointer(new AgvManager());
		return m_inst;
	}

	//�û�ID���û�sock ����
	void SaveAgv(int id, Agv::Pointer agv)
	{
		if (m_mapIdAgvs->find(id) != m_mapIdAgvs->end()) {
			//��������Ѿ������ˣ�ɾ���ɵ�ID
			auto itr = m_mapIdAgvs->find(id);
			if (itr != m_mapIdAgvs->end())
				m_mapIdAgvs->erase(itr);
		}
		(*m_mapIdAgvs)[id] = agv;
	}

	void RemoveSession(int id)
	{
		auto itr = m_mapIdAgvs->find(id);
		if (itr != m_mapIdAgvs->end())
			m_mapIdAgvs->erase(itr);
	}

	MapIdAgvPoint getSession()
	{
		return m_mapIdAgvs;
	}

private:
	AgvManager();

	MapIdAgvPoint m_mapIdAgvs;
};

