#include <QtWebSockets/qwebsocket.h>
#include <QtCore/QDebug>
#include <QtCore/QTimer>

#include "cc_core/game.hpp"
#include "cc_core/proto.hpp"

#include "game_server.h"

QT_USE_NAMESPACE

GameServer::GameServer(const QList<QWebSocket *> &clientList, bool debug)
    : debug_(debug) {
    nplayers_ = clientList.count();
    wsize_ = {30, 30};

    int id = 0;
    for (auto client: clientList) {
        clientToId_[client] = id;
        idToClient_[id] = client;

        connect(client, &QWebSocket::textMessageReceived,
                this, &GameServer::processTextMessage);
        connect(client, &QWebSocket::disconnected,
                this, &GameServer::socketDisconnected);

        sendInitToClient(id);

        ++id;
    }

    game_.reset(new core::game_t({30, 30}, nplayers_));
    timer_.reset(new QTimer(this));
    connect(timer_.get(), &QTimer::timeout, this, &GameServer::makeStep);
}

GameServer::~GameServer() {
    socketDisconnected();
}

void GameServer::sendInitToClient(int id) const {
    proto::message_t message;
    message.emplace_back(proto::id_t{id});
    message.emplace_back(proto::setup_t{wsize_.x, wsize_.y});
    sendMessage(id, message);
}

void GameServer::receiveInitFromClient(int id, const proto::message_t &message)
{
    if (message.size() != 1) {
        qDebug() << "EEE receiveInitFromClient(" << id << "): "
            << "message length (" << message.size() << ") != 1";
        stopGame();
    }

    const auto &e = message[0];
    if (std::holds_alternative<proto::id_t>(e)) {
        const auto &v = std::get<proto::id_t>(e);
        if (v.id != id) {
            qDebug() << "EEE receiveInitFromClient(" << id << "): "
                << "client returned " << v.id << " as its id";
            stopGame();
        }
        setClientReady(id);
    } else {
        qDebug() << "EEE receiveInitFromClient(" << id << "): "
            << "client hasn't acKed";
        stopGame();
    }
}

void GameServer::processClientMessage(int id, const proto::message_t &message) {
    if (message.size() != 1) {
        qDebug() << "EEE processClientMessage(" << id << "): "
            << "message length (" << message.size() << ") != 1";
        stopGame();
    }

    const auto &e = message[0];
    if (std::holds_alternative<proto::end_game_t>(e)) {
        qDebug() << "EEE processClientMessage(" << id << "): "
            << "client has finished game";
        stopGame();
    } else if (std::holds_alternative<proto::move_t>(e)) {
        const auto &v = std::get<proto::move_t>(e);
        core::pos_t pos;
        // FIXME: simplify
        switch (v.dir) {
            case proto::dir_t::UP: pos = core::UP; break;
            case proto::dir_t::DOWN: pos = core::DOWN; break;
            case proto::dir_t::LEFT: pos = core::LEFT; break;
            case proto::dir_t::RIGHT: pos = core::RIGHT; break;
        }
        if (pos.is_direction())
            game_->set_snake_head_direction(id, pos);
    } else {
        qDebug() << "EEE processClientMessage(" << id << "): "
            << "client sent unexpected message('" <<
            proto::to_string(e).c_str() << "')";
        stopGame();
    }
}

void GameServer::startGame() {
    broadcastMessage(game_->state_message());
    timer_->start(500);
}

void GameServer::stopGame() {
    timer_->stop();
    for (const auto &p : idToClient_) {
        QWebSocket *client = p.second;
        if (client->isValid()) {
           disconnect(client, &QWebSocket::disconnected,
                      this, &GameServer::socketDisconnected);
           client->close();
           client->deleteLater();
        }
    }
}

void GameServer::processTextMessage(QString qstr) {
    if (debug_) qDebug() << "III processTextMessage: " << qstr;
    auto str = qstr.toStdString();
    if (str.empty()) return;
    proto::message_t message = proto::message_from_string(str);

    int id = senderToId(sender());

    if (!isClientReady(id)) {
        receiveInitFromClient(id, message);
        if (allClientsReady()) startGame();
        return;
    }

    if (!allClientsReady()) {
        qDebug() << "EEE processTextMessage(" << id << "): "
            << "not all clients ready yet";
        stopGame();
    }

    processClientMessage(id, message);
}

void GameServer::makeStep() {
    broadcastMessage(game_->step());
}

void GameServer::socketDisconnected() {
    qDebug() << "WWW socketDisconnected: connection with client lost";
    stopGame();
}

void GameServer::sendMessage(int id, const proto::message_t &message) const {
    auto str = proto::to_string(message);
    QString qstr(str.data());
    if (debug_) qDebug() << "III sendMessage: " << qstr;
    if (id != -1) {
        idToClient(id)->sendTextMessage(qstr);
    } else {
        for (const auto &p : idToClient_)
            p.second->sendTextMessage(qstr);
    }
}
