#include "commandlistexecutor.h"

CommandListExecutor::CommandListExecutor(QSerialPort* serial)
{
    this->serial = serial;
}

void CommandListExecutor::executeCmdList(QStringList cmds)
{

}


void CommandListExecutor::onRecieveAnswer(QString answ)
{

}

void CommandListExecutor::stop(){

}
