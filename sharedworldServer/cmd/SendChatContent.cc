#include "SendChatContent.h"
#include <string>
#include "../SharedSession.h"
#include <iostream>
#include "../dal/SharedService.h"
#include "CodeConverter.h"
using namespace std;

void SendChatContent::Execute(SharedSession& session)
{
	CodeConverter codeConvert = CodeConverter("utf-8", "gb2312"); 

	JInStream jis(session.GetRequestPack()->buf, session.GetRequestPack()->head.len);
	uint16 cmd = session.GetCmd();

	// 发送者用户名
	string username;
	jis>>username;

	char userName[1000]={0};
	const int userlen = username.length();
	char* iuser = new char[userlen+1];
	strcpy(iuser, username.c_str());
	codeConvert.convert(iuser, strlen(iuser), userName, 1000);
	cout<<username<<endl;

	//发送的聊天内容
	string content;
	jis>>content;
	cout<<"send content:"<<content<<endl;

	
	char Content[1000]={0};
	const int contentlen = content.length();
	char* icontent = new char[contentlen+1];
	strcpy(icontent, content.c_str());
	codeConvert.convert(icontent, strlen(icontent), Content, 1000);

	MD5 md5;
	int16 error_code = 0;
	char error_msg[31] = {0};


	JOutStream& jos = session.GetJos();


	// 包头命令
	jos<<cmd;
	size_t lengthPos = jos.Length();
	jos.Skip(2);
	// 包头cnt、seq、error_code、error_msg
	uint16 cnt = 0;
	uint16 seq = 0;
	jos<<cnt<<seq<<error_code;
	jos.WriteBytes(error_msg, 30);
	jos<<userName;
	jos<<Content;
	jos<<"end";
	size_t tailPos = jos.Length();
	jos.Reposition(lengthPos);
	jos<<(uint16)(tailPos + 8 - sizeof(ResponseHead));


	// 包尾
	jos.Reposition(tailPos);
	// 计算包尾
	unsigned char hash[16];
	md5.MD5Make(hash, (const unsigned char*)jos.Data(), jos.Length());
	for (int i=0; i<8; ++i)
	{
		hash[i] = hash[i] ^ hash[i+8];
		hash[i] = hash[i] ^ ((cmd >> (i%2)) & 0xff);
	}
	jos.WriteBytes(hash, 8);

	//session.Send(jos.Data(), jos.Length());
	
}

