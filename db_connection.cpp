#include <db_connection.h>

db_connection::db_connection()
{
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setDatabaseName("test");
    db.setUserName("postgres");
    db.setPassword("11111");
}

int db_connection::check_db_auth()
{
    QSqlQuery query;
    if (!db.tables().contains("auth"))
    {
        QString qu = QString ("CREATE TABLE auth("
                             "id SERIAL PRIMARY KEY,"
                             "login varchar(50) NOT NULL,"
                             "password_hash varchar(256) NOT NULL,"
                             "user_name varchar(50) NOT NULL,"
                             "user_surname varchar(50) NOT NULL,"
                             "dolzh varchar(50) NOT NULL,"
                             "mail varchar(50));");
        if (!query.exec(qu))
        {
            qDebug() << "error create table!" << query.lastError().text();
            return -1;
        }
        else qDebug() << "Таблица 'clients' успешно создана";
    }

    QString queryStr = QString("SELECT * FROM auth;");
    if (!query.exec(queryStr))
    {
        qDebug() << "error query exec! " << query.lastError().text();
        return -1;
    }

    if (query.size() == 0)
    {
        QString password = "12345678";
        QByteArray passwordData = password.toUtf8();
        QByteArray passwordHash = QCryptographicHash::hash(passwordData, QCryptographicHash::Sha256).toHex();
        password = passwordHash;
        queryStr = QString("INSERT INTO auth(login, password_hash, user_name, user_surname, dolzh, mail) VALUES"
                           "('%1', '%2', '%3', '%4', '%5', '%6');").arg("default").arg(password).arg("user").arg("default").arg("start_user")
                       .arg("user");
        if (!query.exec(queryStr))
        {
            qDebug() << "error insert into table! " << query.lastError().text();
            return -1;
        }
        else qDebug() << "start user add success!";
        return 1;
    }
    return 0;
}

bool db_connection::db_open()
{
    return db.open();
}

void db_connection::db_close()
{
    db.close();
}

QJsonArray db_connection::get_list_of_users()
{
    QSqlQuery query;
    if (!query.exec("SELECT * FROM auth;"))
    {
        qDebug() << "erorr: " << query.lastError().text();
        return QJsonArray();
    }

    QJsonArray jsonArray;
    while (query.next())
    {
        QJsonObject jsonObj;
        jsonObj["login"] = query.value(1).toString();
        jsonObj["password_hash"] = query.value(2).toString();
        jsonObj["user_name"] = query.value(3).toString();
        jsonObj["user_surname"] = query.value(4).toString();
        jsonObj["dolzh"] = query.value(5).toString();
        jsonObj["mail"] = query.value(6).toString();

        jsonArray.append(jsonObj);
    }
    return jsonArray;
}

void db_connection::DelFromTable(QString data)
{
    QSqlQuery query;
    QString queryStr = QString("Delete from auth where login = '%1';").arg(data);
    if (!query.exec(queryStr))
    {
        qDebug() << "error: " << query.lastError().text();
        return;
    }

    qDebug() << "Successfully delete!";
}

QString db_connection::get_data_for_auth(QString uname, QString pass)
{
    QSqlQuery query;
    if (!query.exec("SELECT login, password_hash FROM auth"))
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
        if (jsonObject["amount"].toInt() > 0)
            jsonArray.append(jsonObject);
    }

    return jsonArray;
}

bool db_connection::MessageDb(QString mail, QString name, QString surname)
{
    QSqlQuery query;
    if (!db.tables().contains("messages"))
    {
        if (!query.exec("CREATE TABLE messages ("
                        "id serial primary key,"
                        "mail varchar(50),"
                        "name varchar(50),"
                        "surname varchar(50),"
                        "date_message Date,"
                        "time_message Time);"))
        {
            qDebug() << "Ошибка при создании таблицы:" << query.lastError().text();
            return 0;
        }
    }

    QDate currentDate = QDate::currentDate();
    QTime currentTime = QTime::currentTime();

    QString formattedDate = currentDate.toString(Qt::ISODate);
    QString formattedTime = currentTime.toString(Qt::ISODate);

    QString queryStr = QString("INSERT INTO messages (mail, name, surname, date_message, time_message) VALUES"
                               "('%1', '%2', '%3', '%4', '%5');")
                           .arg(mail)
                           .arg(name)
                           .arg(surname)
                           .arg(formattedDate)
                           .arg(formattedTime);

    if (!query.exec(queryStr))
    {
        qDebug() << "Error add: " << query.lastError().text();
        return 0;
    }

    return 1;

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
                    qDebug() << query.lastError().text();    {

                    }
                    return 0;
                }
                QJsonArray jsonArray = jsonDoc.array();    {

                }
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
                    QString name = jsonObj["name"].toString();    {

                    }
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

void db_connection::UpdateUsersTable(QString json)
{
    QSqlQuery query;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json.toUtf8());
    if (!query.exec("DELETE FROM auth;"))
    {
        qDebug() << "error: " << query.lastError().text();
        return;
    }

    QJsonArray jsonArray = jsonDoc.array();
    for (const auto& jsonValue : jsonArray)
    {
        QJsonObject jsonObj = jsonValue.toObject();
        QString login = jsonObj.value("login").toString();
        QString pass = jsonObj.value("password_hash").toString();
        QString user_name = jsonObj.value("user_name").toString();
        QString user_surname = jsonObj.value("user_surname").toString();
        QString dolzh = jsonObj.value("dolzh").toString();
        QString mail = jsonObj.value("mail").toString();
        if (login == "mm")
            qDebug() << pass;
        QString queryStr = QString("INSERT INTO auth (login, password_hash, user_name, user_surname, dolzh, mail) VALUES"
                               "('%1', '%2', '%3', '%4', '%5', '%6')").arg(login).arg(pass).arg(user_name).arg(user_surname)
                           .arg(dolzh).arg(mail);

        if (!query.exec(queryStr))
        {
            qDebug() << "error: " << query.lastError().text();
            continue;
        }

        qDebug() << "add";
    }

}

QJsonArray db_connection::getOrders()
{
    QSqlQuery query;
    if (!query.exec("SELECT * FROM orders"))
    {
        qDebug() << "error: " << query.lastError().text();
        return QJsonArray();
    }

    QJsonArray jsonArray;
    while (query.next())
    {
        QJsonObject jsonObj;
        jsonObj["item_name"] = query.value(1).toString();
        jsonObj["person_name"] = query.value(2).toString();
        jsonObj["person_surname"] = query.value(3).toString();
        jsonObj["price"] = query.value(4).toDouble();
        jsonObj["date_order"] = query.value(5).toString();
        jsonArray.append(jsonObj);
    }
    return jsonArray;
}

QJsonArray db_connection::getMessages()
{
    QSqlQuery query;
    if (!query.exec("SELECT * FROM messages"))
    {
        qDebug() << "error: " << query.lastError().text();
        return QJsonArray();
    }

    QJsonArray jsonArray;
    while (query.next())
    {
        QJsonObject jsonObj;
        jsonObj["mail"] = query.value(1).toString();
        jsonObj["name"] = query.value(2).toString();
        jsonObj["surname"] = query.value(3).toString();
        jsonObj["date"] = query.value(4).toString();
        jsonObj["time"] = query.value(5).toString();

        jsonArray.append(jsonObj);
    }

    return jsonArray;
}

void db_connection::AddInDbUsers(QVector<QString> vector)
{
    QSqlQuery query;
    if (!db.tables().contains("auth"))
    {
        qDebug() << "error: " << query.lastError().text();
        return;
    }

    QString queryStr = QString("INSERT INTO auth (login, password_hash, user_name, user_surname, dolzh, mail) VALUES"
                               "('%1', '%2', '%3', '%4', '%5', '%6')").arg(vector[4]).arg(vector[5]).arg(vector[1]).arg(vector[2])
                           .arg(vector[3]).arg(vector[0]);

    if (!query.exec(queryStr))
    {
        qDebug() << "error: " << query.lastError().text();
        return;
    }
}


