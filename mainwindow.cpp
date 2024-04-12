#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSignalSpy>
#include <QTest>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    H.setPlot(ui->customPlot);

    std::vector<Val> data(0);
    std::vector<Interval> intervals(0);
    int sourcenum = 1;
    double start = 1;
    double intervalWidth = 10;

    for(int i = 0; i < 10; i++) {
        Val v = {double(i * i), i};
        data.push_back(v);
    }

    //test adding existing msgnum
    data.push_back({26, 5});

    Plot P = {data, start, intervalWidth, sourcenum};
    H.loadPlotData(P);
    H.drawHistogram();

    connect(ui->customPlot, &QCustomPlot::selectionChangedByUser, this, &MainWindow::selectionChanged);

    ui->customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->customPlot, &QCustomPlot::customContextMenuRequested, this, &MainWindow::showMenu);
}

void MainWindow::showMenu(const QPoint& pos) {
    QMenu contextMenu("Context menu", this);
    QAction act1("Show table", this);
    contextMenu.addAction(&act1);

    QSignalSpy spy(&H, &Histogram::dataSignal);

    connect(&act1, &QAction::triggered, &H, &Histogram::getData);
    for(int i = 0; i < ui->customPlot->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(ui->customPlot->plottable(i));
        if(bar->selected()) {
            contextMenu.exec(ui->customPlot->mapToGlobal(pos));
            break;
        }
    }

    //testing recieved data from signal
    if(spy.count()) {
        MsgNumbersMap args = spy.takeFirst().at(0).value<MsgNumbersMap>();
        for(auto& src : args) {
            qDebug() << "(Signal)Source: " << src.first << "\n(Signal)Message numbers: ";
            for(auto& msgnum : src.second) {
                qDebug() << msgnum << " ";
            }
            qDebug() << "\n";
        }
    }
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

