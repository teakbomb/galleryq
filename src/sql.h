#ifndef SQL_H
#define SQL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QList>
#include <QSqlRecord>
#include <QTimer>
#include <qqml.h>

#include "storage.h"

class Sql : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString query READ getQuery WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY queryChanged)
    Q_PROPERTY(bool errored READ hasErrored NOTIFY queryChanged)
    QML_ELEMENT
public:
    Sql(QObject *parent=nullptr);
    void setQuery(const QString& query);
    QString getQuery();
    bool hasErrored();
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    //int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QHash<int, QByteArray> roleNames() const;
private:
    Reader* reader;
    QString query;
    QList<QSqlRecord> results;
    bool errored = false;
    QHash<int, QByteArray> fieldNames;
    QTimer *reloadTimer;
    void updateFieldNames(QSqlRecord record);
    void reset();
    void updateResults(QList<QSqlRecord> newResults);
signals:
    void queryChanged();
public slots:
    void reload();
private slots:
    void dbChanged(const QString& name, const QVariant& payload);
};

#endif // SQL_H
