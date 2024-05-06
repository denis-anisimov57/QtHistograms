#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    H = ui->histogram;

    std::vector<hst::Message> data(0);
    double start = 1;
    double intervalWidth = 10;

    for(int i = 0; i < 10; i++) {
        hst::Message v = {double(i * i), i};
        data.push_back(v);
    }

    H->setIntervals(start, intervalWidth);
    H->addPlot({data, 1});

    //test adding existing msgnum
    data.push_back({26, 5});
    //test adding another plot
    for(int i = 10; i < 16; i++) {
        hst::Message v = {double(i * i), i};
        data.push_back(v);
    }
    H->addPlot({data, 2});

    H->drawHistogram();
}

MainWindow::~MainWindow()
{
    delete ui;
}

