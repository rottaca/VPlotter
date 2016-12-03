#include "convertform.h"
#include "ui_convertform.h"
#include "mainwindow.h"
#include "graphicseffects.h"
#include "convertimagealgorithms.h"

ConvertForm::ConvertForm(MainWindow *parent) :
    QWidget(parent),
    ui(new Ui::ConvertForm),mainWindow(parent)
{
    setWindowFlags(Qt::Window);
    ui->setupUi(this);

    //connect(ui->b_close,SIGNAL(clicked()),this,SLOT(onClickClose()));
    connect(ui->b_convert,SIGNAL(clicked()),this,SLOT(onClickConvert()));
    connect(ui->cb_convert_algorithm,SIGNAL(currentIndexChanged(int)),this,SLOT(onChangeConvertAlgorithm()));
    connect(ui->cb_preprocess_algorithm,SIGNAL(currentIndexChanged(int)),this,SLOT(onChangePreprocessAlgorithm()));
    connect(ui->b_apply,SIGNAL(clicked()),this,SLOT(onClickApplyPreprocessing()));
    onChangeConvertAlgorithm();
    onChangePreprocessAlgorithm();
}

ConvertForm::~ConvertForm()
{
    delete ui;
}

void ConvertForm::onClickConvert()
{
    qDebug("Processing...");
    int algo = ui->cb_convert_algorithm->currentIndex();
    QStringList cmds;
    QMatrix3x3 l2w = ConvertImageAlgorithms::computeLocalToWorldTransform(imgPos,imgScale);
    switch(algo){
    case 0:{
        bool draws[4];
        draws[0] = ui->cb_multiline_draw_0->isChecked();
        draws[1] = ui->cb_multiline_draw_45->isChecked();
        draws[2] = ui->cb_multiline_draw_90->isChecked();
        draws[3] = ui->cb_multiline_draw_135->isChecked();
        cmds = ConvertImageAlgorithms::convertMultiLines(preproImg,
                                                         draws,
                                                         ui->cb_multiline_map->isChecked(),
                                                         ui->hs_multilines_sampling->value()/imgScale,
                                                         ui->hs_multilines_threshold->value(),
                                                         l2w);
        }
        break;
        case 1:{
        cmds = ConvertImageAlgorithms::convertSquares(preproImg,
                                                         qMin(preproImg.width(),preproImg.height()),
                                                         ui->sb_squares_recursionDepth->value(),
                                                         l2w);
        }
        break;
        case 2:{
        cmds = ConvertImageAlgorithms::convertWave(preproImg,
                                                         ui->hs_sin_y_sampling->value()/imgScale,
                                                         ui->hs_sin_x_sampling->value()/imgScale,
                                                         l2w);
        }
        break;
        case 3:{
        cmds = ConvertImageAlgorithms::convertPixels(preproImg,
                                                         ui->hs_pixels_sampling->value()/imgScale,
                                                         l2w,
                                                         ConvertImageAlgorithms::LINEAR);
        }
        break;
        default:
            break;
    }
    qDebug("Done.");
    mainWindow->setCommandList(cmds,true);
}

void ConvertForm::onClickClose()
{
    hide();
}

void ConvertForm::onChangeConvertAlgorithm(){
    int algo = ui->cb_convert_algorithm->currentIndex();
    ui->sw_convert_algo_settings->setCurrentIndex(algo);
}

void ConvertForm::onClickApplyPreprocessing()
{
    int algo = ui->cb_preprocess_algorithm->currentIndex();
    switch(algo){
        case 0:{
            preproImg = img.copy();
            break;
        }
        case 1:{
            int threshold = ui->hs_ed_threshold->value();
            preproImg = GraphicsEffects::applyBlur(img);
            preproImg = GraphicsEffects::applySobel(preproImg);
            preproImg = GraphicsEffects::applyBinarize(preproImg,threshold,0,255);
            break;
        }
        case 2:{
            int threshold = ui->hs_binarize_threshold->value();
            bool inv = ui->cb_binarize_invert->isChecked();
            preproImg = GraphicsEffects::applyBinarize(img,threshold,inv?255:0,inv?0:255);
            break;
        }
        case 3:{
            int min = ui->sb_stretch_min->value();
            int max = ui->sb_stretch_max->value();
            bool automatic = ui->cb_stretch_auto->isChecked();
            float quantile = ui->sb_stretch_quantile->value();
            preproImg = GraphicsEffects::applyStretch(img,automatic,quantile, min,max);
            break;
        }
        default:
            break;
    }
    mainWindow->setPreprocessedImage(preproImg);

}
void ConvertForm::onChangePreprocessAlgorithm()
{
    int algo = ui->cb_preprocess_algorithm->currentIndex();
    ui->sw_preproc_algo_settings->setCurrentIndex(algo);
}
