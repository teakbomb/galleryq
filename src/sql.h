#ifndef SQL_H
#define SQL_H

#include <QObject>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <qqml.h>
#include <QSqlDriver>
#include <QDateTime>
#include <QTimer>
#include <QRandomGenerator>

class Sql : public QSqlQueryModel
{
    Q_OBJECT
    Q_PROPERTY(QString query READ getQuery WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY queryChanged)
    Q_PROPERTY(bool errored READ hasErrored NOTIFY queryChanged)
    QML_ELEMENT
public:
    explicit Sql(QObject *parent = 0);
    ~Sql();
    void setQuery(const QString &query, const QSqlDatabase &db = QSqlDatabase());
    void setQuery(const QSqlQuery &query);
    QString getQuery();
    bool hasErrored();
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const {	return m_roleNames;	}
private:
    void generateRoleNames();
    QHash<int, QByteArray> m_roleNames;
    QString query;
    QTimer *reload_timer;
    QSqlDatabase db;
signals:
    void queryChanged();
public slots:
    void reload();
    void refresh(const QString& name, const QVariant& payload);
};

#endif // SQL_H
