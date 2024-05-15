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
#include <QDate>
#include <QTime>


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
    int check_db_auth();
    bool db_open();
    void db_close();
    void editDbOfOrder(double pr, QString, QString, QString);
    QString get_data_for_auth(QString uname, QString pass);
    QJsonArray get_data_of_items();
    QJsonArray get_data_of_clients();
    // void add_client(QString name, QString mail, QString number);
    bool ChangeInDb(QString, int);
    bool MessageDb(QString, QString, QString);
    QJsonArray getOrders();
    QJsonArray getMessages();
    void AddInDbUsers(QVector<QString>);
    QJsonArray get_list_of_users();
    void DelFromTable(QString);
    void UpdateUsersTable(QString);

    int change_pass(QJsonObject);



    int reset_data(QString name, QString sname, QString login, QString mail);


};

#endif // DB_CONNECTION_H
