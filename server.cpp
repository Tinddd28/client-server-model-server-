#include <server.h>

Server::Server()
{
    if (this->listen(QHostAddress::Any, 2323))
        qDebug() << "start";
    else
        qDebug() << "error";
    nextBlockSize = 0;
    if (db.db_open())
    {
        qDebug() << "connect to db";
    }
    else qDebug() << "error connect to db";
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    socket = new QTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
    //connect(socket, &QTcpSocket::disconnected, socket, &Server::deleteLater);

    qDebug() << "connect user " << socketDescriptor;
}

void Server::slotReadyRead()
{
    QByteArray responseData = socket->readAll();
    QList<QByteArray> responses = responseData.split('#');

    for (const QByteArray& response : responses)
    {
        if (response.isEmpty()) continue;

        QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
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
                select_role(us_name, us_pass);
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
                    SendItemsForClient();
                }
            }
        }
    }

}

void Server::select_role(QString us_name, QString us_pass)
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

void Server::SendItemsForClient()
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



