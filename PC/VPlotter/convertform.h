#ifndef CONVERTFORM_H
#define CONVERTFORM_H

#include <QWidget>
#include <QStringList>
#include <QVector2D>
#include <QImage>


namespace Ui {
class ConvertForm;
}

class MainWindow;

class ConvertForm : public QWidget
{
    Q_OBJECT

public:
    explicit ConvertForm(MainWindow *parent = 0);
    ~ConvertForm();

    void setImageInfo(QImage img, QVector2D pos, float scale){
        this->img = img;
        imgPos = pos;
        imgScale = scale;
        preproImg = img;
        onClickApplyPreprocessing();
    }

public slots:
    void onClickConvert();
    void onClickApplyPreprocessing();
    void onClickClose();
    void onChangeConvertAlgorithm();
    void onChangePreprocessAlgorithm();

signals:
    void onImageConverted(QStringList cmds);

private:
    Ui::ConvertForm *ui;
    MainWindow* mainWindow;
    QVector2D imgPos;
    float imgScale;
    QImage img;
    QImage preproImg;

};

#endif // CONVERTFORM_H
