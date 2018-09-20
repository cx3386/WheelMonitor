#include "stdafx.h"
#include "LevelRecorder.h"

LevelRecorder::LevelRecorder()
{
	levels.reserve(this->max_size);
}

LevelRecorder::~LevelRecorder()
{
}

void LevelRecorder::push(bool lv)
{
	if (levels.size() == max_size)
	{
		levels.erase(levels.begin());
	}
	levels.push_back(lv);
}

int LevelRecorder::state(size_t pos)
{
	auto endId = levels.size() - 1;
	// 更早的数据pos+1，应该存在
	if (pos + 1 > endId) return OutOfRange;
	auto now = levels.at(endId - pos);
	auto pre = levels.at(endId - pos - 1);
	int code = now ? HighLevel : LowLevel;
	if (now^pre)
	{
		if (now)
		{
			code |= 0x4;
		}
		else
		{
			code |= 0x8;
		}
	}
	return code;
}