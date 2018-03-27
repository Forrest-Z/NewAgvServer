
#include "Server.h"
#include "DBManager.h"
#include <string>
#include <iostream>

int main()
{
	//初始化数据库连接
	DBManager::getInstance()->createConnection();
	//初始化服务
	Server::getInstance()->initAll();
    return 0;
}

