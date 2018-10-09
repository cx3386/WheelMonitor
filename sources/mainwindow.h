#pragma once

#include "ui_mainwindow.h"
#include <QtWidgets/QMainWindow>
#include "common.h"

class PlayBackWidget;
class SettingDialog;
class ImageProcess;
class HikVideoCapture;
class PLCSerial;
class QThread;
class ConfigHelper;
class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	//! 只有该类可以改变config，因此不用const
	explicit MainWindow(ConfigHelper *_configHelper, QWidget *parent = Q_NULLPTR);
	~MainWindow();

private:
	Ui::MainWindowClass ui;
	ConfigHelper *configHelper;
	QMutex mutex;
	bool bIsRunning = false;
	/* rec label */
	QLabel *recLabel_pre[2];   //in preview tab
	QLabel *recLabel_input[2]; //in real frame
	/* worker */
	HikVideoCapture *videoCapture[2];
	PlayBackWidget *playBackWidget;
	ImageProcess *imageProcess[2];
	PLCSerial *plcSerial;
	/* thread */
	//thread must be managed in main thread. ~mainwindow(){thread.quit()}
	QThread *imageProcessThread[2];
	QThread *plcSerialThread;
	QThread *dbWatcherThread;
	/*global*/
	void configWindow();
	bool cleanDir(QString dir, int nDays); ///< retain nDays save files, include video/match/log and so on, defualt retain for 0 days, which means clean all
	/* alarm number show */
	void alarmNumUiSetup();
	QLineEdit *alarmIOLineEdit, *alarmNumLineEdit;
	QWidget *alarmFilter;

	AlarmEvent currentAlarmEvent; ///< current alarm state

protected:
	virtual void closeEvent(QCloseEvent *event) override;
	virtual bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
	/* uiShow */
	void uiAlarmLight(AlarmColor alarmColor);
	void uiShowRealtimeImage(int deviceIndex);
	void uiShowCartSpeed();
	void uiShowCIO_0(WORD cio0);

	/* onAlarmChanged */
	void onAlarmChanged(AlarmEvent alarmevent);
	// about REC Label show
	void onRecordON(int deviceIndex);
	void onRecordOFF(int deviceIndex);

	/* Scheduled Tasks*/
	void execScheduledTasks();
	void setupScheduler(int hh, int mm, bool execnow = true, bool everyday = true);

	/* on_clicked */
	void on_action_Start_triggered();
	void on_action_Stop_triggered();
	void on_action_Restart_triggered();
	void on_action_Property_triggered();
	void on_action_Quit_triggered();
	void on_action_Show_Log_triggered();
	void on_action_Backup_Log_triggered();
	void on_action_About_triggered();
	void on_action_About_Qt_triggered();
	void on_alarmPushButton_clicked();

signals:
	/* about plcserial thread */
	void alarmUi2PLC(AlarmColor alarmcolor);

	/* ui */
	void alarmFilterClicked();
};
