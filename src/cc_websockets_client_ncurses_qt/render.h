#ifndef RENDER_H
#define RENDER_H

#include <QtCore/QObject>
#include <QtCore/QThread>

#include "cc_core/proto.hpp"

class Render : public QThread {
    Q_OBJECT
public:
    explicit Render(int id);

    void run() override;
    bool isReady() const;
    void stopRendering();
    void receiveMessage(const proto::message_t &message);

Q_SIGNALS:
    void messageSent(QString qstr);
    void gameFinished();

private:
    int id_ = -1;

    void init();
};

#endif // RENDER_H
