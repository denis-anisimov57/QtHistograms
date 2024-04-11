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
        Val v = {double(i * i), i};
        data.push_back(v);
    }
    data.push_back({26, 5});
    Plot P = {data, 1, 10, 1};
    H.loadPlotData(P);
    H.drawHistogram();
    connect(ui->customPlot, &QCustomPlot::selectionChangedByUser, this, &MainWindow::selectionChanged);

    ui->customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->customPlot, &QCustomPlot::customContextMenuRequested, this, &MainWindow::showMenu);
}

void MainWindow::showMenu(const QPoint& pos) {
    QMenu contextMenu(tr("Context menu"), this);
    QAction act1("Show table", this);
    contextMenu.addAction(&act1);
    connect(&act1, &QAction::triggered, this, &MainWindow::tableSignal);
    for(int i = 0; i < ui->customPlot->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(ui->customPlot->plottable(i));
        if(bar->selected()) {
            contextMenu.exec(ui->customPlot->mapToGlobal(pos));
        }
    }
}

void MainWindow::tableSignal() {
    qDebug() << "selected action!\n";
}

void MainWindow::selectionChanged() {
    for(int i = 0; i < ui->customPlot->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(ui->customPlot->plottable(i));
        QCPPlottableLegendItem* item = ui->customPlot->legend->itemWithPlottable(bar);
        if(item->selected() || bar->selected()) {
            item->setSelected(true);
            bar->setSelection(QCPDataSelection(bar->data()->dataRange()));
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

