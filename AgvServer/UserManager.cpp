#include "UserManager.h"
#include <iostream>
#include "DBManager.h"
#include "SessionManager.h"
#include "Server.h"

UserManager::UserManager() :
	m_idUsers(new MapIdUser)
{
}


UserManager::~UserManager()
{
}

void UserManager::login(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;

	if (msg.head.body_length >= 128)
	{
		std::string username(msg.body);
		std::string password(msg.body + 64);

		QString querySqlA = "select id,user_password,user_role,user_signState from agv_user where user_username=?";
		QList<QVariant> params;
		params << QString(msg.body);
		QList<QList<QVariant> > queryresult = DBManager::GetInstance()->query(querySqlA, params);
		if (queryresult.length() == 0) {
			std::cout << "username not exist" << std::endl;
			response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_USERNAME_NOT_EXIST;
			sprintf_s(response.return_head.error_info, "username not exist", strlen("username not exist"), sizeof(response.return_head.error_info));
		}
		else {
			if (queryresult.at(0).at(1) == QString(msg.body)) {
				//设置登录状态
				QString updateSql = "update agv_user set user_signState=1 where id=? ";
				params.clear();
				params << queryresult.at(0).at(0).toInt();
				if (!DBManager::GetInstance()->exeSql(updateSql, params)) {
					//登录失败
					std::cout << "save login status fail" << std::endl;
					response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_SAVE_SQL_FAIL;
					sprintf_s(response.return_head.error_info, "save login status fail", strlen("save login status fail"), sizeof(response.return_head.error_info));
				}
				else {
					//加入已登录的队伍中
					int id = queryresult.at(0).at(0).toInt();
					int role = queryresult.at(0).at(2).toInt();

					//设置状态:
					SessionManager::getInstance()->SaveSession(conn, id, username, role);
					conn->setId(id);

					response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
					response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_NO_ERROR;
					USER_INFO u;
					u.id = id;
					u.role = role;
					u.status = 1;
					memcpy(u.username, username.c_str(), username.length());
					memcpy(u.password, password.c_str(), password.length());

					memcpy(response.body, &u, sizeof(u));
					response.head.body_length = sizeof(u);
				}
				//}
			}
			else {
				//登录失败
				std::cout << "password error!" << std::endl;
				response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_PASSWORD_ERROR;
				sprintf_s(response.return_head.error_info, "password not correct", strlen("password not correct"), sizeof(response.return_head.error_info));
			}
		}
	}

	//发送返回值
	conn->write_all(response);
}

void UserManager::logout(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
	response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_NO_ERROR;

	QString updateSql = "update agv_user set user_signState=0 where id=? ";
	QList<QVariant> params;
	params << conn->getId();
	DBManager::GetInstance()->exeSql(updateSql, params);

	//登出
	conn->setId(SessionManager::getInstance()->getUnloginId());
	SessionManager::getInstance()->SaveSession(conn, conn->getId());

	//发送返回值
	conn->write_all(response);
}

void UserManager::changePassword(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &(msg.head), sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
	response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_NO_ERROR;

	if (msg.head.body_length <= 0) {
		response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_LENGTH;
	}
	else {
		std::string newPassword(msg.body, msg.head.body_length);
		//TODO:修改数据库的密码
		QString updateSql = "update agv_user set user_password=? where id = ?";
		QList<QVariant> params;
		params << QString(msg.body) << conn->getId();
		if (!DBManager::GetInstance()->exeSql(updateSql, params)) {
			//登录失败
			std::cout << "save new password to database fail" << std::endl;
			response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_SAVE_SQL_FAIL;
			sprintf_s(response.return_head.error_info, "save new password to database fail", strlen("save new password to database fail"), sizeof(response.return_head.error_info));
		}
	}

	//登出//修改完密码，需要重新登录？
	conn->setId(SessionManager::getInstance()->getUnloginId());
	SessionManager::getInstance()->SaveSession(conn, conn->getId());
	//或者直接重置他 的密码。不做登出处理?

	//发送返回值
	conn->write_all(response);
}

void UserManager::list(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
	response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_NO_ERROR;

	//数据库查询[如果长度大于1024的长度，可以分成多条]
	QString sql = "select id,user_name,user_password,user_role,user_status from agv_user where role <=?";
	QList<QVariant> params;
	SessionManager::MapConnSessionPointer mcs = SessionManager::getInstance()->getSession();
	params << (*mcs)[conn].role;
	QList<QList<QVariant> > queryresult = DBManager::GetInstance()->query(sql, params);
	bool needSendLast = true;
	if (queryresult.length() != 0) {
		int pos = 0;
		foreach(auto l, queryresult)
		{
			USER_INFO u;
			memset(&u, 0, sizeof(USER_INFO));
			u.id = l.at(0).toInt();
			QString name = l.at(1).toString();
			QString password = l.at(2).toString();
			memcpy(u.username, name.toStdString().c_str(), name.toStdString().length());
			memcpy(u.password, password.toStdString().c_str(), password.toStdString().length());
			u.role = l.at(3).toInt();
			u.status = l.at(4).toInt();
			memcpy(response.body + pos,&u, sizeof(USER_INFO));
			response.head.body_length += sizeof(USER_INFO);
			pos += sizeof(USER_INFO);
			needSendLast = true;
			if (pos + sizeof(USER_INFO) > CLIENT_MSG_REQUEST_BODY_MAX_SIZE)
			{
				conn->write_all(response);
				pos = 0;
				response.head.body_length = 0;
				needSendLast = false;
			}
		}
	}
	//发送返回值
	if(needSendLast)
		conn->write_all(response);
}

void UserManager::remove(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
	response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_NO_ERROR;
	//数据库查询
	//需要msg中包含一个ID
	if (msg.head.body_length <= sizeof(int)) {
		response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_LENGTH;
	}
	else {
		int id = 0;
		memcpy(&id, msg.body, sizeof(int));
		SessionManager::MapIdConnSession idConn = SessionManager::getInstance()->getIdSock();
		if (idConn->find(id) != idConn->end())
		{
			//该用户在线//登出
			(*idConn)[id]->setId(SessionManager::getInstance()->getUnloginId());
			SessionManager::getInstance()->SaveSession((*idConn)[id], (*idConn)[id]->getId());
		}
		//TODO: 数据库操作
		QString sql = "delete from agv_user where id=?";
		QList<QVariant> params;
		params << id;
		if (!DBManager::GetInstance()->exeSql(sql, params))
		{
			response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;
			response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_SAVE_SQL_FAIL;
		}
	}

	//发送返回值
	conn->write_all(response);
}

void UserManager::add(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
	response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_NO_ERROR;

	//需要msg中包含一个ID
	if (msg.head.body_length <= sizeof(USER_INFO) - 1) {//这里不需要包含登录状态
		response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_LENGTH;
	}
	else {
		USER_INFO u;
		memcpy(&u, msg.body, sizeof(USER_INFO) - 1);
		u.status = 0;//在线状态
		//TODO: 数据库操作
		QString sql = "insert into agv_user user_username, user_password,user_role,user_status values(?,?,?,0);";
		QList<QVariant> params;
		params << QString(u.username)<<QString(u.password)<<u.role;
		if (!DBManager::GetInstance()->exeSql(sql, params))
		{
			response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;
			response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_SAVE_SQL_FAIL;
		}
		//TODO:返回插入后的ID值
	}
	//发送返回值
	conn->write_all(response);
}

void UserManager::modify(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
	response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_NO_ERROR;

	//需要msg中包含一个ID
	if (msg.head.body_length <= sizeof(USER_INFO) - 1) {//这里不需要包含登录状态
		response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_LENGTH;
	}
	else {
		USER_INFO u;
		memcpy(&u, msg.body, sizeof(USER_INFO) - 1);
		u.status = 0;//在线状态
					 //TODO: 数据库操作
		QString updateSql = "update agv_user set user_username=?,user_password=?,user_role=? where id=?";
		QList<QVariant> params;
		params.append(QString(u.username));
		params.append(QString(u.password));
		params.append(u.role);
		params.append(u.id);
		if (!DBManager::GetInstance()->exeSql(updateSql, params)) {
			//成功
			response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;
			response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_SAVE_SQL_FAIL;
		}
	}
	//发送返回值
	conn->write_all(response);
}