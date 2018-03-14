#include "MsgProcessor.h"
#include <boost/unordered_map.hpp>
#include <boost/noncopyable.hpp>
#include <boost/array.hpp>
#include "UserManager.h"

void MsgProcess(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	typedef std::function<void(TcpConnection::Pointer, Client_Request_Msg)> ProcessFunction;

	UserManager::Pointer userManager = UserManager::Instance();

	static struct
	{
		CLIENT_MSG_TODO t;
		ProcessFunction f;
	} table[] =
	{
		{ CLIENT_MSG_TODO_USER_LOGIN,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_USER_LOGOUT,std::bind(&UserManager::logout,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_USER_CHANGED_PASSWORD,std::bind(&UserManager::changePassword,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_USER_LIST,std::bind(&UserManager::list,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_USER_DELTE,std::bind(&UserManager::remove,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_USER_ADD,std::bind(&UserManager::add,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_USER_MODIFY,std::bind(&UserManager::modify,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_MAP_CREATE_START,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_MAP_CREATE_ADD_STATION,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_MAP_CREATE_ADD_LINE,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_MAP_CREATE_ADD_ARC,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_MAP_CREATE_FINISH,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_MAP_LIST_STATION,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_MAP_LIST_LINE,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_MAP_LIST_ARC,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_HAND_REQUEST,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_HAND_RELEASE,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_HAND_FORWARD,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_HAND_BACKWARD,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_HAND_TURN_LEFT,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_HAND_TURN_RIGHT,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_AGV_MANAGE_LIST,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_AGV_MANAGE_ADD,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_AGV_MANAGE_DELETE,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_AGV_MANAGE_MODIFY,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_CREATE_TO_X,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_CREATE_AGV_TO_X,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_CREATE_PASS_Y_TO_X,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_CREATE_AGV_PASS_Y_TO_X,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_QUERY_STATUS,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_CANCEL,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_LIST_UNDO,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_LIST_DOING,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_LIST_DONE_TODAY,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_LIST_DURING,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_LOG_LIST_DURING,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_SUB_AGV_POSITION,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_CANCEL_SUB_AGV_POSITION,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_SUB_AGV_STATSU,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_CANCEL_SUB_AGV_STATSU,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_SUB_LOG,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_CANCEL_SUB_LOG,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_SUB_TASK,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_CANCEL_SUB_TASK,std::bind(&UserManager::login,userManager,std::placeholders::_1,std::placeholders::_2) }
	};
	table[msg.head.todo].f(conn, msg);
}