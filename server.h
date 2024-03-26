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
    int id_p = 1;
    void SendItemsForClient(QString str);

    void select_role(QString us_name, QString us_pass);


public slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead();


};

#endif // SERVER_H
