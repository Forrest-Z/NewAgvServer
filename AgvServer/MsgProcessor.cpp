#include "MsgProcessor.h"
#include <boost/unordered_map.hpp>
#include <boost/noncopyable.hpp>
#include <boost/array.hpp>
#include "UserManager.h"
#include "SessionManager.h"
#include "MapManger.h"
#include "AgvManager.h"

void MsgProcess(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	//过滤未登录的消息[未登录，不能响应 除了登录以外的其他任何请求]
	if (conn->getId() <= 0 && msg.head.todo != CLIENT_MSG_TODO_USER_LOGIN)
	{
		//如果未登录，并且不是登录消息，那么回一句 请登录
		Client_Response_Msg response;
		memset(&response, 0, sizeof(Client_Response_Msg));
		memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
		response.head.body_length = 0;
		response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_NOT_LOGIN;
		conn->write_all(response);
		return;
	}

	typedef std::function<void(TcpConnection::Pointer, Client_Request_Msg)> ProcessFunction;

	UserManager::Pointer userManager = UserManager::getInstance();
	MapManger::Pointer mapManager = MapManger::getInstance();
	AgvManager::Pointer agvManager = AgvManager::getInstance();
	SessionManager::Pointer sessionManager = SessionManager::getInstance();

	static struct
	{
		CLIENT_MSG_TODO t;
		ProcessFunction f;
	} table[] =
	{
		{ CLIENT_MSG_TODO_USER_LOGIN,std::bind(&UserManager::interLogin,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_USER_LOGOUT,std::bind(&UserManager::interLogout,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_USER_CHANGED_PASSWORD,std::bind(&UserManager::interChangePassword,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_USER_LIST,std::bind(&UserManager::interList,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_USER_DELTE,std::bind(&UserManager::interRemove,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_USER_ADD,std::bind(&UserManager::interAdd,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_USER_MODIFY,std::bind(&UserManager::interModify,userManager,std::placeholders::_1,std::placeholders::_2) },

		{ CLIENT_MSG_TODO_MAP_CREATE_START,std::bind(&MapManger::interCreateStart,mapManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_MAP_CREATE_ADD_STATION,std::bind(&MapManger::interCreateAddStation,mapManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_MAP_CREATE_ADD_LINE,std::bind(&MapManger::interCreateAddLine,mapManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_MAP_CREATE_ADD_ARC,std::bind(&MapManger::interCreateAddArc,mapManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_MAP_CREATE_FINISH,std::bind(&MapManger::interCreateFinish,mapManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_MAP_LIST_STATION,std::bind(&MapManger::interListStation,mapManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_MAP_LIST_LINE,std::bind(&MapManger::interListLine,mapManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_MAP_LIST_ARC,std::bind(&MapManger::interListArc,mapManager,std::placeholders::_1,std::placeholders::_2) },

		{ CLIENT_MSG_TODO_HAND_REQUEST,std::bind(&AgvManager::interHandRequest,agvManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_HAND_RELEASE,std::bind(&AgvManager::interHandRelease,agvManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_HAND_FORWARD,std::bind(&AgvManager::interHandForward,agvManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_HAND_BACKWARD,std::bind(&AgvManager::interHandBackward,agvManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_HAND_TURN_LEFT,std::bind(&AgvManager::interHandTurnLeft,agvManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_HAND_TURN_RIGHT,std::bind(&AgvManager::interHandTurnRight,agvManager,std::placeholders::_1,std::placeholders::_2) },

		{ CLIENT_MSG_TODO_AGV_MANAGE_LIST,std::bind(&AgvManager::interList,agvManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_AGV_MANAGE_ADD,std::bind(&AgvManager::interAdd,agvManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_AGV_MANAGE_DELETE,std::bind(&AgvManager::interDelete,agvManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_AGV_MANAGE_MODIFY,std::bind(&AgvManager::interModify,agvManager,std::placeholders::_1,std::placeholders::_2) },

		{ CLIENT_MSG_TODO_TASK_CREATE_TO_X,std::bind(&UserManager::interLogin,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_CREATE_AGV_TO_X,std::bind(&UserManager::interLogin,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_CREATE_PASS_Y_TO_X,std::bind(&UserManager::interLogin,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_CREATE_AGV_PASS_Y_TO_X,std::bind(&UserManager::interLogin,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_QUERY_STATUS,std::bind(&UserManager::interLogin,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_CANCEL,std::bind(&UserManager::interLogin,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_LIST_UNDO,std::bind(&UserManager::interLogin,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_LIST_DOING,std::bind(&UserManager::interLogin,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_LIST_DONE_TODAY,std::bind(&UserManager::interLogin,userManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_TASK_LIST_DURING,std::bind(&UserManager::interLogin,userManager,std::placeholders::_1,std::placeholders::_2) },

		{ CLIENT_MSG_TODO_LOG_LIST_DURING,std::bind(&UserManager::interLogin,userManager,std::placeholders::_1,std::placeholders::_2) },

		//订阅类
		{ CLIENT_MSG_TODO_SUB_AGV_POSITION,std::bind(&SessionManager::addSubAgvPosition,sessionManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_CANCEL_SUB_AGV_POSITION,std::bind(&SessionManager::removeSubAgvPosition,sessionManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_SUB_AGV_STATSU,std::bind(&SessionManager::addSubAgvStatus,sessionManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_CANCEL_SUB_AGV_STATSU,std::bind(&SessionManager::removeSubAgvStatus,sessionManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_SUB_LOG,std::bind(&SessionManager::addSubLog,sessionManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_CANCEL_SUB_LOG,std::bind(&SessionManager::removeSubLog,sessionManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_SUB_TASK,std::bind(&SessionManager::addSubTask,sessionManager,std::placeholders::_1,std::placeholders::_2) },
		{ CLIENT_MSG_TODO_CANCEL_SUB_TASK,std::bind(&SessionManager::removeSubTask,sessionManager,std::placeholders::_1,std::placeholders::_2) }
	};
	table[msg.head.todo].f(conn, msg);
}