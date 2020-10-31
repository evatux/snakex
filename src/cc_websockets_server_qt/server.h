#ifndef SERVER_H
#define SERVER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>

#include "game_server.h"

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(quint16 port, bool debug = false, QObject *parent = nullptr);
    ~Server();

Q_SIGNALS:
    void closed();

private Q_SLOTS:
    void onNewConnection();
    // void socketDisconnected();

private:
    QWebSocketServer *m_pWebSocketServer;
    QList<GameServer *> m_clients;
    bool m_debug;
};

#endif // SERVER_H
