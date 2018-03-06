#pragma once

#include <QList>
#include <QString>
#include <QVariant>
#include <QSqlDatabase>
#include <QMutex>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>

class DBManager :private boost::noncopyable, public boost::enable_shared_from_this<DBManager>
{
public:
	typedef boost::shared_ptr<DBManager> Pointer;
	
	static Pointer GetInstance()
	{
		static Pointer m_inst = Pointer(new DBManager());
		return m_inst;
	}

	virtual ~DBManager();

	//创建数据库连接
	bool createConnection();

	//检查表，如果表存在就OK，不存在创建表
	bool checkTables();

	//关闭数据库连接
	bool closeConnection();

	//执行sql语句
	//原则上，除了id 时间日期 外的其他字段统统text(vchar)
	bool exeSql(QString exeSql, QList<QVariant> args);

	//查询数据
	QList<QList<QVariant> > query(QString qeurysql, QList<QVariant> args);

private:
	DBManager();
	QSqlDatabase database;
	QMutex mutex;
};

