#include "stdafx.h"

#include "common.h"
#include "confighelper.h"

/* Groups Name */
const QString GP_CAM = "IPCamera";
const QString GP_IM = "ImageProess";
const QString GP_OCR = "ocr";
const QString GP_COMMON = "Common";
const QString GP_DEV = "Device";

ConfigHelper::ConfigHelper(const QString& configuration, QObject* parent /*= Q_NULLPTR*/)
    : QObject(parent)
    , configFile(configuration) // 初始化列表有顺序之分
{
    read();
}

ConfigHelper::~ConfigHelper()
{
    // 因为ConfigHelper是QObject子类，类指针成员会被qt内存管理，不需要delete/deleteLater
    save(); //on exit, helper2ini.
}

void ConfigHelper::read()
{
    auto settings = new QSettings(configFile, QSettings::IniFormat, this); //创建名为configFile的INI文件，而不是放在注册表中
    settings->beginGroup(GP_COMMON);
    launchAtLogin = settings->value("launchAtLogin", true).toBool();
    startAtLaunch = settings->value("startAtLaunch", true).toBool();
    verboseLog = settings->value("verboseLog", true).toBool();
    pc2plc_portName = settings->value("pc2plc_portName", "COM3").toString(); // 默认COM3口
    settings->endGroup();

    int size = settings->beginReadArray(GP_DEV);
    if (size != 2) {
        QMessageBox::critical(nullptr, QStringLiteral("错误"), QStringLiteral("未能读取配置文件"), QStringLiteral("确定"));
        qCritical() << "ConfigHelper: read() error";
    }
    for (int i = 0; i < size; ++i) {
        settings->setArrayIndex(i);
        auto& dev = device[i];
        //settings->setValue("CamProfile", QVariant::fromValue<CamProfile>(device[i].camProfile));
        settings->beginGroup(GP_CAM);
        auto& cam = dev.camProfile;
        cam.camIP = settings->value("camIP").toString();
        cam.camPort = settings->value("camPort").toInt();
        cam.camUserName = settings->value("camUserName").toString();
        cam.camPassword = settings->value("camPassword").toString();
        cam.frameInterv = settings->value("frameInterv").toInt();
        settings->endGroup();
        settings->beginGroup(GP_IM);
        auto& im = dev.imProfile;
        im.warningRatio = settings->value("warningRatio").toDouble();
        im.alarmRatio = settings->value("alarmRatio").toDouble();
        im.radius_min = settings->value("radius_min").toInt();
        im.radius_max = settings->value("radius_max").toInt();
        im.gs1 = settings->value("gs1").toInt();
        im.gs2 = settings->value("gs2").toInt();
        im.dp = settings->value("dp").toDouble();
        im.minDist = settings->value("minDist").toDouble();
        im.param1 = settings->value("param1").toDouble();
        im.param2 = settings->value("param2").toDouble();
        im.sensorTriggered = settings->value("sensorTriggered").toBool();
        im.roiRect.x = settings->value("roiRect.x").toInt();
        im.roiRect.y = settings->value("roiRect.y").toInt();
        im.roiRect.width = settings->value("roiRect.width").toInt();
        im.roiRect.height = settings->value("roiRect.height").toInt();
        settings->endGroup();
        settings->beginGroup(GP_OCR);
        auto& ocr = dev.ocrProfile;
        ocr.plate_x_min = settings->value("plate_x_min").toInt();
        ocr.plate_x_max = settings->value("plate_x_max").toInt();
        ocr.plate_y_min = settings->value("plate_y_min").toInt();
        ocr.plate_y_max = settings->value("plate_y_max").toInt();
        ocr.plate_width_min = settings->value("plate_width_min").toInt();
        ocr.plate_width_max = settings->value("plate_width_max").toInt();
        ocr.plate_height_min = settings->value("plate_height_min").toInt();
        ocr.plate_height_max = settings->value("plate_height_max").toInt();
        ocr.num_width_min = settings->value("num_width_min").toInt();
        ocr.num_width_max = settings->value("num_width_max").toInt();
        ocr.num_height_min = settings->value("num_height_min").toInt();
        ocr.num_height_max = settings->value("num_height_max").toInt();
        settings->endGroup();
    }
    settings->endArray();
}

void ConfigHelper::save()
{
    auto settings = new QSettings(configFile, QSettings::IniFormat, this);
    takeEffect();
    settings->beginGroup(GP_COMMON);
    settings->setValue("launchAtLogin", QVariant(launchAtLogin));
    settings->setValue("startAtLaunch", QVariant(startAtLaunch));
    settings->setValue("verboseLog", QVariant(verboseLog));
    settings->setValue("pc2plc_portName", QVariant(pc2plc_portName));
    settings->endGroup();

    settings->beginWriteArray(GP_DEV);
    for (int i = 0; i < 2; ++i) {
        settings->setArrayIndex(i);
        auto& dev = device[i];
        //settings->setValue("CamProfile", QVariant::fromValue<CamProfile>(device[i].camProfile));
        settings->beginGroup(GP_CAM);
        auto& cam = dev.camProfile;
        settings->setValue("camIP", QVariant(cam.camIP));
        settings->setValue("camPort", QVariant(cam.camPort));
        settings->setValue("camUserName", QVariant(cam.camUserName));
        settings->setValue("camPassword", QVariant(cam.camPassword));
        settings->setValue("frameInterv", QVariant(cam.frameInterv));
        settings->endGroup();
        settings->beginGroup(GP_IM);
        auto& im = dev.imProfile;
        settings->setValue("warningRatio", QVariant(im.warningRatio));
        settings->setValue("alarmRatio", QVariant(im.alarmRatio));
        settings->setValue("radius_min", QVariant(im.radius_min));
        settings->setValue("radius_max", QVariant(im.radius_max));
        settings->setValue("gs1", QVariant(im.gs1));
        settings->setValue("gs2", QVariant(im.gs2));
        settings->setValue("dp", QVariant(im.dp));
        settings->setValue("minDist", QVariant(im.minDist));
        settings->setValue("param1", QVariant(im.param1));
        settings->setValue("param2", QVariant(im.param2));
        settings->setValue("sensorTriggered", QVariant(im.sensorTriggered));
        settings->setValue("roiRect.x", QVariant(im.roiRect.x));
        settings->setValue("roiRect.y", QVariant(im.roiRect.y));
        settings->setValue("roiRect.width", QVariant(im.roiRect.width));
        settings->setValue("roiRect.height", QVariant(im.roiRect.height));
        settings->endGroup();
        settings->beginGroup(GP_OCR);
        auto& ocr = dev.ocrProfile;
        settings->setValue("plate_x_min", QVariant(ocr.plate_x_min));
        settings->setValue("plate_x_max", QVariant(ocr.plate_x_max));
        settings->setValue("plate_y_min", QVariant(ocr.plate_y_min));
        settings->setValue("plate_y_max", QVariant(ocr.plate_y_max));
        settings->setValue("plate_width_min", QVariant(ocr.plate_width_min));
        settings->setValue("plate_width_max", QVariant(ocr.plate_width_max));
        settings->setValue("plate_height_min", QVariant(ocr.plate_height_min));
        settings->setValue("plate_height_max", QVariant(ocr.plate_height_max));
        settings->setValue("num_width_min", QVariant(ocr.num_width_min));
        settings->setValue("num_width_max", QVariant(ocr.num_width_max));
        settings->setValue("num_height_min", QVariant(ocr.num_height_min));
        settings->setValue("num_height_max", QVariant(ocr.num_height_max));
        settings->endGroup();
    }
    settings->endArray();
}

void ConfigHelper::takeEffect() const
{
    effect_launchAtLogin();
}

void ConfigHelper::effect_launchAtLogin() const
{
    //HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
    QSettings* reg = new QSettings(R"(HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run)", QSettings::NativeFormat);

    if (launchAtLogin) {
        QString strAppPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        //strAppPath.replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive);
        reg->setValue(qApp->applicationName(), QString("%1 --auto").arg(strAppPath));
    } else {
        reg->remove(qApp->applicationName());
    }
    reg->deleteLater();
}