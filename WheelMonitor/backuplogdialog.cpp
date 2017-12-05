#include "backuplogdialog.h"
#include "ui_backuplogdialog.h"

BackupLogDialog::BackupLogDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BackupLogDialog)
{
    ui->setupUi(this);
}

BackupLogDialog::~BackupLogDialog()
{
    delete ui;
}
