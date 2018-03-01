#include "TcpConnection.h"
#include "Server.h"
#include <iostream>

TcpConnection::TcpConnection(tcp::socket socket)
	:m_socket(std::move(socket)),
	m_queue(1024)
{
	_isWriting = false;
	_isClosed = false;
	_push_times = 0;
	_pop_times = 0;
	_isLogin = false;
}

TcpConnection::~TcpConnection()
{
}


void TcpConnection::start()
{
	read_header();
}

void TcpConnection::_clear_queue(boost::lockfree::queue<Client_Msg> &queue)
{
	Client_Msg buf;
	while (queue.pop(buf));
}

void TcpConnection::_push(Client_Msg msg)
{
	size_t  tms = 0;

	while (getSendQueueLength() > 100)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(1000));
		tms++;
		if (tms >= 1000)
		{
			break;
		}
	}

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
		boost::asio::buffer(&(write_one_msg.header), sizeof(Client_Msg_Head)),
		[this, self](boost::system::error_code ec, std::size_t /*length*/)
	{
		if (!ec)
		{
			if (write_one_msg.header.body_length > 0) {
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
		boost::asio::buffer(&(write_one_msg.body), write_one_msg.header.body_length),
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

void TcpConnection::write_all(Client_Msg msg)
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
		boost::asio::buffer(read_head_buffer, sizeof(Client_Msg_Head)),
		[this, self](boost::system::error_code ec, std::size_t size)
	{
		if (!ec)
		{
			memcpy(&(read_one_msg.header), read_head_buffer, sizeof(Client_Msg_Head));
			if (read_one_msg.header.body_length > 0 ){
				if (read_one_msg.header.body_length <= TCP_ONE_MSG_MAX_SIZE
					&& read_one_msg.header.head == TCP_ONE_MSG_HEAD_HEAD
					&& read_one_msg.header.tail == TCP_ONE_MSG_HEAD_TAIL) 
				{
					read_body();
				}
				else {
					read_header();
				}
			}
			else {
				Server::GetInstance()->pushPackage(read_one_msg);
				read_header();
			}
		}
		else
		{
			//TODO:µôÏß
			_isClosed = true;
			std::cout << "socket closed!" << std::endl;
			//Server::GetInstance()->GetPlayerLogin()->SavePlayerOfflineData(shared_from_this());
			m_socket.close();
		}
	});
}

void TcpConnection::read_body()
{
	auto self(shared_from_this());
	boost::asio::async_read(m_socket,
		boost::asio::buffer(read_body_buffer, read_one_msg.header.body_length),
		[this, self](boost::system::error_code ec, std::size_t size)
	{
		if (!ec)
		{
			memcpy(read_one_msg.body.data, read_body_buffer, size);
			read_one_msg.body.length = size;
			Server::GetInstance()->pushPackage(read_one_msg);
			read_header();
		}
		else
		{
			//TODO:µôÏß
			_isClosed = true;
			std::cout << "socket closed!" << std::endl;
			//Server::GetInstance()->GetPlayerLogin()->SavePlayerOfflineData(shared_from_this());
			m_socket.close();
		}
	});
}
