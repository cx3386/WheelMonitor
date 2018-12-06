#pragma once

#include <QSqlRelationalTableModel>

class MySqlTableModel : public QSqlRelationalTableModel {
    Q_OBJECT

public:
    MySqlTableModel(QObject* parent = Q_NULLPTR, QSqlDatabase db = QSqlDatabase())
        : QSqlRelationalTableModel(parent, db)
    {
    }

    ~MySqlTableModel() = default;

    QVariant data(const QModelIndex& item, int role = Qt::DisplayRole) const override;
};
