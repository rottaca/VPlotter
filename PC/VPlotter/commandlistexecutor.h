#ifndef COMMANDLISTEXECUTOR_H
#define COMMANDLISTEXECUTOR_H

#include <QSerialPort>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QTimer>

class CommandListExecutor : public QObject
{
    Q_OBJECT
public:
    CommandListExecutor(QSerialPort* serial);
    virtual ~CommandListExecutor();

signals:
    void onExecutionFinished();
    void onExecutionAborted();
    void onSendCommand(QString cmd);

public slots:
    void onRecieveAnswer(QString answ);
    void executeCmdList(QStringList cmds);
    void stop();
    void onTimeout();

private:
    void sendCmd(QString str);
private:
    QSerialPort* serial;
    QThread workerThread;
    QStringList cmdList;
    int currCmdIdx;
    bool executingCmds;
    QTimer* timeoutTimer;

};

#endif // COMMANDLISTEXECUTOR_H
