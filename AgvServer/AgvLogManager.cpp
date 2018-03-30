#include "AgvLogManager.h"
#include "DBManager.h"


AgvLogManager::AgvLogManager():
	m_queue(new boost::lockfree::queue<AGV_LOG>(64))
{
}


AgvLogManager::~AgvLogManager()
{
}

void AgvLogManager::init(AgvLogManager::PUB_LOG_FUNC _cb)
{
	cb = _cb;
	//启动一个线程，处理日志队列、并发送给日志订阅者
	std::thread([this]()
	{
		while (true) {
			AGV_LOG log;
			if (m_queue->pop(log))
			{
				QString insertSql = "insert into agv_log(log_level,log_time,log_msg)values(?,?,?);";
				QList<QVariant> params;
				params << log.level << log.time << log.msg;

				DBManager::getInstance()->exeSql(insertSql, params);
				
				//执行发布任务
				cb(log);
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	}).detach();
}

void AgvLogManager::log(AGV_LOG_LEVEL level, std::string msg)
{
	std::string nowstr = getTimeStrNow();
	AGV_LOG log;
	memcpy(&log, 0, sizeof(AGV_LOG));
	memcpy_s(log.time, nowstr.length(), nowstr.c_str(),24);
	log.level = level;
	memcpy_s(log.msg, msg.length(), msg.c_str(),  LOG_MSG_MAX_LENGTH);

	//入队
	m_queue->push(log);
}