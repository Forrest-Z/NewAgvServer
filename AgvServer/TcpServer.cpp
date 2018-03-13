#include "TcpServer.h"
#include "SessionManager.h"
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
			TcpConnection::Pointer  conn = boost::make_shared<TcpConnection>(std::move(socket));
			conn->setId(SessionManager::Instance()->getUnloginId());
			SessionManager::Instance()->SaveSession(conn, conn->getId());
			conn->start();
		}

		do_accept();
	});
}

