#include <QCoreApplication>
#include <server.h>
#include <QCryptographicHash>
#include <QSqlQuery>
#include <QByteArray>
#include <QSqlError>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Server s;



    return a.exec();
}
