/**
 * \file SpeedIntegrator.h
 *
 * \author cx3386
 * \date 十二月 2018
 *
 *
 */

#pragma once

#include <QObject>
#include "Plc.h"

 //class Plc;
class SpeedIntegrator : public QObject {
	Q_OBJECT

public:
	SpeedIntegrator(Plc* plcSerial, QObject* parent = Q_NULLPTR);
	~SpeedIntegrator();
	/*跨线程调用*/
	void start(); //!< 开始积分。调用时间：plc初始化后
	void stop();

private:
	Plc* plc;
	bool bUsrCtrl = false;
	//QTimer* readSpeedTimer; //!< 定时读取台车速度
	static const int readSpeedInterval = Plc::adInterval; //!< 读取ad的间隔ms
	static const double std_L; //!< 标准轮距
	static const double dist_min; // unit:m，积分的精度在5cm以内
	static const double dist_max; // m
	struct Ckpt {
		bool bFirstStart = false; //!< 由第一个经过检测点的tri信号开始计数，一旦usrstop,重新等待第一个tri
		double distance = 0; //!< 本周期已经走过的行程
		double anchor = 0; //!< tri的地点距离s起点的位置
		bool hasTri = false; //!< 本个轮子是否已经收到tri信号
		bool triBeginReady = false; //!< 已发出exptectbegin信号
		bool triEndReady = false; //!< 已发出expectend信号
		void newWheel()
		{
			// 如果此轮没有收到tri信号，将锚置为标准轮距
			if (!hasTri) {
				anchor = std_L;
				qDebug() << "no tri in expect zone";
			}
			distance = distance - anchor;
			anchor = 0;
			hasTri = false;
			triBeginReady = false;
			triEndReady = false;
		} //!< 到达max时，开始一个新的积分周期
		//void init() {} //!< 系统从连续状态脱出并重新进入时，初始化结构
	} ckps[4]; //col,cor,cil,cir

	void speedIntegrator();

public slots:
	void onCkpTri(int ckpId);

signals:
	void expectTriBegin(int ckpId);
	void expectTriEnd(int ckpId);
};
