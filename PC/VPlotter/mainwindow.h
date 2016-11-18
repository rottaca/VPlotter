#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QtSerialPort>

#include "imageconverter.h"
#include "commandlistexecutor.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void printStatus(QString msg, bool error=false);

public slots:
    void sendCmd(QString msg);
    void onClickOpenFile();
    void onClickConnect();
    void onSubmitCmd();
    void onClickLeftUp();
    void onClickLeftDown();
    void onClickRightUp();
    void onClickRightDown();
    void onClickMoveUp();
    void onClickMoveLeft();
    void onClickMoveRight();
    void onClickMoveDown();
    void onClickPenUpDown();
    void onClickSetSpeed();
    void onChangeImgBounds();
    void onTimerReadSerial();
    void onClickCalibrate();
    void onPollPosition();
    void onClickOpenCmdFile();
    void onClickExecuteCmdFile();
    void onClickSimulateCmdFile();
    void onCmdExecFinished();

signals:
    void onSerialAnswerRecieved(QString);
    void onStopCmdExec();
    void onExecCmdList(QStringList);

private:
    Ui::MainWindow *ui;
    QSerialPort serialPort;
    QImage currentImage;
    ImageConverter imgConverter;
    QTimer* timer;
    CommandListExecutor* cmdListExec;
};

#endif // MAINWINDOW_H
