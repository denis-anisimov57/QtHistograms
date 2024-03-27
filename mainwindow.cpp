#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Histogram H(ui->customPlot);
    std::vector<Val> data(0);
    std::vector<Interval> intervals(0);
    for(int i = 0; i < 10; i++) {
        Val v = {double(i * i) / 10., i, i};
        Interval I(i + 0.5, i + 1.5);
        intervals.push_back(I);
        data.push_back(v);
    }
    H.loadIntervals(intervals);
    H.loadData(data);
    H.drawHistogram();
}

MainWindow::~MainWindow()
{
    delete ui;
}

