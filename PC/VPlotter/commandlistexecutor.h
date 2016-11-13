#ifndef COMMANDLISTEXECUTOR_H
#define COMMANDLISTEXECUTOR_H

#include <QSerialPort>
#include <QStringList>

class CommandListExecutor
{
public:
    CommandListExecutor(QSerialPort* serial);

    void executeCmdList(QStringList cmds);
    void stop();

public slots:
    void onRecieveAnswer(QString answ);

private:
    QSerialPort* serial;
};

#endif // COMMANDLISTEXECUTOR_H
