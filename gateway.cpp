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
#include "depend/3rd/Encrypt.h"

#define SMALL_HEAD
#define TAG_VALUE 0xCCEE		   //默认的包的开头的2个字节

#pragma pack(push,4)
typedef struct
{
	uint16_t tag;
	uint16_t len;
	struct
	{
		uint16_t dataSum;
		uint16_t hdrSum;
	} EncodePart;
#ifndef SMALL_HEAD
	union
	{
		struct
		{
			int recvTime;	// 网关接收消耗时间
			int sendTime;	// 网关发送消耗时间
		}s;
		int64_t reserve; // 时间点，用于网关接受到客户端数据后打上的时间戳。
	}u;
#endif
}DATAHEADER;


typedef struct
{
	uint64_t socket_;
	uint16_t index_;
	uint16_t gate_id_;
}NetId;

#pragma pack(pop)

#define GW_OPEN 1
#define GW_CLOSE 2
#define GW_DATA 5

TcpClient client;
class MyClass
{
public:
	MyClass();
	~MyClass();

	void OnConnected() {}
	void OnDisconnected(TcpConn* conn) {
		SendSessionClose(conn);
	}
	void SendNewSession(TcpConn* conn) {
		//static GameWorldClient* wgc = cli_mgr_->GetGameWorldClient();
		//DataPacket &dp = wgc->allocProtoPacket(GW_OPEN);
		//dp << net_id_;
		//dp.writeBuf(inet_ntoa(remote_addr_.sin_addr), 32);
		//wgc->flushProtoPacket(dp);
#pragma pack(push,4)
		struct CmdHdr
		{
			DATAHEADER hdr;	// 这个也是4字节对齐
			uint16_t cmd;
			uint16_t reserve;	//多余的两个字节，为了字节对齐
		};
#pragma pack(pop)
		//STATIC_ASSERT(sizeof(CmdHdr) == sizeof(DATAHEADER) + 4);

		//DataPacket& packet = AllocSendPack();

		//packet.setLength(sizeof(CmdHdr));
		Buffer s;
		s.SetLength(sizeof(CmdHdr));
		CmdHdr* pack_hdr = (CmdHdr*)s.MemPtr();
		pack_hdr->hdr.tag = TAG_VALUE;
		pack_hdr->cmd = GW_OPEN;

		const int pos = sizeof(CmdHdr) - sizeof(pack_hdr->reserve);
		//packet.setPosition(pos);
		s.Seek(pos);

		//return packet;
		//	pack << net_id_;
		uint64_t t_socket = 1;
		uint16_t t_index = 1;
		uint16_t t_gate_id = 1;
		s.WriteBuff((char*)&t_socket, sizeof(t_socket));
		s.WriteBuff((char*)&t_index, sizeof(t_index));
		s.WriteBuff((char*)&t_gate_id, sizeof(t_gate_id));
		
		
		sockaddr_in remote_addr;
		conn->GetSocket().getRemoteAddr(&remote_addr);
		

		s.WriteBuff(inet_ntoa(remote_addr.sin_addr), 32);
		//	pack.writeBuf(buf + nsize, size - nsize);
		//	wgc->flushProtoPacket(pack);
		client.DoWrite(s.MemPtr(), s.Length());
	}

	void SendSessionClose(TcpConn* conn) {
		//static GameWorldClient* wgc = cli_mgr_->GetGameWorldClient();
		//DataPacket &dp = wgc->allocProtoPacket(GW_CLOSE);
		//dp << net_id_;
		//wgc->flushProtoPacket(dp);
#pragma pack(push,4)
		struct CmdHdr
		{
			DATAHEADER hdr;	// 这个也是4字节对齐
			uint16_t cmd;
			uint16_t reserve;	//多余的两个字节，为了字节对齐
		};
#pragma pack(pop)
		//STATIC_ASSERT(sizeof(CmdHdr) == sizeof(DATAHEADER) + 4);

		//DataPacket& packet = AllocSendPack();

		//packet.setLength(sizeof(CmdHdr));
		Buffer s;
		s.SetLength(sizeof(CmdHdr));
		CmdHdr* pack_hdr = (CmdHdr*)s.MemPtr();
		pack_hdr->hdr.tag = TAG_VALUE;
		pack_hdr->cmd = GW_CLOSE;

		const int pos = sizeof(CmdHdr) - sizeof(pack_hdr->reserve);
		//packet.setPosition(pos);
		s.Seek(pos);

		//return packet;
		//	pack << net_id_;
		uint64_t t_socket = 1;
		uint16_t t_index = 1;
		uint16_t t_gate_id = 1;
		s.WriteBuff((char*)&t_socket, sizeof(t_socket));
		s.WriteBuff((char*)&t_index, sizeof(t_index));
		s.WriteBuff((char*)&t_gate_id, sizeof(t_gate_id));
		//	pack.writeBuf(buf + nsize, size - nsize);
		//	wgc->flushProtoPacket(pack);
		client.DoWrite(s.MemPtr(), s.Length());
	}
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
				SendNewSession(conn);
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
				if (conn_status_ == 0) { //刚连接上来
					if (str.size() < 4)
					{
						conn->Close();
						return 0;
					}
					tagsalt_ = *((uint32_t*)str[1]);
					encrypt_.SetTargetSalt(tagsalt_);
					mysalt_ = encrypt_.GetSelfSalt();
					skey_ = encrypt_.GetKeyCRC();

					Buffer s;
					std::string frame;
					s.WriteBuff((char*)&mysalt_, sizeof(mysalt_));
					WebSocketFormat::wsFrameBuild(s.OffsetPtr(), s.AvaliableLength(),
						frame,
						WebSocketFormat::WebSocketFrameType::BINARY_FRAME,
						true,
						false);
					conn->DoWrite(frame.c_str(), frame.size());
					conn_status_ = 1;
				}
				else if (conn_status_ == 1) {
					
					if (type == WebSocketFormat::WebSocketFrameType::CLOSE_FRAME) {
						conn->Close();
						return 0;
					}
					if (str.size() < 2)
					{
						conn->Close();
						return 0;
					}
					uint16_t key = *((uint16_t*)str[1]);
					
					if (key != skey_) {
						conn->Close();
						return 0;
					}
					
				//	
				//	//发送给游戏服
				//	static GameWorldClient* wgc = cli_mgr_->GetGameWorldClient();
				//	DataPacket& pack = wgc->allocProtoPacket(GW_DATA);
					
#pragma pack(push,4)
					struct CmdHdr
					{
						DATAHEADER hdr;	// 这个也是4字节对齐
						uint16_t cmd;
						uint16_t reserve;	//多余的两个字节，为了字节对齐
					};
#pragma pack(pop)
					//STATIC_ASSERT(sizeof(CmdHdr) == sizeof(DATAHEADER) + 4);

					//DataPacket& packet = AllocSendPack();

					//packet.setLength(sizeof(CmdHdr));
					Buffer s;
					s.SetLength(sizeof(CmdHdr));
					CmdHdr* pack_hdr = (CmdHdr*)s.MemPtr();
					pack_hdr->hdr.tag = TAG_VALUE;
					pack_hdr->cmd = GW_DATA;

					const int pos = sizeof(CmdHdr) - sizeof(pack_hdr->reserve);
					//packet.setPosition(pos);
					s.Seek(pos);

					//return packet;
				//	pack << net_id_;
					uint64_t t_socket = 1;
					uint16_t t_index = 1;
					uint16_t t_gate_id = 1;
					s.WriteBuff((char*)&t_socket, sizeof(t_socket));
					s.WriteBuff((char*)&t_index, sizeof(t_index));
					s.WriteBuff((char*)&t_gate_id, sizeof(t_gate_id));
					uint8_t nsize = sizeof(uint16_t);
					s.WriteBuff((char*)str.c_str()+nsize, str.size()-nsize);
				//	pack.writeBuf(buf + nsize, size - nsize);
				//	wgc->flushProtoPacket(pack);
					client.DoWrite(s.MemPtr(), s.Length());
				
				}


				return frameSize;
			}
		}
		return 0;
	}

private:
	bool upgrade_;
	bool conn_status_;

	Encrypt encrypt_;

	uint32_t skey_;//服务端KEY
	uint32_t tagsalt_;
	uint32_t mysalt_;
	
};

MyClass::MyClass()
:upgrade_(false) , conn_status_(0){
	
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

		conn->OnDisconnected([&m](TcpConn* conn) {
			m->OnDisconnected(conn);
		});
		conn->OnRead([m](TcpConn* conn, char* buf, int32_t len)->int32_t {
			int32_t r = m->OnRead(conn, buf, len);
			return r;
		});
	});
	server.Start("127.0.0.1", 9999);
	//server.Start("192.168.1.7", 8001);
	
	client.Start("127.0.0.1", 6999);
	system("pause");
	server.Stop();
	client.Stop();
	JwSocket::Clean();
	return 0;

}


