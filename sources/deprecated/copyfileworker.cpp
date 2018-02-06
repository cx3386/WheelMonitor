#include "stdafx.h"
#include "copyfileworker.h"
#include <QProgressDialog>

CopyFileWorker::CopyFileWorker(QString& src, QString& des, QStringList& ls, QObject *parent /*= Q_NULLPTR*/)
	:QObject(parent)
	, srcPath(src)
	, desPath(des)
	, fileList(ls)
{
}

CopyFileWorker::~CopyFileWorker()
{
}

bool CopyFileWorker::startCopy()
{
	connect(this, &CopyFileWorker::setPgDlgValue, pgDlg, &QProgressDialog::setValue);
	quint64 totalSize = 0;
	for (auto&& qs : std::as_const(fileList))
	{
		QFileInfo info(qs);
		totalSize += info.size();
	}
	pgDlg->setMaximum(totalSize / 1024);
	//emit setPgDlgValue(totalSize / 1024);
	QString diver = desPath.left(3);
	LPCWSTR lpcwstrDriver = (LPCWSTR)diver.utf16();
	ULARGE_INTEGER liFreeBytesAvailable, liTotalBytes, liTotalFreeBytes;
	if (!GetDiskFreeSpaceEx(lpcwstrDriver, &liFreeBytesAvailable, &liTotalBytes, &liTotalFreeBytes))
	{
		qDebug() << "CopyFileWorker: Call to GetDiskFreeSpaceEx() failed.";
		return 0;
	}
	quint64 freeSize = liTotalFreeBytes.QuadPart;

	if (totalSize > freeSize)
	{
		emit setPgDlgValue(totalSize / 1024);	//close
		return false;
	}

	quint64 curSize = 0;
	QDir srcDir(srcPath);
	//QString tmp_desPath(desPath);
	//if (tmp_desPath.endsWith("/"))
	//{
	//	tmp_desPath.chop(1);
	//}

	for (auto&& qs : std::as_const(fileList))//fileList must start with srcPath
	{
		QString relativeFilePath = srcDir.relativeFilePath(qs);
		QString desFileName = QString("%1/%2").arg(desPath).arg(relativeFilePath);//get the relative path
		QFileInfo desFileInfo(desFileName);
		//if (fileInfo.isFile()) // if exist then delete
		//{
		//	QFile::remove(desFileName);
		//}
		QFile desFile(desFileName);
		//desFile.setPermissions(QFile::WriteOwner);
		if (!desFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
		{
			QDir dir;
			dir.mkpath(desFileInfo.absolutePath());//desFileInfo.canonicalPath() //returns "."
			desFile.open(QIODevice::WriteOnly);
		}
		QFile srcFile(qs);
		srcFile.open(QIODevice::ReadOnly);

		QByteArray byteArray;
		int count = 0;
		while (!srcFile.atEnd())
		{
			count++;
			byteArray = srcFile.read(1024);
			desFile.write(byteArray);
			emit setPgDlgValue(curSize / 1024 + count);

			if (pgDlg->wasCanceled())
			{
				desFile.close();
				srcFile.close();
				QFile::remove(desFileName);
				emit setPgDlgValue(totalSize / 1024);	//close
				return true;
			}
		}

		desFile.close();
		srcFile.close();
		curSize += desFileInfo.size();
	}
	return true;
}