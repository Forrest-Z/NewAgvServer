#include "TcpServer.h"

using namespace std;

TcpServer::TcpServer(boost::asio::io_context& io_context, short port)
	:acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
{
}

TcpServer::~TcpServer()
{
}

void TcpServer::start()
{
	do_accept();
}

void TcpServer::do_accept()
{
	acceptor_.async_accept(
		[this](boost::system::error_code ec, tcp::socket socket)
	{
		if (!ec)
		{
			std::make_shared<TcpConnection>(std::move(socket))->start();
		}

		do_accept();
	});
}

