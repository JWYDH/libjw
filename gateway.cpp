// gateway.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>

#include "depend/jw_tcpconn.h"
#include "depend/jw_tcpserver.h"
#include "depend/3rd/WebSocketFormat.h"

class MyClass
{
public:
	MyClass();
	~MyClass();

	void OnConnected() {}
	void OnDisconnected() {}
	void OnRead(const char* buf, int32_t len) {
		//size_t retlen = 0;
		//if (websocket_)
		//{
		//	WebSocketFormat::wsHandshake()
		//}
		//if (->isWebSocket())
		//{
		//	retlen = HttpService::ProcessWebSocket(buffer, len, httpParser, httpSession);
		//}
		//else
		//{
		//	retlen = HttpService::ProcessHttp(buffer, len, httpParser, httpSession);
		//}

		//return retlen;
	}
private:
	bool websocket_ = false;
};

MyClass::MyClass()
{
}

MyClass::~MyClass()
{
}

int main()
{
	JwSocket::Init();
	
#ifndef _MSC_VER		
	runing = true;
	//signal(SIGTERM, sigterm_handler);
	signal(SIGPIPE, SIG_IGN);
	//signal(SIGHUP, sighup_handler);
#endif

	TcpServer server;
	server.OnNewSession([](TcpConn* conn) {
		MyClass *m = new MyClass;
		//conn->OnConnected([&m](TcpConn* conn) {
		//	m->OnConnected();
		//});

		//conn->OnDisconnected([&m](TcpConn* conn) {
		//	m->OnDisconnected();
		//});

		conn->OnRead([&m](TcpConn* conn, char* buf, int32_t len)->bool {
			m->OnRead(buf, len);
			return true;
		});
	});
	server.Start("127.0.0.1", 8001);
	
	system("pause");
	server.Stop();
	
	JwSocket::Clean();
	return 0;

}
