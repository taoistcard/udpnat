/* �쳣��
 *
 * �ļ�����Exception.h
 *
 * ���ڣ�2004.5.5
 *
 * ���ߣ�shootingstars(zhouhuis22@sina.com)
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

#ifndef __HZH_Exception__
#define __HZH_Exception__

#define EXCEPTION_MESSAGE_MAXLEN 256
#include "string.h"

class Exception
{
private:
	char m_ExceptionMessage[EXCEPTION_MESSAGE_MAXLEN];
public:
	Exception(const char *msg)
	{
		strncpy(m_ExceptionMessage, msg, EXCEPTION_MESSAGE_MAXLEN);
	}

	char *GetMessage()
	{
		return m_ExceptionMessage;
	}
};



#endif