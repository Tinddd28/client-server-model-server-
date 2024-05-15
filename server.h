#ifndef SERVER_H
#define SERVER_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QTime>
#include <QString>
#include <QDataStream>
#include <QMap>
#include <QProcess>
#include <QVariantList>

#include <db_connection.h>

class Server : public QTcpServer
{
    Q_OBJECT

public:
    Server();

private:
    QVector<QTcpSocket*> sockets;

    struct ForMes
    {
        QString mail;
        QString name;
        QString surname;
        QString message;

        // Метод сериализации структуры ForMes в QByteArray
        QByteArray serialize() const
        {
            QByteArray byteArray;
            QDataStream stream(&byteArray, QIODevice::WriteOnly);
            stream << mail << name << surname << message;
            return byteArray;
        }

        // Статический метод десериализации QByteArray в структуру ForMes
        static ForMes deserialize(const QByteArray& byteArray)
        {
            ForMes fm;
            QDataStream stream(byteArray);
            stream >> fm.mail >> fm.name >> fm.surname >> fm.message;
            return fm;
        }
    };

    QVector<ForMes> deserializeVector(const QByteArray& byteArray)
    {
        QVector<ForMes> vector;
        QDataStream stream(byteArray);

        int size;
        stream >> size; // Получаем размер вектора

        // Десериализуем каждую структуру в векторе
        for (int i = 0; i < size; ++i) {
            ForMes item;
            // Считываем данные каждой структуры из потока
            stream >> item.mail >> item.name >> item.surname >> item.message;
            vector.append(item);
        }

        return vector;
    }
    db_connection db;
    int id_p = 1;
    void SendItemsForClient(QTcpSocket*);
    void SendListOfClients(QTcpSocket*, QString, QString, QString);
    void SendInfoAboutOrder(QTcpSocket*);
    void SendItemsForSale(QTcpSocket*);
    void SendItemsAndClients(QTcpSocket*);
    void SendClients(QTcpSocket*);

    void SendItems(QTcpSocket*, QString window, QString action, QString data);
    void SendOrders(QTcpSocket*);
    void SendMessages(QTcpSocket*);
    void SendSales(QTcpSocket*);
    void SendProfit(QTcpSocket*);
    void SendUsers(QTcpSocket*, QString);
    QVector<QString> deseriale(QByteArray);


    void select_role(QTcpSocket*, QString us_name, QString us_pass);
    QMap<quintptr, QTcpSocket*> connections;
    bool SendMail(QString, QString);

    void sendResultOfRequestReset(QTcpSocket*, int flag);
    void send_res_of_update(QTcpSocket*, int flag);

public slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead(QTcpSocket*, const QByteArray&);
};

#endif // SERVER_H
