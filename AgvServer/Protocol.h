#pragma once

#include <cstdint>
////定义client到server的消息格式
#define	TCP_ONE_MSG_MAX_SIZE           (1024)  //一条消息的最大长度 

#define TCP_ONE_MSG_HEAD_HEAD		0x88
#define TCP_ONE_MSG_HEAD_TAIL		0xAA

//head
typedef struct _Client_Msg_Head
{
	int8_t head;//固定为0x88
	int8_t complete;//标记这个是否是一个完整的包
	int32_t body_length;//body的长度 最大为TCP_MSG_MAX_SIZE
	int8_t type;//要做的事情的大类
	int8_t todo;//要做的事情大类下面的小类
	int32_t queuenumber;//消息序号，返回时要带上
	int8_t tail;//固定为0xAA
}Client_Msg_Head;

//body
typedef struct _Client_Msg_Body
{
	char data[TCP_ONE_MSG_MAX_SIZE];
	int length;
}Client_Msg_Body;

//一个完整的消息
typedef struct _Client_Msg {
	Client_Msg_Head header;
	Client_Msg_Body body;
	int user_id;
}Client_Msg;

//////////////////////////下面定义具体的消息协议
//分包处理：如果是一个完整的包，ALL
//如果需要分两个包：HEAD + TAIL
//如果需要分三个包：HEAD + BODY + TAIL
//如果要分三个以上包: HEAD + BODY + ... + BODY + TAIL
enum Client_Msg_Complete
{
	CLIENT_MSG_COMPLTE_ALL = 0,//一个完整的包
	CLIENT_MSG_COMPLETE_HEAD,//头部
	CLIENT_MSG_COMPLTE_BODY,//中间部分
	CLIENT_MSG_COMPLTE_TAIL//结尾部分
};

enum Client_Msg_Type
{
	CLIENT_MSG_TYPE_USER = 0,//用户操作
	CLIENT_MSG_TYPE_MAP,//地图操作
	CLIENT_MSG_TYPE_HAND,//手动控制
	CLIENT_MSG_TYPE_AGV_MANAGE,//AGV管理
	CLIENT_MSG_TYPE_TASK,//任务管理
	CLIENT_MSG_TYPE_LOG//日志
};

enum Client_Msg_Todo_User
{
	CLIENT_MSG_TODO_USER_LOGIN = 0,//登录//username[64]+password[64]
	CLIENT_MSG_TODO_USER_LOGOUT,//登出//none
	CLIENT_MSG_TODO_USER_CHANGED_PASSWORD,//修改密码//newpassword[64]
	CLIENT_MSG_TODO_USER_LIST,//列表//none
	CLIENT_MSG_TODO_USER_DELTE,//删除用户//userid
	CLIENT_MSG_TODO_USER_ADD,//添加用户//username[64] password[64] role[1]
};

enum Client_Msg_Todo_Map
{
	CLIENT_MSG_TODO_MAP_CREATE = 0,//创建地图
								   //station_amount[4]+line_amount[4]+arc_amount[4]
								   //+station[id[4]+x[4]+y[4]+name[64]+rfid[4]+r[2]+g[2]+b[2]]{station_amount个}  
								   //+line[id[4] + startstation[4] + endstation[64] +length[4] + draw[1] +r[2]+g[2]+b[2]]{ line_amount个 }
								   //+arc[id[4] + startstation[4] + endstation[64] +length[4] + draw[1] +r[2]+g[2]+b[2]+p1x[4]+p1y[4]+p2x[4]+p2y[4]]{ arc_amount个 }
	CLIENT_MSG_TODO_MAP_SET_IMAGE,//设置地图背景//length[8] data[length转成long]//如果length==0，那么清空背景
	CLIENT_MSG_TODO_MAP_LIST_STATION,//请求所有站点//none
	CLIENT_MSG_TODO_MAP_LIST_LINE,//请求所有直线//none
	CLIENT_MSG_TODO_MAP_LIST_ARC,//请求所有曲线//none
	CLIENT_MSG_TODO_MAP_SUB_AGV_POSITION,//订阅AGV的位置广播//none
	CLIENT_MSG_TODO_MAP_CANCEL_SUB_AGV_POSITION,//取消AGV的位置广播//none
};

enum Client_Msg_Todo_Hand
{
	CLIENT_MSG_TODO_HAND_REQUEST = 0,//请求控制权//none
	CLIENT_MSG_TODO_HAND_RELEASE,//释放控制权//none
	CLIENT_MSG_TODO_HAND_FORWARD,//前进//none
	CLIENT_MSG_TODO_HAND_BACKWARD,//后退//none
	CLIENT_MSG_TODO_HAND_TURN_LEFT,//左转//none
	CLIENT_MSG_TODO_HAND_TURN_RIGHT,//右转//none
	CLIENT_MSG_TODO_HAND_LIGHT,//灯光//[1]
	CLIENT_MSG_TODO_HAND_SUB_AGV_STATUS,//订阅状态//none
	CLIENT_MSG_TODO_HAND_CANCEL_SUB_AGV_STATUS,//取消订阅//none
};

enum Client_Msg_Todo_Agv_Manage
{
	CLIENT_MSG_TODO_AGV_MANAGE_LIST = 0,//车辆列表//none
	CLIENT_MSG_TODO_AGV_MANAGE_ADD,//增加//name[64]+ip[64]
	CLIENT_MSG_TODO_AGV_MANAGE_DELETE,//删除//id[4]
	CLIENT_MSG_TODO_AGV_MANAGE_MODIFY//修改//id[4]+name[64]+ip[64]
};

enum Client_Msg_Todo_Task
{
	CLIENT_MSG_TODO_TASK_CREATE_TO_X = 0,//到X点位的任务//x[4]
	CLIENT_MSG_TODO_TASK_CREATE_AGV_TO_X,//制定Agv到X点位的任务//agvid[4]+x[4]
	CLIENT_MSG_TODO_TASK_CREATE_PASS_Y_TO_X,//去Y取货放到X的任务//x[4]+y[4]
	CLIENT_MSG_TODO_TASK_CREATE_AGV_PASS_Y_TO_X,//指定AGV到Y取货放到X的任务//agvid[4]+x[4]+y[4]
	CLIENT_MSG_TODO_TASK_QUERY_STATUS,//查询任务状态//taskid[4]
	CLIENT_MSG_TODO_TASK_CANCEL,//取消任务//taskid[4]
	CLIENT_MSG_TODO_TASK_LIST_UNDO,//未完成的列表//none
	CLIENT_MSG_TODO_TASK_LIST_DOING,//正在执行的任务列表//none
	CLIENT_MSG_TODO_TASK_LIST_DONE_TODAY,//今天完成的任务//none
	CLIENT_MSG_TODO_TASK_LIST_DURING,//历史完成的任务//时间格式格式yyyy-MM-dd hh-mm-ss；from_time[24] to_time[24]
};

enum Client_Msg_Todo_Log
{
	CLIENT_MSG_TODO_LOG_LIST_DURING = 0,//查询历史日志//时间格式格式yyyy-MM-dd hh-mm-ss；from_time[24] to_time[24]
	CLIENT_MSG_TODO_LOG_SUB,//订阅日志广播//none
	CLIENT_MSG_TODO_LOG_CANCEL_SUB,//取消日志广播//none
};