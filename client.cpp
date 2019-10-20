// gateway.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>

#include "depend/jw_tcpclient.h"
#include "depend/3rd//http_parser.h"
#include "depend/3rd/HttpFormat.h"
#include "depend/3rd/HttpParser.h"
#include "depend/3rd/WebSocketFormat.h"



int main()
{
	JwSocket::Init();
	
#ifndef _MSC_VER		
	runing = true;
	//signal(SIGTERM, sigterm_handler);
	signal(SIGPIPE, SIG_IGN);
	//signal(SIGHUP, sighup_handler);
#endif

	const int x = 1;
	TcpClient* client[x];
	for (size_t i = 0; i < x; i++)
	{
		client[i] = new TcpClient;
		client[i]->Start("127.0.0.1", 8001);
	}
	system("pause");
	for (size_t i = 0; i < x; i++)
	{
		client[i]->Stop();
	}
	
	
	JwSocket::Clean();
	return 0;

}


