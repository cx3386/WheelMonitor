#ifndef BACKUPLOGDIALOG_H
#define BACKUPLOGDIALOG_H

#include <QDialog>

namespace Ui {
class BackupLogDialog;
}

class BackupLogDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BackupLogDialog(QWidget *parent = 0);
    ~BackupLogDialog();

private:
    Ui::BackupLogDialog *ui;
};

#endif // BACKUPLOGDIALOG_H
