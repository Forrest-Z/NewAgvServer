#include "MsgProcessor.h"
#include <boost/unordered_map.hpp>
#include <boost/noncopyable.hpp>
#include <boost/array.hpp>

#define BIG_MSG_MAX_LENGTH		(24*1024*1024)//一般的图片也就24M以内

typedef struct one_big_msg
{
	char data[BIG_MSG_MAX_LENGTH];
	int length;
}ONE_BIG_MSG;

typedef boost::unordered_map<TcpConnection::Pointer, ONE_BIG_MSG> MsgPackMap;
typedef boost::shared_ptr<MsgPackMap> MsgPackMapPointer;

MsgPackMapPointer msgPackMap = boost::make_shared<MsgPackMap>();

void MsgProcess(TcpConnection::Pointer conn, Client_Msg msg)
{
	//0.组包
	int bodylength = msg.body.length;
	char *body = msg.body.data;

	if (msg.header.complete == CLIENT_MSG_COMPLETE_HEAD) {
		memcpy((*msgPackMap)[conn].data, msg.body.data, msg.body.length);
		(*msgPackMap)[conn].length = msg.body.length;
		return;
	}else if (msg.header.complete == CLIENT_MSG_COMPLTE_BODY) {
		if ((*msgPackMap)[conn].length + msg.body.length > BIG_MSG_MAX_LENGTH)return;//超出范围
		memcpy((*msgPackMap)[conn].data+ (*msgPackMap)[conn].length, msg.body.data, msg.body.length);
		(*msgPackMap)[conn].length += msg.body.length;
		return;
	}else if (msg.header.complete == CLIENT_MSG_COMPLTE_TAIL) {
		if ((*msgPackMap)[conn].length + msg.body.length > BIG_MSG_MAX_LENGTH)return;//超出范围
		memcpy((*msgPackMap)[conn].data + (*msgPackMap)[conn].length, msg.body.data, msg.body.length);
		(*msgPackMap)[conn].length += msg.body.length;
		bodylength = (*msgPackMap)[conn].length;
		body = (*msgPackMap)[conn].data;
	}

	//1.判断是否登录了
	if (msg.user_id <= 0) {
		//未登录
		if (msg.header.type != CLIENT_MSG_TYPE_USER || msg.header.todo != CLIENT_MSG_TODO_USER_LOGIN)return;//不是登录信息，不做处理
		//如果是登录信息
		if (bodylength < 128)return;
		std::string usernmae(body, 64);
		std::string password(body+64, 64);
		//TODO:

	}








}