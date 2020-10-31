#ifndef RENDER_H
#define RENDER_H

#include <QtCore/QObject>
#include <QtCore/QThread>

class Render : public QThread
{
    Q_OBJECT
public:
    explicit Render();

    void run() override;

Q_SIGNALS:
    void messageSent(QString qstr);
    void gameFinished();

public Q_SLOTS:
    void receiveMessage(QString qstr);
};

#endif // RENDER_H
