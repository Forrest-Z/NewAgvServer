#pragma once

#include <boost/unordered_map.hpp>
#include <boost/noncopyable.hpp>
#include <boost/array.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

#include <cstdio>
#include <string>
#include "TcpConnection.h"

using boost::asio::ip::tcp;

class Session
{
public:
	Session();

	~Session();
	

	Session(const Session& sess)
	{
		username = sess.username;
		id = sess.id;
		role = role;
	}

	std::string username;         //用户名
	int id;						  //ID
	int role;					  //角色
};

