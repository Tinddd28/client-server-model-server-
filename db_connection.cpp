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
    if (!db.tables().contains("items"))
    {
        QSqlQuery query;
        if (!query.exec("CREATE TABLE items"
                        "(id SERIAL PRIMARY KEY,"
                        "item_name VARCHAR(50) NOT NULL,"
                        "price float NOT NULL,"
                        "type VARCHAR(50) ,"
                        "amount int);")) {
            qDebug() << "Ошибка при создании таблицы:" << query.lastError().text();
            return QJsonArray();
        }
        qDebug() << "Таблица 'clients' успешно создана";
    }

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

QJsonArray db_connection::get_data_of_clients()
{
    if (!db.tables().contains("clients"))
    {
        QSqlQuery query;
        if (!query.exec("CREATE TABLE clients"
                        "(id SERIAL PRIMARY KEY,"
                        "name VARCHAR(50),"
                        "surname VARCHAR(50),"
                        "mail VARCHAR(50),"
                        "phone VARCHAR(11));")) {
            qDebug() << "Ошибка при создании таблицы:" << query.lastError().text();
            return QJsonArray();
        }
        qDebug() << "Таблица 'clients' успешно создана";
    }

    QSqlQuery query;
    if (!query.exec("SELECT * FROM clients;"))
    {
        qDebug() << "Ошибка!";
        return QJsonArray();
    }

    QJsonArray jsonArray;
    while (query.next())
    {
        QJsonObject jsonObj;
        jsonObj["name"] = query.value(1).toString();
        jsonObj["surname"] = query.value(2).toString();
        jsonObj["mail"] = query.value(3).toString();
        jsonObj["phone"] = query.value(4).toString();
        jsonArray.append(jsonObj);
    }
    return jsonArray;
}

bool db_connection::ChangeInDb(QString jsonString, int flag)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "Ошибка при парсинге JSON:" << error.errorString();
        return 0;
    }
    if (jsonDoc.array().isEmpty() || jsonDoc.isEmpty()) {
        return 0;
    }
    else
    {   if (db.open())
        {
            qDebug() << "all ok!";
            if (flag == 1) // json of items;
            {
                QJsonArray jsonArray = jsonDoc.array();

                for (const auto& jsonValue : jsonArray)
                {
                    QJsonObject jsonObj = jsonValue.toObject();
                    QString itemName = jsonObj["item_name"].toString();
                    float price = jsonObj["price"].toDouble();
                    QString type = jsonObj["type"].toString();
                    int amount = jsonObj["amount"].toInt();
                    QString queryStr = QString("UPDATE items SET amount = '%1', price = '%2' WHERE item_name = '%3'")
                                           .arg(amount).arg(price).arg(itemName);

                    if (!query.exec(queryStr)) {
                        qDebug() << "Error executing SQL query:" << query.lastError().text();
                        return 0;
                    }
                }
            }
            else // json of clients
            {
                QJsonArray jsonArray = jsonDoc.array();

                for (const auto& jsonValue : jsonArray)
                {
                    QJsonObject jsonObj = jsonValue.toObject();
                    QString name = jsonObj["name"].toString();
                    QString surname = jsonObj["surname"].toString();
                    QString mail = jsonObj["mail"].toString();
                    QString phone = jsonObj["phone"].toString();
                    QString queryStr = QString("UPDATE clients SET mail = '%1', phone = '%2' WHERE name = '%3' AND surname = '%4'")
                                           .arg(mail).arg(phone).arg(name).arg(surname);

                    if (!query.exec(queryStr)) {
                        qDebug() << "Error executing SQL query:" << query.lastError().text();
                        return 0;
                    }
                }
            }

            return 1;
        }
        else
        {
            qDebug() << "db not open!";
            return 0;
        }
    }
}




