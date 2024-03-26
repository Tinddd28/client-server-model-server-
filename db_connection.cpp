#include <db_connection.h>

db_connection::db_connection()
{
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setDatabaseName("test");
    db.setUserName("postgres");
    db.setPassword("11111");
}

bool db_connection::db_open()
{
    return db.open();
}

QString db_connection::get_data_for_auth(QString uname, QString pass)
{
    QSqlQuery query;
    if (!query.exec("SELECT username, password_hash FROM users"))
    {
        qDebug() << "Ошибка выполнения запроса!";
        return "error";
    }
    int flag = 1;
    while (query.next())
    {
        QString username = query.value(0).toString();
        QString password_hash = query.value(1).toString();
        if (username == uname && password_hash == pass) return uname;
        else flag = 0;
    }
    if (flag == 0) return "No";
}


QJsonArray db_connection::get_data_of_items()
{
    QSqlQuery query;
    QString queryStr = QString("SELECT * FROM items");
    if (!query.exec(queryStr))
    {
        qDebug() << "Ошибка выполнения запроса!";
        return QJsonArray();
    }

    QJsonArray jsonArray;
    while (query.next())
    {
        QJsonObject jsonObject;
        jsonObject["item_name"] = query.value(1).toString();
        jsonObject["price"] = query.value(2).toDouble();
        jsonObject["type"] = query.value(3).toString();
        jsonObject["amount"] = query.value(4).toInt();
        jsonArray.append(jsonObject);
    }

    return jsonArray;
}

