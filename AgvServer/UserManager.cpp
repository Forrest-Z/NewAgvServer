#include "UserManager.h"
#include <iostream>
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
	response.body.length = 0;
	
	if (msg.body.length >= 128)
	{
		std::string username(msg.body.data, 64);
		std::string password(msg.body.data + 64, 64);
		if (username == "qyh"&&password == "1234")
		{
			//登录成功
			std::cout << "user login success!" << std::endl;

			//查询数据库如下数据：
			int id = 1;
			int role = 2;
			char username[64];//用户名
			char password[64];//密码
			int status = 1;//登录状态

			//设置状态:
			SessionManager::Instance()->SaveSession(conn, id, username, role);
			conn->setId(id);

			response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
			response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_NO_ERROR;
			USER_INFO u;
			u.role = role;
			u.status = status;
			memcpy(u.username, username, sizeof(username));
			memcpy(u.password, password, sizeof(password));
			
			memcpy(response.body.data, &u, sizeof(u));
			response.body.length = sizeof(u);
		}
		else if (username == "qyh") {
			//密码错误
			std::cout << "password error!" << std::endl;
			response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_PASSWORD_ERROR;
			sprintf_s(response.return_head.error_info, "password not correct", strlen("password not correct"),sizeof(response.return_head.error_info));
		}
		else {
			std::cout << "username not exist" << std::endl;
			response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_USERNAME_NOT_EXIST;
			sprintf_s(response.return_head.error_info, "username not exist", strlen("username not exist"), sizeof(response.return_head.error_info));
		}
	}
	else {
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_LENGTH;
		sprintf_s(response.return_head.error_info, "login length <128", strlen("login length <128"), sizeof(response.return_head.error_info));
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
	response.body.length = 0;

	//TODO:设置数据库登录状态
	conn->setId(SessionManager::Instance()->getUnloginId());
	SessionManager::Instance()->SaveSession(conn, conn->getId());

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
	response.body.length = 0;
	
	if (msg.body.length <= 0) {
		response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_LENGTH;
	}
	else {
		std::string newPassword(msg.body.data, msg.body.length);
		//TODO:修改数据库的密码
	}
	
	//登出
	conn->setId(SessionManager::Instance()->getUnloginId());
	SessionManager::Instance()->SaveSession(conn, conn->getId());

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
	response.body.length = 0;

	//数据库查询[如果长度大于1024的长度，可以分成多条]


	//发送返回值
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
	response.body.length = 0;
	//数据库查询
	//需要msg中包含一个ID
	if (msg.body.length <= 32) {
		response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_LENGTH;
	}
	else {
		int id = 0;
		memcpy(&id, msg.body.data, sizeof(int));
		SessionManager::MapIdConnSession idConn = SessionManager::Instance()->getIdSock();
		if (idConn->find(id) != idConn->end())
		{
			//该用户在线//登出
			(*idConn)[id]->setId(SessionManager::Instance()->getUnloginId());
			SessionManager::Instance()->SaveSession((*idConn)[id], (*idConn)[id]->getId());
		}
		//TODO: 数据库操作


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
	response.body.length = 0;

	//需要msg中包含一个ID
	if (msg.body.length <= sizeof(USER_INFO) - 1) {//这里不需要包含登录状态
		response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_LENGTH;
	}
	else {
		USER_INFO u;
		memcpy(&u, msg.body.data, sizeof(USER_INFO) - 1);
		u.status = 0;//在线状态
		//TODO: 数据库操作


	}

	//发送返回值
	conn->write_all(response);
}