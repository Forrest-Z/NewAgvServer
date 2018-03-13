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
	
	if (msg.head.body_length >= 128)
	{
		std::string username(msg.body, 64);
		std::string password(msg.body + 64, 64);
		if (username == "qyh"&&password == "1234")
		{
			//��¼�ɹ�
			std::cout << "user login success!" << std::endl;

			//��ѯ���ݿ��������ݣ�
			int id = 1;
			int role = 2;
			char username[64];//�û���
			char password[64];//����
			int status = 1;//��¼״̬

			//����״̬:
			SessionManager::Instance()->SaveSession(conn, id, username, role);
			conn->setId(id);

			response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
			response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_NO_ERROR;
			USER_INFO u;
			u.role = role;
			u.status = status;
			memcpy(u.username, username, sizeof(username));
			memcpy(u.password, password, sizeof(password));
			
			memcpy(response.body, &u, sizeof(u));
			response.head.body_length = sizeof(u);
		}
		else if (username == "qyh") {
			//�������
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

	//���ͷ���ֵ
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

	//TODO:�������ݿ��¼״̬
	conn->setId(SessionManager::Instance()->getUnloginId());
	SessionManager::Instance()->SaveSession(conn, conn->getId());

	//���ͷ���ֵ
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
		//TODO:�޸����ݿ������
	}
	
	//�ǳ�
	conn->setId(SessionManager::Instance()->getUnloginId());
	SessionManager::Instance()->SaveSession(conn, conn->getId());

	//���ͷ���ֵ
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

	//���ݿ��ѯ[������ȴ���1024�ĳ��ȣ����Էֳɶ���]


	//���ͷ���ֵ
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
	//���ݿ��ѯ
	//��Ҫmsg�а���һ��ID
	if (msg.head.body_length <= 32) {
		response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_LENGTH;
	}
	else {
		int id = 0;
		memcpy(&id, msg.body, sizeof(int));
		SessionManager::MapIdConnSession idConn = SessionManager::Instance()->getIdSock();
		if (idConn->find(id) != idConn->end())
		{
			//���û�����//�ǳ�
			(*idConn)[id]->setId(SessionManager::Instance()->getUnloginId());
			SessionManager::Instance()->SaveSession((*idConn)[id], (*idConn)[id]->getId());
		}
		//TODO: ���ݿ����


	}

	//���ͷ���ֵ
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

	//��Ҫmsg�а���һ��ID
	if (msg.head.body_length <= sizeof(USER_INFO) - 1) {//���ﲻ��Ҫ������¼״̬
		response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_LENGTH;
	}
	else {
		USER_INFO u;
		memcpy(&u, msg.body, sizeof(USER_INFO) - 1);
		u.status = 0;//����״̬
		//TODO: ���ݿ����


	}

	//���ͷ���ֵ
	conn->write_all(response);
}