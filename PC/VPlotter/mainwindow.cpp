#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QImage>
#include <QTimer>

#include "graphicseffects.h"
#include "convertform.h"
#include "gcodecommands.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->b_openImage, SIGNAL (clicked()), this, SLOT (onClickOpenFile()));
    connect(ui->b_connect, SIGNAL (clicked()), this, SLOT (onClickConnect()));
    connect(ui->le_command, SIGNAL (returnPressed()), this, SLOT (onSubmitCmd()));

    connect(ui->sb_pos_x, SIGNAL (valueChanged(double)), this, SLOT (onChangeImgBounds()));
    connect(ui->sb_pos_y, SIGNAL (valueChanged(double)), this, SLOT (onChangeImgBounds()));
    connect(ui->sb_scale, SIGNAL (valueChanged(double)), this, SLOT (onChangeImgBounds()));

    connect(ui->b_pen_updown, SIGNAL (clicked()), this, SLOT (onClickPenUpDown()));
    connect(ui->b_move_down, SIGNAL (clicked()), this, SLOT (onClickMoveDown()));
    connect(ui->b_move_left, SIGNAL (clicked()), this, SLOT (onClickMoveLeft()));
    connect(ui->b_move_right, SIGNAL (clicked()), this, SLOT (onClickMoveRight()));
    connect(ui->b_move_up, SIGNAL (clicked()), this, SLOT (onClickMoveUp()));
    connect(ui->b_set_home, SIGNAL (clicked()), this, SLOT (onClickSetHome()));
    connect(ui->b_set_speed, SIGNAL (clicked()), this, SLOT (onClickSetSpeed()));
    connect(ui->b_execute,SIGNAL(clicked()),this,SLOT(onClickExecuteCmdFile()));
    connect(&serialPort, SIGNAL(readyRead()), this, SLOT(onTimerReadSerial()));
    connect(ui->b_simulate,SIGNAL(clicked()),this,SLOT(onClickSimulateCmdFile()));
    connect(ui->cb_nondrawing,SIGNAL(clicked()),this,SLOT(onChangeRenderOptions()));
    connect(ui->cb_penUD,SIGNAL(clicked()),this,SLOT(onChangeRenderOptions()));
    connect(ui->cb_animate,SIGNAL(clicked()),this,SLOT(onChangeRenderOptions()));
    connect(ui->sb_animate_speed,SIGNAL(valueChanged(double)),this,SLOT(onChangeRenderOptions()));
    connect(ui->vp_plotterRenderer,SIGNAL(onSimulationFinished()),this,SLOT(onSimulationFinished()));
    connect(ui->b_convert,SIGNAL(clicked()),this,SLOT(onClickConvert()));
    connect(ui->b_load_cmdFile,SIGNAL(clicked()),this,SLOT(onClickOpenCmdFile()));
    connect(ui->b_save_cmdFile,SIGNAL(clicked()),this,SLOT(onClickSaveCmdFile()));
    connect(ui->rb_show_preproc,SIGNAL(clicked()),this,SLOT(onClickShowRadio()));
    connect(ui->rb_show_raw,SIGNAL(clicked()),this,SLOT(onClickShowRadio()));
    connect(ui->rb_show_simulation,SIGNAL(clicked()),this,SLOT(onClickShowRadio()));
    connect(ui->b_gen_boundinBox,SIGNAL(clicked()),this,SLOT(onClickGenerateBoundingBox()));
    connect(ui->b_home,SIGNAL(clicked()),this,SLOT(onClickHome()));
    connect(ui->te_comand_script,SIGNAL(textChanged()),this,SLOT(onCommandEditorChanged()));
    connect(ui->vp_plotterRenderer,SIGNAL(onChangeProgress(float)),this,SLOT(onChangeProgress(float)));
    connect(this,SIGNAL(syncPen(float,float,bool)),ui->vp_plotterRenderer,SLOT(syncPen(float,float,bool)));

    timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(onPollPosition()));
    //timer->start(5000);
    cmdListExec = new CommandListExecutor(&serialPort);

    connect(cmdListExec,SIGNAL(onExecutionAborted()),this,SLOT(onCmdExecFinished()));
    connect(cmdListExec,SIGNAL(onExecutionFinished()),this,SLOT(onCmdExecFinished()));
    connect(cmdListExec,SIGNAL(onSendCommand(QString)),this,SLOT(sendCmd(QString)));
    connect(this,SIGNAL(onStopCmdExec()),cmdListExec,SLOT(stop()));
    connect(this,SIGNAL(onSerialAnswerRecieved(QString)),cmdListExec,SLOT(onRecieveAnswer(QString)));
    connect(this,SIGNAL(onExecCmdList(QStringList)),cmdListExec,SLOT(executeCmdList(QStringList)));
    connect(cmdListExec,SIGNAL(onChangeProgress(float)),this,SLOT(onChangeProgress(float)));


    ui->vp_plotterRenderer->setPlotterSize(870,1200);
    ui->sb_pos_x->setMaximum(870);
    ui->sb_pos_y->setMaximum(1200);

    onChangeRenderOptions();

    ui->te_comand_script->setContextMenuPolicy(Qt::NoContextMenu);

    convertForm = new ConvertForm(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::printStatus(QString msg, bool error)
{
    QString html("<font color=\"%2\">%1</font><br>");

    QTextEdit textEdit;
    textEdit.setPlainText(msg);
    QString ret = textEdit.toHtml();
    html = html.arg(ret);

    if(error)
        html = html.arg("red");
    else
        html = html.arg("green");

    ui->te_output->moveCursor (QTextCursor::End);
    ui->te_output->insertHtml(html);
    ui->te_output->moveCursor (QTextCursor::End);
}
void MainWindow::setCommandList(QStringList cmds, bool autoSimulate)
{
    QString str = cmds.join("\n");
    setUpdatesEnabled(false);
    ui->te_comand_script->setPlainText(str);
    setUpdatesEnabled(true);
    onCommandEditorChanged();
    if(autoSimulate){
        ui->vp_plotterRenderer->abortSimulation();
        ui->b_simulate->setText("Simulate");
        onClickSimulateCmdFile();
    }
}

void MainWindow::setPreprocessedImage(QImage img)
{
    ui->vp_plotterRenderer->setPreprocessedImage(img);
    ui->rb_show_preproc->setChecked(true);
    ui->vp_plotterRenderer->showItems(VPlotterRenderer::PREPROC);
}

void MainWindow::onClickSaveCmdFile()
{

    QString name = QFileDialog::getSaveFileName(this,
       "Save command File", "", "V Plot Files (*.vplot)");

    if(name.isNull())
        return;

    QFile file(name);
    if(!file.open(QIODevice::WriteOnly)) {
        printStatus(QString("Can't save cmd file %1").arg(name),true);
        return;
    }

    QString cmds = ui->te_comand_script->toPlainText();
    file.write(cmds.toLocal8Bit(),cmds.length());

    printStatus(QString("Cmd file saved: %1").arg(name));

}

void MainWindow::onClickOpenCmdFile()
{
    QString name = QFileDialog::getOpenFileName(this,
       "Open Cmd File", "", "V Plot Files (*.vplot)");

    if(name.isNull())
        return;

    QFile file(name);
    if(!file.open(QIODevice::ReadOnly)) {
        printStatus(QString("Can't load cmd file %1").arg(name),true);
        return;
    }
    QString txt = file.readAll();
    ui->te_comand_script->setPlainText(txt);
    onCommandEditorChanged();
}

void MainWindow::onClickOpenFile()
{
    QString file = QFileDialog::getOpenFileName(this,
       "Open Image", "", "Image Files (*.png *.jpg *.bmp *.jpeg)");

    if(file.isNull() || !currentImage.load(file))
    {
        printStatus(QString("Can't open image: %1").arg(file),true);
        ui->fr_conversationSettings->setEnabled(false);
        ui->vp_plotterRenderer->setRawImage(QImage());
        ui->vp_plotterRenderer->setPreprocessedImage(QImage());
        ui->vp_plotterRenderer->setImageBounds(0,0,0);
        ui->l_img_path->setText("");
        return;
    }

    // Only use grayscale images
    currentImage = currentImage.convertToFormat(QImage::Format_Grayscale8);

    // Truncate text on the right and don't show the full (long) filename
    QFontMetrics metrix(ui->l_img_path->font());
    ui->l_img_path->setText(metrix.elidedText(file, Qt::ElideLeft, ui->l_img_path->width()));

    ui->vp_plotterRenderer->setRawImage(currentImage);
    ui->vp_plotterRenderer->setPreprocessedImage(QImage());
    // TODO
    float scale = std::min(ui->vp_plotterRenderer->getDrawAreaSize().x()/currentImage.width(),
                      ui->vp_plotterRenderer->getDrawAreaSize().y()/currentImage.height());
    QVector2D pos = ui->vp_plotterRenderer->getDrawAreaOrigin();
    ui->vp_plotterRenderer->setImageBounds(scale,pos.x(),pos.y());
    convertForm->setImageInfo(currentImage,
                            pos,scale);
    ui->vp_plotterRenderer->simulateCommands(QStringList());

    ui->sb_scale->setValue(scale);
    ui->sb_pos_x->setValue(pos.x());
    ui->sb_pos_y->setValue(pos.y());
    ui->fr_conversationSettings->setEnabled(true);
    ui->rb_show_raw->setChecked(true);
    ui->vp_plotterRenderer->showItems(VPlotterRenderer::RAW);
    ui->vp_plotterRenderer->resetScale();
}

void MainWindow::onClickConnect()
{
    if(serialPort.isOpen()){
        serialPort.close();
        ui->le_portName->setEnabled(true);
        ui->b_connect->setText("Connect");
        ui->le_command->setEnabled(false);
        ui->gb_manual->setEnabled(false);
        ui->b_execute->setEnabled(false);
        timer->stop();
        cmdListExec->stop();
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
        ui->gb_manual->setEnabled(true);
        ui->b_execute->setEnabled(true);
        ui->te_output->clear();

        QString msg = QString("Connection to %1 successful!").arg(port);
        printStatus(msg);
    }
}
void MainWindow::onSubmitCmd()
{
    QString msg = ui->le_command->text();
    msg.append("\n");
    sendCmd(msg);

    ui->le_command->setText("");
}

void MainWindow::onChangeImgBounds()
{
    float x,y,scale;
    x = ui->sb_pos_x->value();
    y = ui->sb_pos_y->value();
    scale = ui->sb_scale->value();
    ui->vp_plotterRenderer->setImageBounds(scale,x,y);
    convertForm->setImageInfo(currentImage,
                              QVector2D(x,y),
                              scale);
    ui->l_imgBounds->setText(QString("%1x%2")
                             .arg(currentImage.width()*scale)
                             .arg(currentImage.height()*scale));

}
void MainWindow::onTimerReadSerial()
{
    if(!serialPort.isOpen())
        return;

    while(serialPort.canReadLine()){
        QString data = QString(serialPort.readLine());
        if(data.length() > 0){
            if(data.contains("SYNC")){
                QStringList strl = data.split(" ");
                float x = strl.at(1).toFloat();
                float y = strl.at(2).toFloat();
                bool drawing = true;
                if(strl.at(3).contains("U"))
                    drawing = false;
                ui->l_pos->setText(QString("%1 x %2").arg(x).arg(y));
                emit syncPen(x,y,drawing);
                continue;
            }

            ui->te_output->moveCursor (QTextCursor::End);

            QTextEdit textEdit;
            textEdit.setPlainText(data);
            QString ret = textEdit.toHtml();
            // Error recieved ?
            if(ret.contains("ACK") && !ret.contains("ACK: 0"))
                ui->te_output->insertHtml(QString("<font color=\"red\">%1</font>").arg(ret));
            else
                ui->te_output->insertHtml(QString("<font color=\"black\">%1</font>").arg(ret));

            ui->te_output->moveCursor (QTextCursor::End);

            // Send data to cmd executor
            emit onSerialAnswerRecieved(data);
        }
    }
}

void MainWindow::onClickLeftUp(){}
void MainWindow::onClickLeftDown(){}
void MainWindow::onClickRightUp(){}
void MainWindow::onClickRightDown(){}
void MainWindow::onClickMoveUp(){
    sendCmd(GCODE_USE_RELATIVE_POS.append("\n"));
    sendCmd(GCODE_MOVE_Y(-ui->sb_move_dist->value()).append("\n"));
    sendCmd(GCODE_USE_ABSOLUTE_POS.append("\n"));
}
void MainWindow::onClickMoveLeft(){
    sendCmd(GCODE_USE_RELATIVE_POS.append("\n"));
    sendCmd(GCODE_MOVE_X(-ui->sb_move_dist->value()).append("\n"));
    sendCmd(GCODE_USE_ABSOLUTE_POS.append("\n"));
}
void MainWindow::onClickMoveRight(){
    sendCmd(GCODE_USE_RELATIVE_POS.append("\n"));
    sendCmd(GCODE_MOVE_X(ui->sb_move_dist->value()).append("\n"));
    sendCmd(GCODE_USE_ABSOLUTE_POS.append("\n"));
}
void MainWindow::onClickMoveDown(){
    sendCmd(GCODE_USE_RELATIVE_POS.append("\n"));
    sendCmd(GCODE_MOVE_Y(ui->sb_move_dist->value()).append("\n"));
    sendCmd(GCODE_USE_ABSOLUTE_POS);
}
void MainWindow::onClickPenUpDown(){
    if(ui->b_pen_updown->text().compare("Pen Down")==0){
        sendCmd(GCODE_PEN_DOWN.append("\n"));
        ui->b_pen_updown->setText("Pen Up");
    }else{
        sendCmd(GCODE_PEN_UP.append("\n"));
        ui->b_pen_updown->setText("Pen Down");
    }
}
void MainWindow::onClickSetSpeed(){
    float speed = ui->sb_speed->value();
    sendCmd(GCODE_SPEED_DIV(speed).append("\n"));
}
void MainWindow::onCmdExecFinished(){
    ui->b_execute->setText("Execute");
}

void MainWindow::sendCmd(QString msg){

    ui->te_output->moveCursor (QTextCursor::End);
    QTextEdit textEdit;
    textEdit.setPlainText(msg);
    QString ret = textEdit.toHtml();
    ui->te_output->insertHtml(QString("<font color=\"blue\">%1</font>").arg(ret));
    ui->te_output->moveCursor (QTextCursor::End);

    int sz = serialPort.write(msg.toLocal8Bit());

    if(sz<msg.length())
    {
        printStatus(QString("Failed to write all data (only %1 of %2 bytes)").arg(sz).arg(msg.length()),true);
    }
}
void MainWindow::onClickSetHome()
{
    sendCmd(GCODE_SET_HOME.append("\n"));
}
void MainWindow::onPollPosition(){
    sendCmd(GCODE_GET_POSITION.append("\n"));
    QByteArray data = serialPort.readLine();
    QString strData(data);
    QStringList list = strData.split(" ");
    if(list.size() == 2)
        ui->l_pos->setText(list.at(0) + "x" + list.at(1));
    else
        ui->l_pos->setText("unknown...");
}


void MainWindow::onClickExecuteCmdFile()
{
    if(ui->b_execute->text().compare("Execute") == 0){
        QStringList cmds = ui->te_comand_script->toPlainText().split("\n");
        emit onExecCmdList(cmds);
        ui->b_execute->setText("Stop execution");
        executionTimeStart.restart();
    }else{
        emit onStopCmdExec();
        ui->b_execute->setText("Execute");
    }
}
void MainWindow::onClickSimulateCmdFile()
{
    if(ui->b_simulate->text().compare("Simulate") == 0){
        QStringList cmds = ui->te_comand_script->toPlainText().split("\n");
        ui->vp_plotterRenderer->simulateCommands(cmds);
        ui->rb_show_simulation->setChecked(true);
        ui->vp_plotterRenderer->showItems(VPlotterRenderer::SIMULATION);
        ui->b_simulate->setText("Stop Simulation");
        executionTimeStart.restart();
    }else{
        ui->vp_plotterRenderer->abortSimulation();
    }
}
void MainWindow::onChangeRenderOptions()
{
    ui->vp_plotterRenderer->setRenderOptions(ui->cb_nondrawing->isChecked(),
                                             ui->cb_penUD->isChecked(),
                                             ui->cb_animate->isChecked(),
                                             ui->sb_animate_speed->value());
}
void MainWindow::onSimulationFinished()
{
    ui->b_simulate->setText("Simulate");
}

void MainWindow::onClickConvert()
{
    convertForm->show();
    convertForm->activateWindow();
    convertForm->raise();
}
void MainWindow::onClickShowRadio()
{
    if(ui->rb_show_preproc->isChecked()){
        ui->vp_plotterRenderer->showItems(VPlotterRenderer::PREPROC);
    }else if(ui->rb_show_raw->isChecked()){
        ui->vp_plotterRenderer->showItems(VPlotterRenderer::RAW);
    }else{
        ui->vp_plotterRenderer->showItems(VPlotterRenderer::SIMULATION);
    }
}
void MainWindow::onCommandEditorChanged()
{
    long cnt = ui->te_comand_script->document()->blockCount();
    ui->l_editor_lines_of_code->setText(QString("%1").arg(cnt));
}

void MainWindow::onClickGenerateBoundingBox()
{
    QStringList cmds;
    cmds.append(GCODE_PEN_UP);
    cmds.append(GCODE_MOVE_TO_AND_SPEED(ui->sb_pos_x->value(),ui->sb_pos_y->value(),1));
    cmds.append(GCODE_PEN_DOWN);
    cmds.append(GCODE_USE_RELATIVE_POS);
    cmds.append(GCODE_SPEED_DRAW);
    cmds.append(GCODE_MOVE_X(ui->sb_scale->value()*currentImage.width()));
    cmds.append(GCODE_MOVE_Y(ui->sb_scale->value()*currentImage.height()));
    cmds.append(GCODE_MOVE_X(-ui->sb_scale->value()*currentImage.width()));
    cmds.append(GCODE_MOVE_Y(-ui->sb_scale->value()*currentImage.height()));
    cmds.append(GCODE_USE_ABSOLUTE_POS);
    cmds.append(GCODE_PEN_UP);
    setCommandList(cmds,true);
}
void MainWindow::onChangeProgress(float p)
{
    ui->pb_progress->setValue(p*100);
    int msFullPrint = executionTimeStart.elapsed()/p;

    int hours = msFullPrint/(1000*60*60);
    int minutes = (msFullPrint-(hours*1000*60*60))/(1000*60);
    int seconds = (msFullPrint-(minutes*1000*60)-(hours*1000*60*60))/1000;

    QString formattedTime = (QString("%1").arg(hours, 2, 10, QLatin1Char('0')) + ":" +
                         QString( "%1" ).arg(minutes, 2, 10, QLatin1Char('0')) + ":" +
                         QString( "%1" ).arg(seconds, 2, 10, QLatin1Char('0')));

    ui->l_time_full->setText(formattedTime);

    int msLeftPrint = msFullPrint*(1-p);

    hours = msLeftPrint/(1000*60*60);
    minutes = (msLeftPrint-(hours*1000*60*60))/(1000*60);
    seconds = (msLeftPrint-(minutes*1000*60)-(hours*1000*60*60))/1000;

    formattedTime = (QString("%1").arg(hours, 2, 10, QLatin1Char('0')) + ":" +
                         QString( "%1" ).arg(minutes, 2, 10, QLatin1Char('0')) + ":" +
                         QString( "%1" ).arg(seconds, 2, 10, QLatin1Char('0')));

    ui->l_time_left->setText(msLeftPrint);

    QTime t = executionTimeStart.addMSecs(msFullPrint);
    ui->l_end_time->setText(t.toString("hh:mm"));

}

void MainWindow::onClickHome()
{
    sendCmd(GCODE_PEN_UP.append("\n"));
    sendCmd(GCODE_SPEED_DRAW.append("\n"));
    sendCmd(GCODE_HOME.append("\n"));
}
