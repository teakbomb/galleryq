#include "sql.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QRandomGenerator>

Sql::Sql(QObject *parent) {
    static Reader r{};
    reader = &r;
    reloadTimer = new QTimer(this);
    reloadTimer->setSingleShot(true);
    connect(reloadTimer, &QTimer::timeout, this, &Sql::reload);
    connect(Storage::instance(), &Storage::refresh, this, &Sql::dbChanged);
}
void Sql::setQuery(const QString& query) {
    this->query = query;
    auto q = reader->doRead(query);
    if(q.lastError().isValid()) {
        errored = true;
        reset();
        emit queryChanged();
        return;
    }
    errored = false;

    QList<QSqlRecord> newResults;
    while(q.next()) {
        newResults.append(q.record());
    }
    q.finish();

    updateResults(newResults);

    emit queryChanged();
}

void Sql::updateResults(QList<QSqlRecord> newResults) {
    if(!newResults.isEmpty()) {
        updateFieldNames(newResults.first());
    } else {
        fieldNames.clear();
    }

    if(results.length() == 0 || newResults.length() == 0) {
        beginResetModel();

        results.clear();
        results.append(newResults);

        endResetModel();
        return;
    }

    auto shared = qMin(newResults.length(), results.length());

    for(int i = 0; i < shared; i++) {
        if(newResults[i] != results[i]) {
            results[i] = newResults[i];
            emit dataChanged(index(i), index(i));
        }
    }

    if(results.length() < newResults.length()) {
        beginInsertRows(QModelIndex(), results.length(), newResults.length()-1);

        while(results.length() < newResults.length())
            results.append(newResults[results.length()]);

        endInsertRows();

    } else if(results.length() > newResults.length()) {
        beginRemoveRows(QModelIndex(), newResults.length(), results.length()-1);

        while(results.length() > newResults.length())
            results.removeLast();

        endRemoveRows();
    }


}

QString Sql::getQuery() {
    return query;
}
bool Sql::hasErrored() {
    return errored;
}
int Sql::rowCount(const QModelIndex &parent) const {
    return results.length();
}
/*int Sql::columnCount(const QModelIndex &parent) const {
    return 1;
}*/
QVariant Sql::data(const QModelIndex &index, int role) const {

    QVariant value;
    if(role > Qt::UserRole) {
        int column = role - Qt::UserRole - 1;
        int row = index.row();
        if(row < results.length())
            value = results[row].value(column);
    }
    return value;
}

QHash<int, QByteArray> Sql::roleNames() const {
    return fieldNames;
}

void Sql::updateFieldNames(QSqlRecord record) {
    fieldNames.clear();
    for( int i = 0; i < record.count(); i ++) {
        fieldNames.insert(Qt::UserRole + i + 1, "sql_" + record.fieldName(i).toUtf8());
    }
}

void Sql::reset() {
    beginResetModel();
    fieldNames.clear();
    results.clear();
    endResetModel();
}

void Sql::reload(){
    setQuery(getQuery());
}

void Sql::dbChanged(const QString& name, const QVariant& payload) {
    if(!reloadTimer->isActive()) {
        reloadTimer->start(100 + (QRandomGenerator::global()->generate() % 100));
    }
}
