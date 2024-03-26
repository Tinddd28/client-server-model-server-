#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H
#include <QSqlDatabase>
#include <QDebug>
#include <QCryptographicHash>
#include <QSqlQuery>
#include <QByteArray>
#include <QSqlError>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <db_connection.h>

class db_connection
{
private:
    QString db_name;
    QString user_name;
    QString pass;
    QString db_host;
    QSqlQuery query;
    QSqlError error;
public:
    db_connection();
    QSqlDatabase db;
    bool db_open();
    QString get_data_for_auth(QString uname, QByteArray pass);
    QJsonArray get_data_of_items();
    // void get_data_of_clients();
    // void add_client(QString name, QString mail, QString number);



};

#endif // DB_CONNECTION_H
