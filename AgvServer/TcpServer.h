#pragma once

#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/pool/pool.hpp>
#include <boost/asio.hpp>

#include "tcpconnection.h"

using boost::asio::ip::tcp;

class TcpServer
{
public:
	TcpServer(boost::asio::io_context& io_context, short port);
	~TcpServer();
	void start();

private:
	void do_accept();
	tcp::acceptor acceptor_;
};

