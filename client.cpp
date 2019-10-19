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

	TcpClient client;
	client.Start("127.0.0.1", 8001);
	
	system("pause");
	client.Stop();
	
	JwSocket::Clean();
	return 0;

}


