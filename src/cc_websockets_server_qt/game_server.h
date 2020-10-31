#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <QtCore/QTimer>

#include <memory>

#include "cc_core/game.hpp"

QT_FORWARD_DECLARE_CLASS(QWebSocket)

class GameServer : public QObject
{
    Q_OBJECT
public:
    explicit GameServer(QWebSocket *pSocket);
    ~GameServer();

    QWebSocket *socket() const { return pSocket; }

Q_SIGNALS:
    void closed();

private Q_SLOTS:
    void processTextMessage(QString message);
    void startGame();
    void makeStep();
    void socketDisconnected();

private:
    QWebSocket *pSocket;
    std::unique_ptr<QTimer> timer_;
    std::unique_ptr<core::game_t> game_;

    void sendMessage(const proto::message_t &message);
};

#endif // GAME_SERVER_H
