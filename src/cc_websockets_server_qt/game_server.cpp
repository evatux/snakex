#include "QtWebSockets/qwebsocket.h"
#include <QtCore/QDebug>

#include "cc_core/game.hpp"
#include "cc_core/proto.hpp"

#include "game_server.h"

QT_USE_NAMESPACE

GameServer::GameServer(QWebSocket *pSocket) : pSocket(pSocket) {
    connect(pSocket, &QWebSocket::textMessageReceived, this, &GameServer::processTextMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &GameServer::socketDisconnected);

    game_.reset(new core::game_t({30, 30}, 1));
    timer_.reset(new QTimer(this));
    connect(timer_.get(), &QTimer::timeout, this, &GameServer::makeStep);
}

GameServer::~GameServer() {
    socketDisconnected();
}

void GameServer::startGame() {
    proto::message_t message = game_->state_message();
    sendMessage(message);

    timer_->start(500);
}

void GameServer::processTextMessage(QString message) {
    auto str = message.toStdString();
    if (str.empty()) return;

    switch(str[0]) {
        case '0': case '1': case '2': case '3': startGame(); break;
        // Player 1
        case 'W': game_->set_snake_head_direction(0, core::UP);    break;
        case 'S': game_->set_snake_head_direction(0, core::DOWN);  break;
        case 'A': game_->set_snake_head_direction(0, core::LEFT);  break;
        case 'D': game_->set_snake_head_direction(0, core::RIGHT); break;
        // Player 2
        case 'w': game_->set_snake_head_direction(1, core::UP);    break;
        case 's': game_->set_snake_head_direction(1, core::DOWN);  break;
        case 'a': game_->set_snake_head_direction(1, core::LEFT);  break;
        case 'd': game_->set_snake_head_direction(1, core::RIGHT); break;
        // Control
        case 'q': pSocket->close(); return;
    }
}

void GameServer::makeStep() {
    proto::message_t message = game_->step();
    sendMessage(message);
}

void GameServer::socketDisconnected() {
    timer_->stop();
    if (pSocket) pSocket->deleteLater();
    pSocket = nullptr;
}

void GameServer::sendMessage(const proto::message_t &message) {
    auto str = proto::to_string(message);
    QString qstr(str.data());
    pSocket->sendTextMessage(qstr);
}
