#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <QtCore/QTimer>

#include <map>
#include <memory>

#include "cc_core/game.hpp"

QT_FORWARD_DECLARE_CLASS(QWebSocket)

class GameServer : public QObject {
    Q_OBJECT
public:
    explicit GameServer(const QList<QWebSocket *> &clientList);
    ~GameServer();

Q_SIGNALS:
    void closed();

private Q_SLOTS:
    void processTextMessage(QString message);
    void startGame();
    void makeStep();
    void socketDisconnected();

private:
    core::pos_t wsize_;

    std::map<QWebSocket *, int> clientToId_;
    std::map<int, QWebSocket *> idToClient_;

    std::unique_ptr<QTimer> timer_;
    std::unique_ptr<core::game_t> game_;

    void sendMessage(const proto::message_t &message);
    int clientToId(QWebSocket *client) const { return clientToId_[client]; }
    int senderToId(QObject *sender) const { return clientToId((QWebSocket *)sender); }
};

#endif // GAME_SERVER_H
