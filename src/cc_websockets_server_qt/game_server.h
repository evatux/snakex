#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include <QtCore/QObject>
#include <QtCore/QList>

#include <cassert>
#include <map>
#include <memory>

#include "cc_core/game.hpp"

QT_FORWARD_DECLARE_CLASS(QTimer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class GameServer : public QObject {
    Q_OBJECT
public:
    explicit GameServer(const QList<QWebSocket *> &clientList, bool debug = false);
    ~GameServer();

Q_SIGNALS:
    void closed();

private Q_SLOTS:
    void processTextMessage(QString qstr);
    void makeStep();
    void socketDisconnected();

private:
    bool debug_;

    int nplayers_;
    core::pos_t wsize_;

    std::map<QWebSocket *, int> clientToId_;
    std::map<int, QWebSocket *> idToClient_;

    std::unique_ptr<QTimer> timer_;
    std::unique_ptr<core::game_t> game_;

    // Client QWebSocket <-> id mapping
    int clientToId(QWebSocket *client) const { return clientToId_.at(client); }
    int senderToId(QObject *sender) const {
        return clientToId((QWebSocket *)sender);
    }
    QWebSocket *idToClient(int id) const { return idToClient_.at(id); }

    // Sending messages
    void sendMessage(int id, const proto::message_t &message) const;
    void broadcastMessage(const proto::message_t &message) const {
        sendMessage(-1, message);
    }

    // Check or set clients' readiness
    unsigned clientReadiness_ = 0;
    bool isClientReady(int id) const { return clientReadiness_ & (1U << id); }
    bool allClientsReady() const {
        return clientReadiness_ + 1 == 1U << nplayers_;
    }
    void setClientReady(int id) {
        assert(!isClientReady(id));
        clientReadiness_ |= 1U << id;
    }

    // Client initialization and finalization
    void sendInitToClient(int id) const;
    void receiveInitFromClient(int id, const proto::message_t &message);
    void processClientMessage(int id, const proto::message_t &message);

    // Game start and stop
    void startGame();
    void stopGame();
};

#endif // GAME_SERVER_H
