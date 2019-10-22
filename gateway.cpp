// gateway.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>

#include "depend/jw_tcpconn.h"
#include "depend/jw_tcpserver.h"
#include "depend/jw_tcpclient.h"
#include "depend/3rd//http_parser.h"
#include "depend/3rd/HttpFormat.h"
#include "depend/3rd/HttpParser.h"
#include "depend/3rd/WebSocketFormat.h"

#define DEFAULT_TAG 0xA73754B0
TcpClient client;
class MyClass
{
public:
	MyClass();
	~MyClass();

	void OnConnected() {}
	void OnDisconnected() {}
	int32_t OnRead(TcpConn* conn, const char* buf, int32_t len) {
		
		if (!upgrade_){
			int32_t retlen = len;
			auto httpParser = std::make_shared<HTTPParser>(HTTP_REQUEST);
			if (!httpParser->isCompleted())
			{
				retlen = httpParser->tryParse(buf, len);
				if (!httpParser->isCompleted())
				{
					return retlen;
				}
			}
			if (httpParser->isWebSocket() && httpParser->hasKey("Sec-WebSocket-Key"))
			{
				
				{
					auto response = WebSocketFormat::wsHandshake(httpParser->getValue("Sec-WebSocket-Key"));
					conn->DoWrite(response.c_str(), response.size());
					upgrade_ = true;
				}
			}
			else
			{
				//if (httpParser->isKeepAlive())
				{
					httpParser->clearParse();
				}
			}
			return retlen;
		}
		else {
			std::string str;
			WebSocketFormat::WebSocketFrameType type;
			size_t frameSize;
			bool fin;
			if (WebSocketFormat::wsFrameExtractBuffer(buf, len, str, type, frameSize, fin))
			{
				printf(str.c_str());
				if (conn_status_) {
					client.DoWrite(str.c_str(), str.size());
				//	if (ws_head_.opcode == OPCODE_CLR) {
				//		Close();
				//		return false;
				//	}
				//	if (size < 4) {
				//		MSG_ERR("size is min");
				//		return true;
				//	}
				//	uint32_t tag = *((uint32_t*)buf);
				//	if (tag != skey_) {
				//		Close();
				//		return false;
				//	}
				//	//发送给游戏服
				//	static GameWorldClient* wgc = cli_mgr_->GetGameWorldClient();
				//	DataPacket& pack = wgc->allocProtoPacket(GW_DATA);
				//	pack << net_id_;
				//	uint8_t nsize = sizeof(uint32_t);
				//	pack.writeBuf(buf + nsize, size - nsize);
				//	wgc->flushProtoPacket(pack);
				//	return true;
				}

				if (!conn_status_) { //刚连接上来
					//if (frameSize < 4)
					//{
					//	conn->Close();
					//	return 0;
					//}
					//uint32_t tag = *((uint32_t*)str[1]);
					//if (tag != DEFAULT_TAG) {
					//	conn->Close();
					//	return 0;
					//}
					skey_ = rand();
					Buffer s;
					std::string frame;
					s.WriteBuff((char*)&skey_, sizeof(skey_));
					WebSocketFormat::wsFrameBuild(s.OffsetPtr(), s.AvaliableLength(),
						frame,
						WebSocketFormat::WebSocketFrameType::BINARY_FRAME,
						true,
						false);
					conn->DoWrite(frame.c_str(), frame.size());
					conn_status_ = true;
				}
				return frameSize;
			}
		}
		return 0;
	}

private:
	bool upgrade_;
	bool conn_status_;
	uint32_t skey_;//服务端KEY
	
};

MyClass::MyClass()
:upgrade_(false) , conn_status_(false){
	
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
		conn->OnRead([m](TcpConn* conn, char* buf, int32_t len)->int32_t {
			int32_t r = m->OnRead(conn, buf, len);
			return r;
		});
	});
	server.Start("127.0.0.1", 8001);
	//server.Start("192.168.1.7", 8001);
	
	client.Start("127.0.0.1", 8002);
	system("pause");
	server.Stop();
	client.Stop();
	JwSocket::Clean();
	return 0;

}


