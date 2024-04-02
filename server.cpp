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
    }
    else if (window == "salesmanager")
    {
        QString action = jsonObj.value("action").toString();
        if (action == "data")
        {
            QString data = jsonObj.value("data").toString();
            if (data == "items")
            {
                SendItemsForClient(socket);
            }
            else if (data == "clients")
            {
                SendListOfClients(socket);
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
}

bool Server::SendMail(QString mail, QString message)
{
    QStringList args;
    args << "/home/kali/Qt/projects/send.py" << mail << message;
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

void Server::SendListOfClients(QTcpSocket* socket)
{
    QJsonDocument jsonData = QJsonDocument(db.get_data_of_clients());
    QString jsonString = jsonData.toJson();

    QJsonObject jsonObj;
    jsonObj.insert("window", "salesmanager");
    jsonObj.insert("action", "data");
    jsonObj.insert("data", "clients");
    jsonObj.insert("clients", jsonString);

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

