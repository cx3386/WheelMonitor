/**
 * \file SpeedIntegrator.h
 *
 * \author cx3386
 * \date ʮ���� 2018
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
	/*���̵߳���*/
	void start(); //!< ��ʼ���֡�����ʱ�䣺plc��ʼ����
	void stop();

private:
	Plc* plc;
	bool bUsrCtrl = false;
	//QTimer* readSpeedTimer; //!< ��ʱ��ȡ̨���ٶ�
	static const int readSpeedInterval = Plc::adInterval; //!< ��ȡad�ļ��ms
	static const double std_L; //!< ��׼�־�
	static const double dist_min; // unit:m�����ֵľ�����5cm����
	static const double dist_max; // m
	struct Ckpt {
		bool bFirstStart = false; //!< �ɵ�һ�����������tri�źſ�ʼ������һ��usrstop,���µȴ���һ��tri
		double distance = 0; //!< �������Ѿ��߹����г�
		double anchor = 0; //!< tri�ĵص����s����λ��
		bool hasTri = false; //!< ���������Ƿ��Ѿ��յ�tri�ź�
		bool triBeginReady = false; //!< �ѷ���exptectbegin�ź�
		bool triEndReady = false; //!< �ѷ���expectend�ź�
		void newWheel()
		{
			// �������û���յ�tri�źţ���ê��Ϊ��׼�־�
			if (!hasTri) {
				anchor = std_L;
				qDebug() << "no tri in expect zone";
			}
			distance = distance - anchor;
			anchor = 0;
			hasTri = false;
			triBeginReady = false;
			triEndReady = false;
		} //!< ����maxʱ����ʼһ���µĻ�������
		//void init() {} //!< ϵͳ������״̬�ѳ������½���ʱ����ʼ���ṹ
	} ckps[4]; //col,cor,cil,cir

	void speedIntegrator();

public slots:
	void onCkpTri(int ckpId);

signals:
	void expectTriBegin(int ckpId);
	void expectTriEnd(int ckpId);
};
