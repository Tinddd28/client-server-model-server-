#include <server.h>

Server::Server()
{
    if (this->listen(QHostAddress::Any, 2323))
        qDebug() << "start";
    else
        qDebug() << "error";
    if (db.db_open())
    {
        qDebug() << "connect to db";
    }
    else qDebug() << "error connect to db";
    int f = db.check_db_auth();
    if (f < 0)
    {
        this->close();
    }
    //Проверка существовани
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, &QTcpSocket::readyRead, this, [this, socket]()
    {
        QByteArray requestData = socket->readAll();
        QList<QByteArray> requests = requestData.split('#');

        for (const QByteArray& request : requests)
        {
            if (request.isEmpty()) {
                continue;
            }

            slotReadyRead(socket, request);
        }
        //qDebug() << "server read a data";
    });

    connect(socket, &QTcpSocket::disconnected, this, [this, socket]()
    {
        connections.remove(socket->socketDescriptor());
        socket->deleteLater();
        //qDebug() << "clients disconnected";
    });

    connections.insert(socketDescriptor, socket);
}

void Server::sendResultOfRequestReset(QTcpSocket* socket, int flag)
{
    QJsonObject jsonObj;
    jsonObj.insert("window", "mainwindow");
    jsonObj.insert("action", "reset");
    jsonObj.insert("flag", flag);
    QJsonDocument jsonDoc(jsonObj);
    socket->write(jsonDoc.toJson());
}

void Server::send_res_of_update(QTcpSocket* socket, int flag)
{
    QJsonObject jsonObj;
    jsonObj.insert("window", "mainwindow");
    jsonObj.insert("action", "result");
    jsonObj.insert("result", flag);

    QJsonDocument jsonDoc(jsonObj);
    socket->write(jsonDoc.toJson());
}

void Server::slotReadyRead(QTcpSocket* socket, const QByteArray& request)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(request);
    QJsonObject jsonObj = jsonDoc.object();


    QString window = jsonObj.value("window").toString();
    if (window == "mainwindow")
    {
        QString action = jsonObj.value("action").toString();
        if (action == "login")
        {
            QJsonObject data = jsonObj.value("data").toObject();
            QString us_name = data.value("us_name").toString();
            QString us_pass = data.value("us_pass").toString();
            select_role(socket, us_name, us_pass);
        }
        else if (action == "reset")
        {
            QJsonObject data = jsonObj.value("data").toObject();
            QString us_name = data.value("name").toString();
            QString us_sname = data.value("sname").toString();
            QString login = data.value("login").toString();
            QString mail = data.value("mail").toString();
            int flag = db.reset_data(us_name, us_sname, login, mail);
            sendResultOfRequestReset(socket, flag);
        }
        else if (action == "change")
        {
            QJsonObject data = jsonObj.value("data").toObject();
            int flag = db.change_pass(data);
            send_res_of_update(socket, flag);

        }
    }
    else if (window == "salesmanager")
    {
        QString action = jsonObj.value("action").toString();
        if (action == "data")
        {
            QString data = jsonObj.value("data").toString();
            if (data == "items")
            {
                SendItems(socket, window, action, data);
            }
            else if (data == "clients")
            {
                SendListOfClients(socket, window, action, data);
            }
            else if (data == "order")
            {
                SendInfoAboutOrder(socket);
            }
            else if (data == "changes")
            {
                QJsonObject changes = jsonObj.value("changes").toObject();
                QString itemsString = changes["items"].toString();
                QString clientsString = changes["clients"].toString();
                double price = changes["price"].toDouble();
                QString name = changes["name"].toString();
                QString surname = changes["surname"].toString();
                QString item = changes["item"].toString();
                if (db.ChangeInDb(itemsString, 1))
                    qDebug() << "success";
                else return;

                if (db.ChangeInDb(clientsString, 2)) qDebug() << "success";
                else return;
                db.editDbOfOrder(price, name, surname, item);
            }
        }
    }
    else if (window == "markmanager")
    {
        QString action = jsonObj.value("action").toString();
        if (action == "set_sale")
        {
            SendItemsForSale(socket);
        }
        else if (action == "message")
        {
            SendClients(socket);
        }
        else if (action == "send_sale")
        {
            QString data = jsonObj.value("items").toString();
            if (db.ChangeInDb(data, 1))
                qDebug() << "success!";
        }
        else if (action == "send")
        {
            QString dataString = jsonObj.value("data").toString();
            QByteArray data = QByteArray::fromBase64(dataString.toLatin1());
            QVector<Server::ForMes> desVector = deserializeVector(data);
            for (int i = 0; i < desVector.size(); i++)
            {
                ForMes fm = desVector[i];
                if (SendMail(fm.mail, fm.message))
                {
                    if (db.MessageDb(fm.mail, fm.name, fm.surname)) qDebug() << "db add message";
                    else qDebug() << "error message";
                }
                else qDebug() << "Message didnt sent!";

            }
        }
    }
    else if (window == "dataout")
    {
        QString action = jsonObj.value("action").toString();
        if (action == "data")
        {
            QString data = jsonObj.value("data").toString();
            if (data == "items")
            {
                SendItems(socket, window, action, data);
            }
            else if (data == "clients")
            {
                SendListOfClients(socket, window, action, data);
            }
            else if (data == "orders")
            {
                SendOrders(socket);
            }
            else if (data == "messages")
            {
                SendMessages(socket);
            }
            else if (data == "profit")
            {
                SendProfit(socket);
            }
            else if (data == "sales")
            {
                SendSales(socket);
            }
        }
    }
    else if (window == "director")
    {
        QString action = jsonObj.value("action").toString();
        {
            if (action == "add")
            {jsonObj.insert("window", "salesmanager");
                jsonObj.insert("action", "data");
                jsonObj.insert("data", "items");
                QString data = jsonObj.value("add").toString();
                QByteArray d = QByteArray::fromBase64(data.toLatin1());
                QVector<QString> vector = deseriale(d);
                qDebug() << vector[0];
                qDebug() << vector[5];
                db.AddInDbUsers(vector);
            }
            else if (action == "edituser")
            {
                SendUsers(socket, action);
            }
            else if (action == "adduser")
            {
                SendUsers(socket, action);
            }
            else if (action == "deluser")
            {
                SendUsers(socket, action);
            }
            else if (action == "del")
            {
                QString data = jsonObj.value("user").toString();
                qDebug() << data;
                db.DelFromTable(data);
            }
            else if (action == "edit")
            {
                QString data = jsonObj.value("users").toString();
                db.UpdateUsersTable(data);
            }
            else if (action == "view")
            {
                SendUsers(socket, action);
            }
        }
    }
}

void Server::SendUsers(QTcpSocket *socket, QString action)
{
    QJsonDocument jsonData = QJsonDocument(db.get_list_of_users());
    QString res = jsonData.toJson();
    QJsonObject jsonObj;
    jsonObj.insert("window", "director");
    jsonObj.insert("action", action);
    jsonObj.insert("users", res);

    QJsonDocument jsonDoc(jsonObj);
    socket->write(jsonDoc.toJson());
}

QVector<QString> Server::deseriale(QByteArray byteArray)
{
    QVector<QString> vector;
    QDataStream stream(byteArray);
    QVariantList list;
    stream >> list;

    // Преобразование QVariantList обратно в QVector<QString>
    for (const QVariant& variant : list) {
        vector.append(variant.toString());
    }
    return vector;
}

void Server::SendOrders(QTcpSocket *socket)
{
    QJsonDocument jsonData = QJsonDocument(db.getOrders());
    QString res = jsonData.toJson();
    QJsonObject jsonObj;
    jsonObj.insert("window", "dataout");
    jsonObj.insert("action", "data");
    jsonObj.insert("data", "orders");
    jsonObj.insert("orders", res);

    QJsonDocument jsonDoc(jsonObj);
    socket->write(jsonDoc.toJson());
}

void Server::SendMessages(QTcpSocket *socket)
{
    QJsonDocument jsonData = QJsonDocument(db.getMessages());
    QString res = jsonData.toJson();
    QJsonObject jsonObj;
    jsonObj.insert("window", "dataout");
    jsonObj.insert("action", "data");
    jsonObj.insert("data", "messages");
    jsonObj.insert("messages", res);

    QJsonDocument jsonDoc(jsonObj);
    socket->write(jsonDoc.toJson());
}

void Server::SendSales(QTcpSocket *socket)
{

}

void Server::SendProfit(QTcpSocket *socket)
{

}

void Server::SendItems(QTcpSocket *socket, QString window, QString action, QString data)
{
    QJsonDocument jsonData = QJsonDocument(db.get_data_of_items());
    QString res = jsonData.toJson();
    QJsonObject jsonObj;
    jsonObj.insert("window", window);
    jsonObj.insert("action", action);
    jsonObj.insert("data", data);
    jsonObj.insert(data, res);

    QJsonDocument jsonDoc(jsonObj);
    socket->write(jsonDoc.toJson());
}

bool Server::SendMail(QString mail, QString message)
{
    QStringList args;
    args << "send.py" << mail << message;
    QProcess process;
    process.start("python", args);
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput();
    QByteArray error = process.readAllStandardError();


    if (!error.isEmpty()) {
        qDebug() << error;
        return 0;
    } else {
        qDebug() << output;
        return 1;
    }
}

void Server::SendClients(QTcpSocket *socket)
{
    QJsonDocument jsonData = QJsonDocument(db.get_data_of_clients());
    QString res = jsonData.toJson();
    QJsonObject jsonObj;
    jsonObj.insert("window", "markmanager");
    jsonObj.insert("action", "message");
    jsonObj.insert("clients", res);

    QJsonDocument jsonDoc = QJsonDocument(jsonObj);
    socket->write(jsonDoc.toJson());
}

void Server::SendItemsForSale(QTcpSocket* socket)
{
    QJsonDocument jsonData = QJsonDocument(db.get_data_of_items());
    QString res = jsonData.toJson();
    QJsonObject jsonObj;
    jsonObj.insert("window", "markmanager");
    jsonObj.insert("action", "set_sale");
    jsonObj.insert("items", res);

    QJsonDocument jsonDoc(jsonObj);
    socket->write(jsonDoc.toJson());
}

void Server::select_role(QTcpSocket* socket, QString us_name, QString us_pass)
{
    QString res = db.get_data_for_auth(us_name, us_pass);
    QString status = "success";
    if (res == "director")
        id_p = 1;
    else if (res == "salesmanager") id_p = 2;
    else if (res == "marketingmanager") id_p = 3;
    else if (res == "default") id_p = 1;
    else if (res == "No") status = "fail";
    QJsonObject jsonObj;
    jsonObj.insert("window", "mainwindow");
    jsonObj.insert("action", "login");
    jsonObj.insert("status", status);
    jsonObj.insert("id", id_p);

    QJsonDocument jsonDoc(jsonObj);
    socket->write(jsonDoc.toJson());
}

void Server::SendItemsForClient(QTcpSocket *socket)
{
    QJsonDocument jsonData = QJsonDocument(db.get_data_of_items());
    QString jsonString = jsonData.toJson();
    QJsonObject jsonObj;
    jsonObj.insert("window", "salesmanager");
    jsonObj.insert("action", "data");
    jsonObj.insert("data", "items");
    jsonObj.insert("items", jsonString);

    QJsonDocument jsonDoc(jsonObj);
    socket->write(jsonDoc.toJson());
}

void Server::SendListOfClients(QTcpSocket* socket, QString window, QString action, QString data)
{
    QJsonDocument jsonData = QJsonDocument(db.get_data_of_clients());
    QString jsonString = jsonData.toJson();

    QJsonObject jsonObj;
    jsonObj.insert("window", window);
    jsonObj.insert("action", action);
    jsonObj.insert("data", data);
    jsonObj.insert(data, jsonString);

    QJsonDocument jsonDoc(jsonObj);
    socket->write(jsonDoc.toJson());
}

void Server::SendInfoAboutOrder(QTcpSocket* socket)
{
    QJsonDocument jsonData = QJsonDocument(db.get_data_of_items());
    QString jsonItems = jsonData.toJson();
    jsonData = QJsonDocument(db.get_data_of_clients());
    QString jsonClients = jsonData.toJson();

    QJsonObject jsonObj;
    jsonObj.insert("window", "salesmanager");
    jsonObj.insert("action", "data");
    jsonObj.insert("data", "order");
    QJsonObject jsonD;
    jsonD.insert("order_items", jsonItems);
    jsonD.insert("order_clients", jsonClients);
    jsonObj.insert("order", jsonD);

    QJsonDocument jsonDoc(jsonObj);
    socket->write(jsonDoc.toJson());
}

