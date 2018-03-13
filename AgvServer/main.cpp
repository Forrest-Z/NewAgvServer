
#include "Server.h"
#include "DBManager.h"
#include <string>
#include <iostream>

int main()
{
	//初始化数据库连接
	DBManager::GetInstance()->createConnection();
	//初始化连接服务
	Server::GetInstance()->initListen();
    return 0;
}

