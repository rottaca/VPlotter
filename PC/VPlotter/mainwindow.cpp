#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QImage>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    QObject::connect(ui->b_openImage, SIGNAL (clicked()), this, SLOT (onClickOpenFile()));
    QObject::connect(ui->b_connect, SIGNAL (clicked()), this, SLOT (onClickConnect()));
    QObject::connect(ui->le_command, SIGNAL (returnPressed()), this, SLOT (onSubmitCmd()));

}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::printStatus(QString msg, bool error)
{
    if(error)
        ui->te_output->setTextColor( QColor( "red" ) );
    else
        ui->te_output->setTextColor( QColor( "black" ) );

    ui->te_output->append(msg);
}

void MainWindow::onClickOpenFile()
{
    QString file = QFileDialog::getOpenFileName(this,
       "Open Image", "~", "Image Files (*.png *.jpg *.bmp)");

    if(file.isNull())
        return;

    if(!currentImage.load(file))
    {
        printStatus(QString("Can't open image: %1").arg(file),true);
        ui->gb_imageConvert->setEnabled(false);
        ui->l_image->setPixmap(QImage());

        return;
    }

    ui->l_image->setPixmap(QPixmap::fromImage(currentImage).scaled(
                               ui->l_image->width(),
                               ui->l_image->height(),
                               Qt::KeepAspectRatioByExpanding,
                               Qt::FastTransformation));
    ui->gb_imageConvert->setEnabled(true);
}

void MainWindow::onClickConnect()
{
    if(serialPort.isOpen()){
        serialPort.close();

        ui->le_portName->setEnabled(true);
        ui->b_connect->setText("Connect");
        ui->le_command->setEnabled(false);
        return;
    }
    QString port = ui->le_portName->text();
    serialPort.setPortName(port);

    if (!serialPort.open(QIODevice::ReadWrite)) {
        QString msg = QString("Can't open %1, error code %2!").arg(port).arg(serialPort.error());
        printStatus(msg,true);
    }else{
        ui->le_portName->setEnabled(false);
        ui->b_connect->setText("Disconnect");
        ui->le_command->setEnabled(true);

        QString msg = QString("Connection to %1 successful!").arg(port);
        printStatus(msg);
    }
}
void MainWindow::onSubmitCmd()
{
    QString msg = ui->le_command->text();
    int sz = serialPort.write(msg.toLatin1());
    if(sz<msg.length())
    {
        printStatus(QString("Failed to write all data (only %1 of %2 bytes)").arg(sz).arg(msg.length()),true);
    }
    ui->le_command->setText("");
}

void MainWindow::onClickLeftUp(){}
void MainWindow::onClickLeftDown(){}
void MainWindow::onClickRightUp(){}
void MainWindow::onClickRightDown(){}
void MainWindow::onClickMoveUp(){}
void MainWindow::onClickMoveLeft(){}
void MainWindow::onClickMoveRight(){}
void MainWindow::onClickMoveDown(){}
