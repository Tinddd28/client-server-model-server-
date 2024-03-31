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

void db_connection::db_close()
{
    db.close();
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

void db_connection::editDbOfOrder(double price, QString name, QString surname, QString item)
{
    QSqlQuery query;
    if (!db.tables().contains("orders"))
    {
        QString queryStr = QString("CREATE TABLE orders"
                                   "(id serial primary key,"
                                   "item_name varchar(50),"
                                   "person_name varchar(30),"
                                   "person_surname varchar(30),"
                                   "price float,"
                                   "date_order Date);");
        if (!query.exec(queryStr))
        {
            qDebug() << "unlucky";
            return;
        }
    }

    QString queryStr = QString("INSERT INTO orders (item_name, person_name, person_surname, price, date_order) VALUES "
                               "('%1', '%2', '%3', %4, '%5')")
                           .arg(item)
                           .arg(name)
                           .arg(surname)
                           .arg(price)  // price уже является числом, поэтому его можно передать напрямую
                           .arg(QDate::currentDate().toString(Qt::ISODate));
    if (!query.exec(queryStr))
    {
        qDebug() << "error!";
        return;
    }

    qDebug() << "successfull add";
}

bool db_connection::ChangeInDb(QString jsonString, int flag)
{
    QJsonParseError error;
    QSqlQuery query;
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
            if (flag == 1) // json of items;
            {
                if (!query.exec("DELETE FROM items;"))
                {
                    qDebug() << "Failed to clear table:";
                    qDebug() << query.lastError().text();
                    return 0;
                }
                QJsonArray jsonArray = jsonDoc.array();
                for (const auto& jsonValue : jsonArray)
                {
                    QJsonObject jsonObj = jsonValue.toObject();
                    QString item_name = jsonObj["item_name"].toString();
                    double price = jsonObj["price"].toDouble();
                    QString type = jsonObj["type"].toString();
                    int amount = jsonObj["amount"].toInt();
                    QString queryS = QString("INSERT INTO items (item_name, price, type, amount) VALUES ('" + item_name + "', '" + QString::number(price) + "', '" + type + "', '"
                                             + QString::number(amount) + "');");
                    if (!query.exec(queryS))
                    {
                        qDebug() << "ai-ai-ai" << query.lastError().text();
                        return 0;
                    }

                }
            }
            else // json of clients
            {
                if (!query.exec("DELETE FROM clients;"))
                {
                    qDebug() << "Failed to clear table:";
                    qDebug() << query.lastError().text();
                    return 0;
                }
                QJsonArray jsonArray = jsonDoc.array();
                for (const auto& jsonValue : jsonArray)
                {
                    QJsonObject jsonObj = jsonValue.toObject();
                    QString name = jsonObj["name"].toString();
                    QString surname = jsonObj["surname"].toString();
                    QString mail = jsonObj["mail"].toString();
                    QString phone = jsonObj["phone"].toString();
                    QString queryS = QString("INSERT INTO clients (name, surname, mail, phone) VALUES ('" + name + "', '" + surname+ "', '" + mail + "', '"
                                             + phone + "');");
                    if (!query.exec(queryS))
                    {
                        qDebug() << "ai-ai-ai" << query.lastError().text();
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




