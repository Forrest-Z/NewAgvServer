
#include "Server.h"
#include "DBManager.h"
#include <string>
#include <iostream>

int main()
{
	DBManager::GetInstance()->createConnection();
	Server::GetInstance()->initListen();
    return 0;
}

