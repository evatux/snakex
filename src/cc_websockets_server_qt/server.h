#ifndef SERVER_H
#define SERVER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>

#include "game_server.h"

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)

class Server : public QObject {
    Q_OBJECT
public:
    explicit Server(quint16 port, int nplayers, bool debug = false, QObject *parent = nullptr);
    ~Server();

Q_SIGNALS:
    void closed();

private Q_SLOTS:
    void onNewConnection();

private:
    QWebSocketServer *pWebSocketServer_;
    QList<QWebSocket *> clientList_;

    int nplayers_;
    bool debug_;
};

#endif // SERVER_H
