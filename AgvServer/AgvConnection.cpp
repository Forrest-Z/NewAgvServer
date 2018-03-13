#include "AgvConnection.h"
#include "AgvManager.h"
#include <iostream>


AgvConnection::AgvConnection(int _id, std::string _ip, int _port, AgvConnectionOnReadPackage _onReadPack) :
	id(_id),
	ip(_ip),
	port(_port),
	onReadPackage(_onReadPack)
{
	std::thread([this]()
	{
		io_context_p.reset(new boost::asio::io_context());
		socket_p.reset(new tcp::socket(*io_context_p));
		ep_p.reset(new tcp::endpoint(boost::asio::ip::address::from_string(ip), port));
		doConnect();
		io_context_p->run();
	}).detach();
}

AgvConnection::~AgvConnection()
{
}

void AgvConnection::doSend(char *data, int len)
{
	if (data == nullptr || len <= 0 || len > 64)return;
	if (socket_p->is_open())
	{
		boost::asio::async_write(*socket_p,
			boost::asio::buffer(data,
				len),
			[this](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				//发送成功
			}
			else
			{
				socket_p->close();
			}
		});
	}
}

void AgvConnection::reConnect()
{
	io_context_p->stop();
	if (socket_p->is_open())
		socket_p->close();
	std::thread([this]()
	{
		io_context_p.reset(new boost::asio::io_context());
		socket_p.reset(new tcp::socket(*io_context_p));
		ep_p.reset(new tcp::endpoint(boost::asio::ip::address::from_string(ip), port));
		doConnect();
		io_context_p->run();
	}).detach();
}


void AgvConnection::doConnect()
{
	socket_p->async_connect(*ep_p, [this](boost::system::error_code ec)
	{
		if (!ec)
		{
			//TODO:connect!
			do_read_header();
		}
		else {
			//TODO: disconnect! reconnect after 5 second
			std::this_thread::sleep_for(std::chrono::seconds(5));
			doConnect();
		}
	});

}

void AgvConnection::do_read_header()
{
	boost::asio::async_read(*socket_p,
		boost::asio::buffer(readbuff, 2),
		[this](boost::system::error_code ec, std::size_t /*length*/)
	{
		if (!ec)
		{
			if (readbuff[0] == 0x55 && readbuff[1] < 64) {
				readpacklen = readbuff[1];
				do_read_body();
			}
			else {
				do_read_header();
			}
		}
		else
		{
			socket_p->close();
			reConnect();
		}
	});
}

void AgvConnection::do_read_body()
{
	boost::asio::async_read(*socket_p,
		boost::asio::buffer(readbuff + 2, readpacklen - 2),
		[this](boost::system::error_code ec, std::size_t /*length*/)
	{
		if (!ec)
		{
			//TODO:处理
			if (onReadPackage)
				onReadPackage(readbuff, readpacklen + 1);
			//继续读取
			do_read_header();
		}
		else
		{
			socket_p->close();
			//TODO:onclose();
			reConnect();
		}
	});
}