#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Histogram H(ui->customPlot);
    std::vector<Val> data(0);
    for(int i = 0; i < 10; i++) {
        Val v = {1.0, i, i};
        data.push_back(v);
    }
    H.loadData(data);
    H.drawHistogram();
}

MainWindow::~MainWindow()
{
    delete ui;
}

