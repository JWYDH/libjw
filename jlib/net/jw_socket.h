#pragma once
#ifdef WIN32
#include <WinSock.h>
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include "errno.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
typedef int	SOCKET;
//#pragma region define win32 const variable in linux
#define INVALID_SOCKET	-1
#define SOCKET_ERROR	-1
//#pragma endregion
#endif





class JwSocket {

public:
	JwSocket();
	~JwSocket();

	operator SOCKET ();
	JwSocket& operator = (SOCKET s) {
		socket_fd_ = s;
		return (*this);
	}
	inline void getLoaclAddr(sockaddr_in* addr_in)
	{
		*addr_in = local_addr_;
	}
	//��ȡԶ�̵ĵ�ַ�Ͷ˿ڵ�sockaddr_in�ṹ
	inline void getRemoteAddr(sockaddr_in* addr_in)
	{
		*addr_in = remote_addr_;
	}
	
	inline void setLoaclAddr(const sockaddr_in* addr_in)
	{
		local_addr_ = *addr_in;
	}
	//����Զ�̵�ַ��Ϣ
	inline void setRemoteAddr(const sockaddr_in* addr_in)
	{
		remote_addr_ = *addr_in;
	}
	//============================================
	bool SetNoBlock(bool enable = true);
	int GetRecvBufSize(int* size);
	bool SetRecvBufSize(int size);
	int GetSendBufSize(int* size);
	bool SetSendBufSize(int size);

	bool SetNoDelay();

	bool Create(int af = AF_INET, int type = SOCK_STREAM, int protocol = 0);

	bool Connect(const char* ip, unsigned short port);

	bool Bind(const char *ip, unsigned short port);

	bool Listen(int backlog = 5); 


	bool Accept(JwSocket& socket);

	int Send(const char* buf, int len, int flags = 0);


	int Recv(char* buf, int len, int flags = 0);

	void Close();


	
	static int Init();	

	static int Clean();

	static bool DNSParse(const char* domain, char* ip);
protected:
	SOCKET socket_fd_;
	sockaddr_in	local_addr_;	//�󶨵ı��ص�ַ
	sockaddr_in	remote_addr_;	//Զ�̵�ַ
};

