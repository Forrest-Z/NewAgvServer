#pragma once

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/thread.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>

using boost::asio::ip::tcp;

class AgvConnection : public boost::enable_shared_from_this<AgvConnection>
{
public:
	typedef boost::shared_ptr<AgvConnection> Pointer;
	AgvConnection(int _id,std::string ip, int port);
	virtual ~AgvConnection();
	//不起线程和缓存因为要求速度高
	void doSend(char *data, int len);
private:
	void doConnect();
	void do_read_header();

	void do_read_body();
	
	std::unique_ptr<boost::asio::io_context> io_context_p;
	std::unique_ptr<tcp::socket> socket_p;
	std::unique_ptr<tcp::endpoint> ep_p;

	char readbuff[64];
	int readpacklen;

	std::string ip;
	int port;
	int id;
};

