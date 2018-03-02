#include "TcpConnection.h"
#include "Server.h"
#include "SessionManager.h"
#include <iostream>

TcpConnection::TcpConnection(tcp::socket socket)
	:m_socket(std::move(socket)),
	m_queue(5)
{
	_isWriting = false;
	_isClosed = false;
	_push_times = 0;
	_pop_times = 0;
	
}

TcpConnection::~TcpConnection()
{
}


void TcpConnection::start()
{
	_id = SessionManager::Instance()->getUnloginId();
	SessionManager::Instance()->SaveSession(shared_from_this(), _id);
	read_header();
}

void TcpConnection::_clear_queue(boost::lockfree::queue<Client_Response_Msg> &queue)
{
	Client_Response_Msg buf;
	while (queue.pop(buf));
}

void TcpConnection::_push(Client_Response_Msg msg)
{
	size_t  tms = 0;
	m_queue.push(msg);
	_push_times += 1;

	if (!_isWriting)
	{
		_write_head();
	}
}
void TcpConnection::_write_head()
{
	auto self(shared_from_this());
	if (m_queue.empty()) {
		_isWriting = false;
		_push_times = 0;
		_pop_times = 0;
		return;
	}

	m_queue.pop(write_one_msg);
	_pop_times += 1;

	_isWriting = true;

	boost::asio::async_write(m_socket,
		boost::asio::buffer(&(write_one_msg.head), sizeof(Client_Common_Head)),
		[this, self](boost::system::error_code ec, std::size_t /*length*/)
	{
		if (!ec)
		{
			_write_return_head();
		}
		else {
			_isWriting = false;
		}
	});
}

void TcpConnection::_write_return_head()
{
	auto self(shared_from_this());
	boost::asio::async_write(m_socket,
		boost::asio::buffer(&(write_one_msg.return_head), sizeof(CLIENT_RETURN_MSG_HEAD)),
		[this, self](boost::system::error_code ec, std::size_t /*length*/)
	{
		if (!ec)
		{
			if (write_one_msg.head.body_length > 0)
			{
				_write_body();
			}
			else {
				_write_head();
			}
		}
		else {
			_isWriting = false;
		}
	});
}

void TcpConnection::_write_body()
{
	auto self(shared_from_this());
	boost::asio::async_write(m_socket,
		boost::asio::buffer(&(write_one_msg.body), write_one_msg.head.body_length),
		[this, self](boost::system::error_code ec, std::size_t /*length*/)
	{
		if (!ec)
		{
			_write_head();
		}
		else {
			_isWriting = false;
		}
	});
}

void TcpConnection::write_all(Client_Response_Msg msg)
{
	if (_isClosed)
	{
		//clear the queue
		_clear_queue(m_queue);
		_push_times = 0;
		_pop_times = 0;
		return;
	}
	_push(msg);
}

void TcpConnection::read_header()
{
	auto self(shared_from_this());
	boost::asio::async_read(m_socket,
		boost::asio::buffer(read_head_buffer, sizeof(Client_Common_Head)),
		[this, self](boost::system::error_code ec, std::size_t size)
	{
		if (!ec)
		{
			memcpy(&(read_one_msg.head), read_head_buffer, sizeof(Client_Common_Head));
			if (read_one_msg.head.body_length > 0 ){
				if (read_one_msg.head.body_length <= CLIENT_MSG_REQUEST_BODY_MAX_SIZE
					&& read_one_msg.head.head == CLIENT_COMMON_HEAD_HEAD
					&& read_one_msg.head.tail == CLIENT_COMMON_HEAD_TAIL) 
				{
					read_body();
				}
				else {
					read_header();
				}
			}
			else {
				read_one_msg.id = _id;
				Server::GetInstance()->pushPackage(read_one_msg);
				read_header();
			}
		}
		else
		{
			//µôÏß
			_isClosed = true;
			std::cout << "socket closed!" << std::endl;
			//TODO:
			SessionManager::Instance()->RemoveSession(shared_from_this());
			m_socket.close();
		}
	});
}

void TcpConnection::read_body()
{
	auto self(shared_from_this());
	boost::asio::async_read(m_socket,
		boost::asio::buffer(read_body_buffer, read_one_msg.head.body_length),
		[this, self](boost::system::error_code ec, std::size_t size)
	{
		if (!ec)
		{
			memcpy(read_one_msg.body.data, read_body_buffer, size);
			read_one_msg.body.length = size;
			read_one_msg.id = _id;
			Server::GetInstance()->pushPackage(read_one_msg);
			read_header();
		}
		else
		{
			//µôÏß
			_isClosed = true;
			std::cout << "socket closed!" << std::endl;
			//TODO:
			SessionManager::Instance()->RemoveSession(shared_from_this());
			m_socket.close();
		}
	});
}
