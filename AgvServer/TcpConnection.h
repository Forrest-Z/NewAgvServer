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
TCPConnection����һ��client����
*/
class TcpConnection : public boost::enable_shared_from_this<TcpConnection>
{
public:
	typedef boost::shared_ptr<TcpConnection> Pointer;
	TcpConnection(tcp::socket socket);
	~TcpConnection();

	void write_all(Client_Response_Msg msg);

	void start();

	tcp::socket& socket() { return m_socket; }

	void setId(int id)
	{
		_id = id;
	}

	int getId()
	{
		return _id;
	}

protected:
	void  _push(Client_Response_Msg msg);
	void  _write_head();
	void  _write_return_head();
	void  _write_body();
	void  _clear_queue(boost::lockfree::queue<Client_Response_Msg> &queue);
private:

	//////////��ȡ
	Client_Request_Msg read_one_msg;//��ȡһ����Ϣ�Ļ���
	void read_header();
	void read_body();

	tcp::socket m_socket;

	//д��
	Client_Response_Msg write_one_msg;
	boost::lockfree::queue<Client_Response_Msg>     m_queue;//���Ͷ���

	boost::atomic_bool                  _isWriting;
	boost::atomic_bool                  _isClosed;
	boost::atomic_int32_t              _push_times;
	boost::atomic_int32_t              _pop_times;
	boost::atomic_int32_t              _id;//��ǵ�ǰ�û�ID�����<=0,��ô����δ��¼

	int getSendQueueLength() const
	{
		return _push_times - _pop_times;
	}

};

