#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    H = new hst::Histogram(ui->customPlot, ui->statusbar);

    std::vector<hst::Message> data(0);
    std::vector<hst::Interval> intervals(0);
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
    for(int i = 10; i < 16; i++) {
        hst::Message v = {double(i * i), i};
        data.push_back(v);
    }
    H->addPlot({data, 2});

    H->drawHistogram();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    H->move(event->key());
}

MainWindow::~MainWindow()
{
    delete H;
    delete ui;
}

