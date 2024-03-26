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
    connect(socket, &QTcpSocket::disconnected, socket, &Server::deleteLater);

    qDebug() << "connect user " << socketDescriptor;
}

void Server::slotReadyRead()
{
    socket = (QTcpSocket*)sender();
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_14);
    if (in.status() == QDataStream::Ok)
    {
        qDebug() << "read...";
        for (;;)
        {
            if (nextBlockSize == 0)
            {
                if (socket->bytesAvailable() < 2) break;
                in >> nextBlockSize;
            }
            if (socket->bytesAvailable() < nextBlockSize) break;

            int flagOfData;
            in >> flagOfData;
            if (flagOfData == 1) //login
            {
                QString us_name;
                QByteArray us_pass;
                in >> us_name >> us_pass;
                QString res_auth = db.get_data_for_auth(us_name, us_pass);

                qDebug() << res_auth;
                if (res_auth == "director")
                    SendToClient("director");
                if (res_auth == "salesmanager")
                    SendToClient("salesmanager");
                if (res_auth == "marketingmanager")
                    SendToClient("marketingmanager");

            }
            else if (flagOfData == 2) //data
            {
                QJsonDocument jsonData = QJsonDocument(db.get_data_of_items());
                QString jsonString = jsonData.toJson();
                SendItemsForClient(jsonString);
                //qDebug() << jsonString;
            }
            nextBlockSize = 0;

            break;
        }
    }
    else qDebug() << "DataStream error";
}

void Server::SendToClient(QString str) // for authorization
{
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_14);
    out << quint16(0) << 1 << str;
    out.device()->seek(0);
    out << quint16(Data.size() - sizeof(quint16));
    socket->write(Data);

}

void Server::SendItemsForClient(QString str) // for send items
{
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_14);
    out << quint16(0) << 2 << str;
    out.device()->seek(0);
    out << quint16(Data.size() - sizeof(quint16));
    socket->write(Data);

}
