#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QtSerialPort>

#include "imageconverter.h"

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
    bool sendCmd(QString msg);

public slots:
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

private:
    Ui::MainWindow *ui;
    QSerialPort serialPort;
    QImage currentImage;
    ImageConverter imgConverter;

};

#endif // MAINWINDOW_H
