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

	//�������ݿ�����
	bool createConnection();

	//�����������ھ�OK�������ڴ�����
	bool checkTables();

	//�ر����ݿ�����
	bool closeConnection();

	//ִ��sql���
	//ԭ���ϣ�����id ʱ������ ��������ֶ�ͳͳtext(vchar)
	bool exeSql(QString exeSql, QList<QVariant> args);

	//��ѯ����
	QList<QList<QVariant> > query(QString qeurysql, QList<QVariant> args);

private:
	DBManager();
	QSqlDatabase database;
	QMutex mutex;
};

