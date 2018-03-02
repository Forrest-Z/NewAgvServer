#pragma once

#include <cstdint>
////定义client到server的消息格式
#define	CLIENT_MSG_REQUEST_BODY_MAX_SIZE           (1024)  //一条消息的最大长度 

#define CLIENT_COMMON_HEAD_HEAD		0x88
#define CLIENT_COMMON_HEAD_TAIL		0xAA

//消息的head
typedef struct _Client_Common_Head
{
	int8_t head;//固定为0x88
	int32_t body_length;//body的长度 最大为TCP_MSG_MAX_SIZE
	int8_t todo;//要做的事情
	int32_t queuenumber;//消息序号，返回时要带上
	int8_t tail;//固定为0xAA
}Client_Common_Head;

//消息的body
typedef struct _Client_Common_Body
{
	char data[CLIENT_MSG_REQUEST_BODY_MAX_SIZE];
	int length;
}Client_Common_Body;

//!!!完整的请求消息
typedef struct _Client_Request_Msg {
	Client_Common_Head head;
	Client_Common_Body body;
	int id;//用于标记来自哪个conn
}Client_Request_Msg;

//返回消息的结构的额外头
typedef struct _CLIENT_RETURN_MSG_HEAD
{
	int8_t result;//CLIENT_RETURN_MSG_RESULT
	int32_t error_code;//CLIENT_RETURN_MSG_ERROR_CODE_
	char error_info[256];
}CLIENT_RETURN_MSG_HEAD;

//!!!完整的返回消息
typedef struct _Client_Response_Msg {
	Client_Common_Head head;
	CLIENT_RETURN_MSG_HEAD return_head;//用于判断返回结果
	Client_Common_Body body;
}Client_Response_Msg;
//////////////////////////下面定义具体的消息协议

//定义消息头的 todo
typedef enum Client_Msg_Todo
{
	CLIENT_MSG_TODO_USER_LOGIN = 0,//登录//username[64]+password[64]
	CLIENT_MSG_TODO_USER_LOGOUT,//登出//none
	CLIENT_MSG_TODO_USER_CHANGED_PASSWORD,//修改密码//newpassword[64]
	CLIENT_MSG_TODO_USER_LIST,//列表//none
	CLIENT_MSG_TODO_USER_DELTE,//删除用户//userid[32]
	CLIENT_MSG_TODO_USER_ADD,//添加用户//username[64] password[64] role[1]
	CLIENT_MSG_TODO_MAP_CREATE_START,//创建地图开始
	CLIENT_MSG_TODO_MAP_CREATE_ADD_STATION,//添加站点 station[id[4]+x[4]+y[4]+name[64]+rfid[4]+r[2]+g[2]+b[2]]{个} //如果超出1024长度，可以分成多条
	CLIENT_MSG_TODO_MAP_CREATE_ADD_LINE, //添加直线 line[id[4] + startstation[4] + endstation[64] + length[4] + draw[1] + r[2] + g[2] + b[2]]{个 }//如果超出1024长度，可以分成多条
	CLIENT_MSG_TODO_MAP_CREATE_ADD_ARC,//添加曲线 arc[id[4] + startstation[4] + endstation[64] +length[4] + draw[1] +r[2]+g[2]+b[2]+p1x[4]+p1y[4]+p2x[4]+p2y[4]]{个 }//如果超出1024长度，可以分成多条
	CLIENT_MSG_TODO_MAP_CREATE_FINISH,//创建地图完成
	CLIENT_MSG_TODO_MAP_LIST_STATION,//请求所有站点//none
	CLIENT_MSG_TODO_MAP_LIST_LINE,//请求所有直线//none
	CLIENT_MSG_TODO_MAP_LIST_ARC,//请求所有曲线//none
	CLIENT_MSG_TODO_HAND_REQUEST,//请求控制权//none
	CLIENT_MSG_TODO_HAND_RELEASE,//释放控制权//none
	CLIENT_MSG_TODO_HAND_FORWARD,//前进//none
	CLIENT_MSG_TODO_HAND_BACKWARD,//后退//none
	CLIENT_MSG_TODO_HAND_TURN_LEFT,//左转//none
	CLIENT_MSG_TODO_HAND_TURN_RIGHT,//右转//none
	CLIENT_MSG_TODO_AGV_MANAGE_LIST,//车辆列表//none
	CLIENT_MSG_TODO_AGV_MANAGE_ADD,//增加//name[64]+ip[64]
	CLIENT_MSG_TODO_AGV_MANAGE_DELETE,//删除//id[4]
	CLIENT_MSG_TODO_AGV_MANAGE_MODIFY,//修改//id[4]+name[64]+ip[64]
	CLIENT_MSG_TODO_TASK_CREATE_TO_X,//到X点位的任务//x[4]
	CLIENT_MSG_TODO_TASK_CREATE_AGV_TO_X,//制定Agv到X点位的任务//agvid[4]+x[4]
	CLIENT_MSG_TODO_TASK_CREATE_PASS_Y_TO_X,//去Y取货放到X的任务//x[4]+y[4]
	CLIENT_MSG_TODO_TASK_CREATE_AGV_PASS_Y_TO_X,//指定AGV到Y取货放到X的任务//agvid[4]+x[4]+y[4]
	CLIENT_MSG_TODO_TASK_QUERY_STATUS,//查询任务状态//taskid[4]
	CLIENT_MSG_TODO_TASK_CANCEL,//取消任务//taskid[4]
	CLIENT_MSG_TODO_TASK_LIST_UNDO,//未完成的列表//none
	CLIENT_MSG_TODO_TASK_LIST_DOING,//正在执行的任务列表//none
	CLIENT_MSG_TODO_TASK_LIST_DONE_TODAY,//今天完成的任务//none
	CLIENT_MSG_TODO_TASK_LIST_DURING,//历史完成的任务//时间格式格式yyyy-MM-dd hh-mm-ss；from_time[24] to_time[24]
	CLIENT_MSG_TODO_LOG_LIST_DURING,//查询历史日志//时间格式格式yyyy-MM-dd hh-mm-ss；from_time[24] to_time[24]
	CLIENT_MSG_TODO_SUB_AGV_POSITION,//订阅车辆位置信息
	CLIENT_MSG_TODO_CANCEL_SUB_AGV_POSITION,//取消车辆位置信息订阅
	CLIENT_MSG_TODO_SUB_AGV_STATSU,//订阅车辆状态信息
	CLIENT_MSG_TODO_CANCEL_SUB_AGV_STATSU,//取消车辆状态信息订阅
	CLIENT_MSG_TODO_SUB_LOG,//订阅日志
	CLIENT_MSG_TODO_CANCEL_SUB_LOG,//取消日志订阅
	CLIENT_MSG_TODO_SUB_TASK,//任务订阅
	CLIENT_MSG_TODO_CANCEL_SUB_TASK,//取消任务订阅
}CLIENT_MSG_TODO;

//定义消息头的 todo//---------------------------------------------------------------------------------------------------------------------------------

//result位的定义
enum
{
	CLIENT_RETURN_MSG_RESULT_SUCCESS = 0,//全局的成功
	CLIENT_RETURN_MSG_RESULT_FAIL,//全局的错误
};

//error_code位的定义
enum {
	CLIENT_RETURN_MSG_ERROR_NO_ERROR = 0,
	CLIENT_RETURN_MSG_ERROR_UNKNOW,//未知错误
	CLIENT_RETURN_MSG_ERROR_LENGTH,//数据长度有问题
	CLIENT_RETURN_MSG_ERROR_CODE_USERNAME_NOT_EXIST,//登陆用户名不存在
	CLIENT_RETURN_MSG_ERROR_CODE_PASSWORD_ERROR,//登陆密码错误
	CLIENT_RETURN_MSG_ERROR_CODE_NOT_LOGIN,//用户未登录
	CLIENT_RETURN_MSG_ERROR_CODE_SAVE_SQL_FAIL,//保存数据库失败
};

////////////////////////////////////////以下是特殊情况的返回结构体

