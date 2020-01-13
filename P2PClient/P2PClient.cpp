 /* P2P ����ͻ���
 * 
 * �ļ�����P2PClient.c
 *
 * ���ڣ�2004-5-21
 *
 * ���ߣ�shootingstars(zhouhuis22@sina.com)
 *
 */

/*  Դ���޸�����:
 *
 *	����޸ĵĴ���������Ϊ�˽�ʡʱ����ԭ����(shootingstars)�Ĵ���Ļ�������
 *  �Ĳ����õġ�ԭ����İ�Ȩ��ԭ�������С����������޸ĵĲ��ֱ��˲����κε�����
 *  ֻ��Ϊ�˷�����ʹ���޸ĺ�Ĵ������UDP��͸������֤���ѡ�
 *
 *  ԭ���ߵĹ���UDP��͸�ļ�������Ͻ����һ�ݶ���UDP��͸������˵�������ǲ�
 *  �����ر���������ϸ�������ڴ���P2P���򿪷��Ĺ�����, ����������͸���˺�
 *  �õ�ʵ�֡����ھ������������ⷽ������⣬��˾������Լ�����ʹ��UDP����
 *  ��͸�ľ���Ҳд�������ҷ��� ϣ�����ڴ����ⷽ�湤����������������.
 *  
 *  Դ���޸��߼��:
 *
 *  Hwycheng Leo, like C/C++/Python/Perl, hate Java. 
 *  һֱʹ��C/C++���Դ�������������Ϳͻ��˷���Ŀ���������������������P2P��������
 *  ��������ȫ���ǿ���BitTorrent������� - FlashBT(��̬�쳵). �����ڹ�˾���𿪷�
 *  P2P + IM ����ҵ��ƽ̨��
 *  
 *  �ʼ�/MSN: FlashBT@Hotmail.com           [��ӭ�Ȱ������̺�P2P���������Ž���]
 *  �����ҳ: http://www.hwysoft.com/chs/   [FlashBT(��̬�쳵)�Ĺٷ���ҳ]
 *  ����Blog: http://hwycheng.blogchina.com [��Ҫ���Լ�д��һЩ�������º��������]
 */

#ifndef LINUX
	#pragma comment(lib, "ws2_32.lib")
	#include "windows.h"
	typedef int socklen_t;
	typedef int ssize_t;
#else
	typedef int SOCKET;
	typedef int USHORT;
	typedef int BOOL;
	#define SOCKET_ERROR (-1)
	#define TRUE 1
	#define FALSE 0
	extern "C"{
		#include <stdio.h>
		#include <stdlib.h>
		#include <sys/types.h>
		#include <sys/socket.h>
		#include <netinet/in.h>
		#include <arpa/inet.h>
		#include <string.h>
		#include <unistd.h>
		#include <pthread.h>
	}
	void Sleep(int milli_sec)
	{
		usleep(milli_sec * 1000);
	}
	void closesocket(SOCKET s)
	{
		close(s);
	}
#endif

#include "../proto.h"
#include "../Exception.h"
#include <iostream>
#include <thread>
#include <functional>

USHORT g_nClientPort = 9896;
USHORT g_nServerPort = SERVER_PORT;

UserList ClientList;

#define COMMANDMAXC 256
#define MAXRETRY    1

SOCKET PrimaryUDP;
char UserName[10];
char ServerIP[20];

bool RecvedACK;

void InitWinSock()
{
#ifndef LINUX
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Windows sockets 2.2 startup");
		throw Exception("");
	}
	else{
		printf("Using %s (Status: %s)\n",
			wsaData.szDescription, wsaData.szSystemStatus);
		printf("with API versions %d.%d to %d.%d\n\n",
			LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion),
			LOBYTE(wsaData.wHighVersion), HIBYTE(wsaData.wHighVersion));
	}
#endif
}

SOCKET mksock(int type)
{
	SOCKET sock = socket(AF_INET, type, 0);
	if (sock < 0)
	{
        printf("create socket error");
		throw Exception("");
	}
	return sock;
}

stUserListNode GetUser(char *username)
{
	for(UserList::iterator UserIterator=ClientList.begin();
						UserIterator!=ClientList.end();
							++UserIterator)
	{
		if( strcmp( ((*UserIterator)->userName), username) == 0 )
			return *(*UserIterator);
	}
	throw Exception("not find this user");
}

void BindSock(SOCKET sock)
{
	sockaddr_in sin;
	memset(&sin, sizeof(sin), 0);
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(g_nClientPort);
	
	if (bind(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0)
		throw Exception("bind error");
}

void ConnectToServer(SOCKET sock,char *username, char *serverip)
{
	sockaddr_in remote;
	memset(&remote, sizeof(remote), 0);
	remote.sin_addr.s_addr = inet_addr(serverip);
	remote.sin_family = AF_INET;
	remote.sin_port = htons(g_nServerPort);
	
	stMessage sendbuf;
	sendbuf.iMessageType = LOGIN;
	strncpy(sendbuf.message.loginmember.userName, username, 10);

	sendto(sock, (const char*)&sendbuf, sizeof(sendbuf), 0, (const sockaddr*)&remote,sizeof(remote));
	int usercount;
	socklen_t fromlen = sizeof(remote);

    for ( ;; )
    {
        fd_set readfds;
        fd_set writefds;

        FD_ZERO( &readfds );
        FD_ZERO( &writefds );

        FD_SET( sock, &readfds );
        int maxfd = sock;

        timeval to;
        to.tv_sec = 2;
        to.tv_usec = 0;

        int n = select( maxfd + 1, &readfds, &writefds, NULL, &to );

        if ( n > 0 )
        {
            if ( FD_ISSET( sock, &readfds ) )
			{
				int iread = recvfrom(sock, (char *)&usercount, sizeof(int), 0, (sockaddr *)&remote, &fromlen);
				if(iread<=0)
				{
					throw Exception("Login error\n");
				}

				break;
			}
        }
        else if ( n < 0 )
        {
			throw Exception("Login error\n");
        }

		sendto(sock, (const char*)&sendbuf, sizeof(sendbuf), 0, (const sockaddr*)&remote,sizeof(remote));
    }

	// ��¼������˺󣬽��շ���˷������Ѿ���¼���û�����Ϣ
	std::cout<<"Have "<<usercount<<" users logined server:"<<std::endl;
	for(int i = 0;i<usercount;i++)
	{
		stUserListNode *node = new stUserListNode;
		recvfrom(sock, (char*)node, sizeof(stUserListNode), 0, (sockaddr *)&remote, &fromlen);
		ClientList.push_back(node);
		std::cout<<"Username:"<<node->userName<<std::endl;
		in_addr tmp;
		tmp.s_addr = htonl(node->ip);
		std::cout<<"UserIP:"<<inet_ntoa(tmp)<<std::endl;
		std::cout<<"UserPort:"<<node->port<<std::endl;
		std::cout<<""<<std::endl;
	}
}

void OutputUsage()
{
	std::cout<<"You can input you command:\n"
		<<"Command Type:\"send\",\"tell\", \"exit\",\"getu\"\n"
		<<"Example : send Username Message\n"
		<<"Example : tell Username ip:port Message\n"
		<<"          exit\n"
		<<"          getu\n"
		<<std::endl;
}

/* ������Ҫ�ĺ���������һ����Ϣ��ĳ���û�(C)
 *���̣�ֱ����ĳ���û�������IP������Ϣ�������ǰû����ϵ��
 *      ��ô����Ϣ���޷����ͣ����Ͷ˵ȴ���ʱ��
 *      ��ʱ�󣬷��Ͷ˽�����һ��������Ϣ������ˣ�
 *      Ҫ�����˷��͸��ͻ�Cһ����������C���������ʹ���Ϣ
 *      �������̽��ظ�MAXRETRY��
 */
bool SendMessageTo(char *UserName, char *Message)
{
	char realmessage[256];
	unsigned int UserIP;
	unsigned short UserPort;
	bool FindUser = false;
	for(UserList::iterator UserIterator=ClientList.begin();
						UserIterator!=ClientList.end();
						++UserIterator)
	{
		if( strcmp( ((*UserIterator)->userName), UserName) == 0 )
		{
			UserIP = (*UserIterator)->ip;
			UserPort = (*UserIterator)->port;
			FindUser = true;
		}
	}

	if(!FindUser)
		return false;

	strcpy(realmessage, Message);
	for(int i=0;i<MAXRETRY;i++)
	{
		RecvedACK = false;

		sockaddr_in remote;
		memset(&remote, sizeof(remote), 0);
		remote.sin_addr.s_addr = htonl(UserIP);
		remote.sin_family = AF_INET;
		remote.sin_port = htons(UserPort);
		stP2PMessage MessageHead;
		MessageHead.iMessageType = P2PMESSAGE;
		MessageHead.iStringLen = (int)strlen(realmessage)+1;
		int isend = sendto(PrimaryUDP, (const char *)&MessageHead, sizeof(MessageHead), 0, (const sockaddr*)&remote, sizeof(remote));
		isend = sendto(PrimaryUDP, (const char *)&realmessage, MessageHead.iStringLen, 0, (const sockaddr*)&remote, sizeof(remote));
		printf("send msg to %s %s %d\n", UserName, inet_ntoa(remote.sin_addr), UserPort);
		// �ȴ������߳̽��˱���޸�
		for(int j=0;j<10;j++)
		{
			if(RecvedACK)
				return true;
			else
				Sleep(300);
		}
		printf("send msg to %s %s %d, timeout\n", UserName, inet_ntoa(remote.sin_addr), UserPort);
		// û�н��յ�Ŀ�������Ļ�Ӧ����ΪĿ�������Ķ˿�ӳ��û��
		// �򿪣���ô����������Ϣ����������Ҫ����������Ŀ������
		// ��ӳ��˿ڣ�UDP�򶴣�
		sockaddr_in server;
		memset(&server, sizeof(server), 0);
		server.sin_addr.s_addr = inet_addr(ServerIP);
		server.sin_family = AF_INET;
		server.sin_port = htons(g_nServerPort);
	
		stMessage transMessage;
		transMessage.iMessageType = P2PTRANS;
		strcpy(transMessage.message.translatemessage.userName, UserName);

		sendto(PrimaryUDP, (const char*)&transMessage, sizeof(transMessage), 0, (const sockaddr*)&server, sizeof(server));

		printf("send msg to server %s %d\n", ServerIP, g_nServerPort);
		Sleep(100);// �ȴ��Է��ȷ�����Ϣ��
	}
	return false;
}

bool SendMessageTo2(char *UserName, char *Message, const char *pIP, USHORT nPort )
{
	char realmessage[256];
	unsigned int UserIP = 0L;
	unsigned short UserPort = 0;

	if ( pIP != NULL )
	{
		UserIP = ntohl( inet_addr( pIP ) );
		UserPort = nPort;
	}
	else
	{
		bool FindUser = false;
		for(UserList::iterator UserIterator=ClientList.begin();
							UserIterator!=ClientList.end();
							++UserIterator)
		{
			if( strcmp( ((*UserIterator)->userName), UserName) == 0 )
			{
				UserIP = (*UserIterator)->ip;
				UserPort = (*UserIterator)->port;
				FindUser = true;
			}
		}

		if(!FindUser)
			return false;
	}

	strcpy(realmessage, Message);

	sockaddr_in remote;
	remote.sin_addr.s_addr = htonl(UserIP);
	remote.sin_family = AF_INET;
	remote.sin_port = htons(UserPort);
	stP2PMessage MessageHead;
	MessageHead.iMessageType = P2PMESSAGE;
	MessageHead.iStringLen = (int)strlen(realmessage)+1;

	printf( "Send message, %s:%ld -> %s\n", inet_ntoa( remote.sin_addr ), ntohs( remote.sin_port ), realmessage );
	
	for(int i=0;i<MAXRETRY;i++)
	{
		RecvedACK = false;
		int isend = sendto(PrimaryUDP, (const char *)&MessageHead, sizeof(MessageHead), 0, (const sockaddr*)&remote, sizeof(remote));
		isend = sendto(PrimaryUDP, (const char *)&realmessage, MessageHead.iStringLen, 0, (const sockaddr*)&remote, sizeof(remote));
		
		// �ȴ������߳̽��˱���޸�
		for(int j=0;j<10;j++)
		{
			if(RecvedACK)
				return true;
			else
				Sleep(300);
		}
	}
	return false;
}

// ���������ʱֻ��exit��send����
// ����getu�����ȡ��ǰ�������������û�
void ParseCommand(char * CommandLine)
{
	if(strlen(CommandLine)<4)
		return;
	char Command[10];
	strncpy(Command, CommandLine, 4);
	Command[4]='\0';

	if(strcmp(Command,"exit")==0)
	{
		stMessage sendbuf;
		sendbuf.iMessageType = LOGOUT;
		strncpy(sendbuf.message.logoutmember.userName, UserName, 10);
		sockaddr_in server;
		server.sin_addr.s_addr = inet_addr(ServerIP);
		server.sin_family = AF_INET;
		server.sin_port = htons(g_nServerPort);

		sendto(PrimaryUDP,(const char*)&sendbuf, sizeof(sendbuf), 0, (const sockaddr *)&server, sizeof(server));
		shutdown(PrimaryUDP, 2);
		closesocket(PrimaryUDP);
		exit(0);
	}
	else if(strcmp(Command,"send")==0)
	{
		char sendname[20];
		char message[COMMANDMAXC];
		int i;
		for(i=5;;i++)
		{
			if(CommandLine[i]!=' ')
				sendname[i-5]=CommandLine[i];
			else
			{
				sendname[i-5]='\0';
				break;
			}
		}
		strcpy(message, &(CommandLine[i+1]));
		if(SendMessageTo(sendname, message))
			printf("Send OK!\n");
		else 
			printf("Send Failure!\n");
	}
	else if(strcmp(Command,"tell")==0)
	{
		char sendname[20];
		char sendto[ 64 ] = {0};
		char message[COMMANDMAXC];
		int i;
		for(i=5;;i++)
		{
			if(CommandLine[i]!=' ')
				sendname[i-5]=CommandLine[i];
			else
			{
				sendname[i-5]='\0';
				break;
			}
		}

		i++;
		int nStart = i;
		for(;;i++)
		{
			if(CommandLine[i]!=' ')
				sendto[i-nStart]=CommandLine[i];
			else
			{
				sendto[i-nStart]='\0';
				break;
			}
		}

		strcpy(message, &(CommandLine[i+1]));

		char szIP[32] = {0};
		char *p1 = sendto;
		char *p2 = szIP;
		while ( *p1 != ':' )
		{
			*p2++ = *p1++;	
		}

		p1++;
		USHORT nPort = atoi( p1 );

		if(SendMessageTo2(sendname, message, strcmp( szIP, "255.255.255.255" ) ? szIP : NULL, nPort ))
			printf("Send OK!\n");
		else 
			printf("Send Failure!\n");
	}
	else if(strcmp(Command,"getu")==0)
	{
		int command = GETALLUSER;
		sockaddr_in server;
		server.sin_addr.s_addr = inet_addr(ServerIP);
		server.sin_family = AF_INET;
		server.sin_port = htons(g_nServerPort);

		sendto(PrimaryUDP,(const char*)&command, sizeof(command), 0, (const sockaddr *)&server, sizeof(server));
	}
}

// ������Ϣ�߳�
#ifndef LINUX
DWORD WINAPI RecvThreadProc(LPVOID lpParameter)
#else
//void RecvThreadProc();
void* RecvThreadProc(void* lpParameter)
#endif
{
	sockaddr_in remote;
	socklen_t sinlen = sizeof(remote);
	stP2PMessage recvbuf;
	for(;;)
	{
		memset(&remote, sizeof(remote), 0);
		int iread = recvfrom(PrimaryUDP, (char *)&recvbuf, sizeof(recvbuf), 0, (sockaddr *)&remote, &sinlen);
		if(iread<=0)
		{
			printf("recv error\n");
			continue;
		}
		switch(recvbuf.iMessageType)
		{
		case P2PMESSAGE:
			{
				// ���յ�P2P����Ϣ
				char *comemessage= new char[recvbuf.iStringLen];
				int iread1 = recvfrom(PrimaryUDP, comemessage, 256, 0, (sockaddr *)&remote, &sinlen);
				comemessage[iread1-1] = '\0';
				if(iread1<=0)
					throw Exception("Recv Message Error\n");
				else
				{
					printf("Recv a Message, %s:%ld -> %s\n", inet_ntoa( remote.sin_addr), htons(remote.sin_port), comemessage);
					
					stP2PMessage sendbuf;
					sendbuf.iMessageType = P2PMESSAGEACK;
					sendto(PrimaryUDP, (const char*)&sendbuf, sizeof(sendbuf), 0, (const sockaddr*)&remote, sizeof(remote));
					printf("Send a Message Ack to %s:%ld\n", inet_ntoa( remote.sin_addr), htons(remote.sin_port) );
				}

				delete []comemessage;
				break;

			}
		case P2PSOMEONEWANTTOCALLYOU:
			{
				// ���յ��������ָ����IP��ַ��
				printf("Recv p2someonewanttocallyou from %s:%ld\n", inet_ntoa( remote.sin_addr), htons(remote.sin_port) );

				sockaddr_in to_addr;
				memset(&to_addr, sizeof(to_addr), 0);
				to_addr.sin_addr.s_addr = htonl(recvbuf.iStringLen);
				to_addr.sin_family = AF_INET;
				to_addr.sin_port = htons(recvbuf.Port);

				// UDP hole punching
				stP2PMessage message;
				message.iMessageType = P2PTRASH;
				sendto(PrimaryUDP, (const char *)&message, sizeof(message), 0, (const sockaddr*)&to_addr, sizeof(to_addr));
	
				printf("Send p2ptrash to %s:%ld\n", inet_ntoa(to_addr.sin_addr), htons(to_addr.sin_port));

				stMessage msg;
				msg.iMessageType = CLINET_TEST;
				strcpy(msg.message.testContent.content, "test resend...");
				sendto(PrimaryUDP, (const char *)&msg, sizeof(msg), 0, (const sockaddr*)&remote, sizeof(remote));
				printf("Send client test to %s:%ld\n", inet_ntoa(remote.sin_addr), htons(remote.sin_port));

				break;
			}
		case P2PMESSAGEACK:
			{
				// ������Ϣ��Ӧ��
				RecvedACK = true;
				printf("Recv message ack from %s:%ld\n", inet_ntoa( remote.sin_addr), htons(remote.sin_port) );

				break;
			}
		case P2PTRASH:
			{
				// �Է����͵Ĵ���Ϣ�����Ե���
				//do nothing ...
				printf("Recv p2ptrash data from %s:%ld\n", inet_ntoa( remote.sin_addr), htons(remote.sin_port) );
				stP2PMessage message;
				message.iMessageType = P2PTRASH_BACK;

				sendto(PrimaryUDP, (const char*)&message, sizeof(message), 0, (const sockaddr*)&remote, sizeof(remote));
				printf("Send p2ptrash back\n");
				break;
			}
		case P2PTRASH_BACK:
			{
				printf("Recv p2ptrash back data from %s:%ld\n", inet_ntoa(remote.sin_addr), htons(remote.sin_port));
				break;
			}
		case GETALLUSER:
			{
				int usercount;
				socklen_t fromlen = sizeof(remote);
				int iread = recvfrom(PrimaryUDP, (char *)&usercount, sizeof(int), 0, (sockaddr *)&remote, &fromlen);
				if(iread<=0)
				{
					throw Exception("Login error\n");
				}
				
				ClientList.clear();

				std::cout<<"Have "<<usercount<<" users logined server:"<<std::endl;
				for(int i = 0;i<usercount;i++)
				{
					stUserListNode *node = new stUserListNode;
					recvfrom(PrimaryUDP, (char*)node, sizeof(stUserListNode), 0, (sockaddr *)&remote, &fromlen);
					ClientList.push_back(node);
					std::cout<<"Username:"<<node->userName<<std::endl;
					in_addr tmp;
					tmp.s_addr = htonl(node->ip);
					std::cout<<"UserIP:"<<inet_ntoa(tmp)<<std::endl;
					std::cout<<"UserPort:"<<node->port<<std::endl;
					std::cout<<""<<std::endl;
				}
				break;
			}
		}
	}
}

int testNATProp()
{
	try
	{
		InitWinSock();
		
		PrimaryUDP = mksock(SOCK_DGRAM);
		BindSock(PrimaryUDP);

		char szServerIP1[ 32 ] = {0};
		char szServerIP2[ 32 ] = {0};

		std::cout<<"Please input server1 ip:";
		std::cin>>szServerIP1;

		std::cout<<"Please input server2 ip:";
		std::cin>>szServerIP2;

		sockaddr_in remote1;
		remote1.sin_addr.s_addr = inet_addr(szServerIP1);
		remote1.sin_family = AF_INET;
		remote1.sin_port = htons(g_nServerPort);
		
		sockaddr_in remote2;
		remote2.sin_addr.s_addr = inet_addr(szServerIP2);
		remote2.sin_family = AF_INET;
		remote2.sin_port = htons(g_nServerPort);

		char chData = 'A';
		int nCount = 0;

		for(;;)
		{
			nCount++;
			printf( "send message to: %s:%ld, %ld\n", szServerIP1, g_nServerPort, nCount );

			sendto(PrimaryUDP, (const char*)&chData, sizeof(char), 0, (const sockaddr*)&remote1,sizeof(remote1));

			if ( szServerIP2[ 0 ] != 'x' )
			{
				printf( "send message to: %s:%ld, %ld\n", szServerIP2, g_nServerPort, nCount );
				sendto(PrimaryUDP, (const char*)&chData, sizeof(char), 0, (const sockaddr*)&remote2,sizeof(remote2));
			}

			Sleep( 5000 );
		}
	}
	catch(Exception &e)
	{
		printf(e.GetMessage());
		return 1;
	}
	return 0;
}


int main(int argc, char* argv[])
{
//	testNATProp();
// 	return 0;

	if ( argc > 1 )
	{
		g_nClientPort = atoi( argv[ 1 ] );
	}

	if ( argc > 2 )
	{
		g_nServerPort = atoi( argv[ 2 ] );
	}

	try
	{
		InitWinSock();
		
		PrimaryUDP = mksock(SOCK_DGRAM);
		BindSock(PrimaryUDP);

		std::cout<<"Please input server ip:";
		std::cin>>ServerIP;

		std::cout<<"Please input your name:";
		std::cin>>UserName;

		ConnectToServer(PrimaryUDP, UserName, ServerIP);
#ifndef LINUX
		HANDLE threadhandle = CreateThread(NULL, 0, RecvThreadProc, NULL, NULL, NULL);
		CloseHandle(threadhandle);
#else
		pthread_t thread_id;
		pthread_attr_t attr;
		int s = pthread_attr_init(&attr);

		s = pthread_create(&thread_id, &attr,
			&RecvThreadProc, NULL);
		if (s != 0)
			printf("pthread_create %d\n", s);
#endif

		//std::thread td(std::bind(RecvThreadProc));

		OutputUsage();

		for(;;)
		{
			char Command[COMMANDMAXC];
			gets(Command);
			ParseCommand(Command);
		}
	}
	catch(Exception &e)
	{
		printf(e.GetMessage());
		return 1;
	}
	return 0;
}

