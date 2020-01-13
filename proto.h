/* P2P ������Э��
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

#pragma once
#include <list>

// ����iMessageType��ֵ
#define LOGIN 1
#define LOGOUT 2
#define P2PTRANS 3
#define GETALLUSER  4
#define CLINET_TEST   5			// �����������Ŀհ���Ϣ

// �������˿�
#define SERVER_PORT 41010 //6060

// Client��¼ʱ����������͵���Ϣ
struct stLoginMessage
{
	char userName[10];
	char password[10];
};

// Clientע��ʱ���͵���Ϣ
struct stLogoutMessage
{
	char userName[10];
};

// Client���������������һ��Client(userName)���Լ�������UDP����Ϣ
struct stP2PTranslate
{
	char userName[10];
};

struct stClientTest
{
	char content[20];
};
// Client����������͵���Ϣ��ʽ
struct stMessage
{
	int iMessageType;
	union _message
	{
		stLoginMessage loginmember;
		stLogoutMessage logoutmember;
		stP2PTranslate translatemessage;
		stClientTest testContent;
	}message;
};

// �ͻ��ڵ���Ϣ
struct stUserListNode
{
	char userName[10];
	unsigned int ip;
	unsigned short port;
};

// Server��Client���͵���Ϣ
struct stServerToClient
{
	int iMessageType;
	union _message
	{
		stUserListNode user;
	}message;

};

//======================================
// �����Э�����ڿͻ���֮���ͨ��
//======================================
#define P2PMESSAGE 100               // ������Ϣ
#define P2PMESSAGEACK 101            // �յ���Ϣ��Ӧ��
#define P2PSOMEONEWANTTOCALLYOU 102  // ��������ͻ��˷��͵���Ϣ
                                     // ϣ���˿ͻ��˷���һ��UDP�򶴰�
#define P2PTRASH        103          // �ͻ��˷��͵Ĵ򶴰������ն�Ӧ�ú��Դ���Ϣ
#define P2PTRASH_BACK   104			// �ظ��Է��Ĵ���Ϣ
#define P2PDOWNLOAD		105			// ����һ���ļ�

// �ͻ���֮�䷢����Ϣ��ʽ
struct stP2PMessage
{
	int iMessageType;
	int iStringLen;         // or IP address or File offset
	unsigned short Port;	// or appended data size
};

//using namespace std;
typedef std::list<stUserListNode *> UserList;

