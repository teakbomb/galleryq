#include "sql.h"
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QSqlDatabase>
#include <QRandomGenerator>

#include "storage.h"

Sql::Sql(QObject *parent) : QSqlQueryModel(parent) {
    static Reader r{}; //default db is on the writer thread, we need our own
    db = r.db;
    reload_timer = new QTimer(this);
    reload_timer->setSingleShot(true);
    connect(reload_timer, &QTimer::timeout, this, &Sql::reload);
    connect(Storage::instance(), &Storage::refresh, this, &Sql::refresh);
}

Sql::~Sql() {
    disconnect(Storage::instance(), &Storage::refresh, this, &Sql::refresh);
}

void Sql::setQuery(const QString &query, const QSqlDatabase &db) {
    auto start = QDateTime::currentMSecsSinceEpoch();
    QSqlQueryModel::setQuery(query, this->db);
    auto end = QDateTime::currentMSecsSinceEpoch();
    //qInfo() << end - start << query;

    if(lastError().isValid())
        qInfo() << query << lastError();

    while(canFetchMore())
        fetchMore();

    this->query = query;

    generateRoleNames();

    emit queryChanged();
}

void Sql::setQuery(const QSqlQuery & query) {
    qInfo() << "not safe";
}

void Sql::reload(){
    setQuery(getQuery());
}

QString Sql::getQuery() {
    return query;
}

bool Sql::hasErrored() {
    return lastError().type() != QSqlError::NoError;
}

void Sql::generateRoleNames() {
   m_roleNames.clear();
   for( int i = 0; i < record().count(); i ++) {
       m_roleNames.insert(Qt::UserRole + i + 1, "sql_" + record().fieldName(i).toUtf8());
   }
   m_roleNames.insert(Qt::DisplayRole, "display");
}

QVariant Sql::data(const QModelIndex &index, int role) const {
    QVariant value;
    if(role < Qt::UserRole) {
        value = QSqlQueryModel::data(index, role);
    } else {
        int columnIdx = role - Qt::UserRole - 1;
        QModelIndex modelIndex = this->index(index.row(), columnIdx);
        value = QSqlQueryModel::data(modelIndex, Qt::DisplayRole);
    }
    return value;
}

void Sql::refresh(const QString& name, const QVariant& payload) {
    if(!reload_timer->isActive()) {
        reload_timer->start(100 + (QRandomGenerator::global()->generate() % 100));
    }
}
