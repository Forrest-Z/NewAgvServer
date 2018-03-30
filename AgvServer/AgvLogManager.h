#pragma once

#include <boost/noncopyable.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/lockfree/queue.hpp>

#include "Common.h"

class AgvLogManager :private boost::noncopyable, public boost::enable_shared_from_this<AgvLogManager>
{
public:
	typedef boost::shared_ptr<AgvLogManager> Pointer;

	typedef std::function<void(AGV_LOG log)> PUB_LOG_FUNC;
	//��ʼ���������(�㲥����T)
	void init(AgvLogManager::PUB_LOG_FUNC _cb);

	virtual ~AgvLogManager();

	static Pointer getInstance()
	{
		static Pointer m_inst = Pointer(new AgvLogManager());
		return m_inst;
	}

	//��¼һ����Ϣ
	void log(AGV_LOG_LEVEL level, std::string msg);

private:
	AgvLogManager();
	boost::lockfree::queue<AGV_LOG> *m_queue;//��־���У���ֹ��־���� ������Ϣ�����߳�
	PUB_LOG_FUNC cb;
};

