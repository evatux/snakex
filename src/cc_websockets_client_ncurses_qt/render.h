#ifndef RENDER_H
#define RENDER_H

#include <QtCore/QObject>
#include <QtCore/QThread>

class Render : public QThread
{
    Q_OBJECT
public:
    explicit Render(int id);

    void run() override;
    void stopRendering();

Q_SIGNALS:
    void messageSent(QString qstr);
    void gameFinished();

public Q_SLOTS:
    void receiveMessage(QString qstr);

private:
    int id_;

    void init();
};

#endif // RENDER_H
