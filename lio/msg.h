#ifndef L_MSG_H_
#define L_MSG_H_

#include "stddef.h"
#include "string.h"

enum
{
	//一系列消息类型的定义
	MSG_TYPE_TEST,
};

struct MSG
{
	int type;
	int from;
	int target;
	int param;
	size_t content_length;
	char* content;
	MSG(int type_=0, int from_=0, int target_=0, int param_=0) :
		type(type_), from(from_), target(target_), param(param_), content_length(0), content(NULL) {}
	MSG(const MSG& msg)
	{
		memcpy(this, &msg, sizeof(msg));
		if(content_length > 0 && content)
		{
			content = new char[content_length];
			memcpy(content, msg.content, content_length);
		}
	}
	~MSG()
	{
		if(content_length > 0 && content)
			delete[] content;
	}
	MSG &operator=(const MSG& msg)
	{
		if(&msg == this) return *this;
		memcpy(this, &msg, sizeof(msg));
		if(content_length > 0 && content)
		{
			content = new char[content_length];
			memcpy(content, msg.content, content_length);
		}
		return *this;
	}
};

#endif // L_MSG_H_
