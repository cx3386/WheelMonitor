#pragma once

#include "common.h"
#include "ui_mainwindow.h"
#include <QtWidgets/QMainWindow>

class ViewSensorImage;
class PlayBackWidget;
class SettingDialog;
class ImageProcess;
class HikVideoCapture;
class Plc;
class QThread;
class ConfigHelper;
class AlarmManager;
class QSqlTableModel;
class QDataWidgetMapper;
enum class AlarmColor;
class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	//! 只有该类可以改变config，因此不用const
	explicit MainWindow(ConfigHelper* _configHelper, QWidget* parent = Q_NULLPTR);
	~MainWindow();

private:
	Ui::MainWindowClass ui;
	ConfigHelper* configHelper;
	QMutex mutex;
	bool bIsRunning = false;
	/* rec label */
	QLabel* recLabel_pre[2]; //in preview tab
	QLabel* recLabel_input[2]; //in real frame
	/* worker */
	HikVideoCapture* videoCapture[2];
	PlayBackWidget* playBackWidget;
	ImageProcess* imageProcess[2];
	Plc* plc;
	AlarmManager* alarmManager;
	/* thread */
	//thread must be managed in main thread. ~mainwindow(){thread.quit()}
	//QThread *videoCapture
	QThread* imageProcessThread[2];
	QThread* plcThread;
	QThread* dbWatcherThread;
	//数据模型和映射
	QSqlTableModel *outerModel, *innerModel;
	QDataWidgetMapper *outerMapper, *innerMapper, *alarmMapper;
	/*global*/
	void configWindow();
	void setupDatabaseWatcher(); //!< 建立对数据库文件的监视
	void setupDataMapper(); //!< 建立数据库与界面的映射关系
	bool cleanDir(QString dir, int nDays); //!< retain nDays save files, include video/match/log and so on, defualt retain for 0 days, which means clean all
	/* alarm number show */
	//void alarmNumUiSetup();

	//AlarmEvent currentAlarmEvent; ///< current alarm state
	//docks
	QList<QDockWidget*> m_docks;

protected:
	virtual void closeEvent(QCloseEvent* event) override;

public slots:
	void uiShowCio100(int cio100);
	void uiShowAlarmLight(AlarmColor alarmColor);
private slots:
	/* uiShow */
	void uiShowRealtimeImage(int deviceIndex);
	void uiShowTruckSpeed();
	void uiShowCio0(int cio0);
	void uiShowSensor(int state, int devId);
	/* onAlarmChanged */
	//void onAlarmChanged(AlarmEvent alarmevent);
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
	void showViewSensorImage();

signals:
	/* about plcserial thread */
	void alarmUi2PLC(AlarmColor alarmcolor);
};
