#ifndef SERVER_H
#define SERVER_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QTime>
#include <QString>
#include <QDataStream>
#include <QMap>

#include <db_connection.h>

class Server : public QTcpServer
{
    Q_OBJECT

public:
    Server();
    QTcpSocket *socket;


private:
    QVector<QTcpSocket*> sockets;
    QByteArray Data;
    void SendToClient(QString str);
    quint16 nextBlockSize;
    QVector<QTcpSocket*> Sockets;

    db_connection db;
    void SendItemsForClient(QString str);
    //QString db_auth(QString uname, QByteArray pass);



public slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead();


};

#endif // SERVER_H
