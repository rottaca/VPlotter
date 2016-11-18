#include "commandlistexecutor.h"

CommandListExecutor::CommandListExecutor(QSerialPort* port){
    this->serial = port;
    this->moveToThread(&workerThread);
    workerThread.start();
    executingCmds = false;
    timeoutTimer = new QTimer();
    timeoutTimer->moveToThread(&workerThread);
    timeoutTimer->setInterval(2000);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer,SIGNAL(timeout()),this,SLOT(onTimeout()));

}
CommandListExecutor::~CommandListExecutor(){
    executingCmds = false;
    workerThread.quit();
    workerThread.wait();
}
void CommandListExecutor::executeCmdList(QStringList cmds)
{
    currCmdIdx = 0;
    cmdList = cmds;
    executingCmds = true;

    qDebug(cmdList.at(currCmdIdx).toLocal8Bit());
    // Send first command and wait for answer
    sendCmd(cmdList.at(currCmdIdx++));
}


void CommandListExecutor::onRecieveAnswer(QString answ)
{
    if(!executingCmds)
        return;

    timeoutTimer->stop();
    // Busy ? wait for ack
    if(answ.contains("BUSY"))
        return;

    // Ack recieved ? Ack:0 or error code is unimportant.
    // Just execute file
    if(answ.contains("ACK")){
        if(!answ.contains("ACK: 0")){
            qDebug("Error!");
            stop();
        }

        if(currCmdIdx < cmdList.size()){
            qDebug((QString("Send Next: ") + cmdList.at(currCmdIdx).toLocal8Bit()).toLocal8Bit());
            sendCmd(cmdList.at(currCmdIdx++));
        }
        else{
            emit onExecutionFinished();
            executingCmds = false;
        }
    }
}
void CommandListExecutor::onTimeout()
{   if(!executingCmds)
        return;

    qDebug("Resend command after timeout: ");
    qDebug(cmdList.at(currCmdIdx-1).toLocal8Bit());
    sendCmd(cmdList.at(currCmdIdx-1));
}

void CommandListExecutor::stop(){
    executingCmds = false;
    timeoutTimer->stop();
    emit onExecutionAborted();
}
void CommandListExecutor::sendCmd(QString str)
{
    emit onSendCommand(str);
    //timeoutTimer->start();
}
