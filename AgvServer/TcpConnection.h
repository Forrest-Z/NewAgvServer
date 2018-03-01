#pragma once

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>
#include "Protocol.h"

using  boost::asio::ip::tcp;

/*
TCPConnection管理一个client连接
*/
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
	typedef boost::shared_ptr<TcpConnection> Pointer;
	TcpConnection(tcp::socket socket);
	~TcpConnection();

	void write_all(Client_Msg msg);

	void start();

	tcp::socket& socket() { return m_socket; }

	void setIsLogin(bool b)
	{
		_isLogin = b;
	}

protected:
	void  _push(Client_Msg msg);
	void  _write_head();
	void  _write_body();
	void  _clear_queue(boost::lockfree::queue<Client_Msg> &queue);
private:
	Client_Msg read_one_msg;//读取一条消息的缓存
	char read_head_buffer[sizeof(Client_Msg_Head)];
	char read_body_buffer[sizeof(Client_Msg_Body)];

	Client_Msg write_one_msg;

	void read_header();

	void read_body();

	tcp::socket m_socket;

	boost::lockfree::queue<Client_Msg>     m_queue;//发送队列
	boost::atomic_bool                  _isWriting;
	boost::atomic_bool                  _isClosed;
	boost::atomic_uint32_t              _push_times;
	boost::atomic_uint32_t              _pop_times;
	boost::atomic_bool                  _isLogin;//是否已经登录

	int getSendQueueLength() const
	{
		return _push_times - _pop_times;
	}

};

