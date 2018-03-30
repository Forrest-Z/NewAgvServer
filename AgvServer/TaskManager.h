#pragma once

#include <boost/noncopyable.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>

class TaskManager :private boost::noncopyable, public boost::enable_shared_from_this<TaskManager>
{
public:

	typedef boost::shared_ptr<TaskManager> Pointer;

	virtual ~TaskManager();

	static Pointer getInstance()
	{
		static Pointer m_inst = Pointer(new TaskManager());
		return m_inst;
	}
private:
	TaskManager();
};

