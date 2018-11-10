/*!
 * \file LevelRecorder.h
 * \date 2018/09/19 20:10
 *
 * \author cx3386
 * Contact: cx3386@163.com
 * \par Copyright(c):
 * cx3386.
 * All Rights Reserved
 * \brief ���ڼ�¼��ƽ�ĸߵ�
 *
 * TODO: long description
 *
 * \note
 *
 * \version 1.0
 */
#pragma once

class LevelRecorder
{
public:
	LevelRecorder();
	~LevelRecorder();
	void init(bool b = false) { levels.clear(); levels.push_back(b); }
	void push(bool); //!< push_back a data to levels
	//! clear all data
	void clear() { levels.clear(); }
	int state(size_t pos); //!< �����pos�µ����ݵ�״̬��pos=0Ϊ���µ����ݵ�
	enum State
	{
		HighLevel = 0x1,
		LowLevel = 0x2,
		PositiveEdge = HighLevel | 0x4, //!< ���Ǹߵ�ƽҲ��������
		NegativeEdge = LowLevel | 0x8, //!< ��ʹ�͵�ƽҲ���½���
		OutOfRange = 0x10,
	};
private:
	const size_t max_size = 1024;
	std::vector<bool> levels;
};
