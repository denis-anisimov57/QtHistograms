#include "test_histogram.h"
#include <QTest>
#include <QSignalSpy>

Test_Histogram::Test_Histogram(QObject* parent): QObject(parent) {}

void Test_Histogram::initTestCase() {
    qDebug() << "Starting tests for Histogram";
    qDebug() << "Initializing values...";

    qcp = new QCustomPlot();
    H = new hst::Histogram(qcp);

    qDebug() << "Done!";
}

void Test_Histogram::setIntervals() {
    std::vector<hst::Message> vec = {{1, 100}, {2, 200}, {3, 300}};

    QVERIFY_EXCEPTION_THROWN(H->addPlot({vec, 1}), std::runtime_error);
    QVERIFY_EXCEPTION_THROWN(H->setIntervals(0, -10), std::runtime_error);

    H->setIntervals(-10, 30);
    try {
        H->addPlot({vec, 1});
    }
    catch(std::runtime_error& ex) {
        QFAIL("Incorrect excpetion: interval length is positive");
    }
}

void Test_Histogram::addPlot() {
    std::vector<hst::Message> valVec =
    {{101, 1}, {243, 2}, {5, 3}, {67.3, 4}, {34, 5},
     {123, 6}, {-45, 7}, {-10, 8}, {10.8, 9}, {531, 10}};
    hst::Plot p = {valVec, 23};
    H->addPlot(p);
    H->drawHistogram();
    QSignalSpy spy(H, &hst::Histogram::dataSignal);
    for(int i = 0; i < qcp->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(qcp->plottable(i));
        bar->setSelection(QCPDataSelection(bar->data()->dataRange()));
    }
    H->getData();
    MsgNumbersMap args = spy.takeFirst().at(0).value<MsgNumbersMap>();
    QVERIFY(!args.empty());
    QCOMPARE(args.size(), 2ul);
    std::set<int> msgnums1 = args[23];
    QCOMPARE(msgnums1.size(), 9ul);
    QCOMPARE(msgnums1.count(8), 1ul);
    QCOMPARE(msgnums1.count(10), 1ul);
    QCOMPARE(msgnums1.count(7), 0ul);
    QCOMPARE(msgnums1, std::set<int>({1, 2, 3, 4, 5, 6, 8, 9, 10}));
    std::set<int> msgnums2 = args[1];
    QCOMPARE(msgnums2, std::set<int>({100, 200, 300}));
}

void Test_Histogram::selectBars() {
    qcp->deselectAll();
    QCOMPARE(qcp->plottableCount(), 19);
    for(int i = 0; i < 3; i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(qcp->plottable(qrand() % qcp->plottableCount()));
        bar->setSelection(QCPDataSelection(bar->data()->dataRange()));
    }
    emit qcp->selectionChangedByUser();
    for(int i = 0; i < 10; i++) {
        int barIndex = qrand() % qcp->plottableCount();
        QCPBars* bar = dynamic_cast<QCPBars*>(qcp->plottable(barIndex));
        QCPPlottableLegendItem* item = qcp->legend->itemWithPlottable(bar);
        QCOMPARE(bar->selected(), item->selected());
    }
}

void Test_Histogram::dataInIntervals() {
    std::vector<int> intHeights = {6, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    std::map<double, int> tickHeights;
    for(unsigned long long i = 0; i < intHeights.size(); i++) {
        tickHeights[5 + i * 30] = intHeights[i];
    }

    for(int i = 0; i < qcp->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(qcp->plottable(i));
        double tick = bar->data()->at(0)->mainKey();
        try {
            tickHeights.at(tick);
        }
        catch(std::out_of_range) {
            std::string errStr = "Invalid bar tick: " + std::to_string(tick);
            QFAIL(errStr.c_str());
        }
        QCOMPARE(bar->data()->at(0)->mainValue(), tickHeights[tick]);
    }
}

void Test_Histogram::changeIntervals() {
    H->setIntervals(0, 100);
    QCOMPARE(qcp->plottableCount(), 6);
    std::map<double, int> tickHeights = {{50, 7}, {150, 2}, {250, 1}, {350, 0}, {450, 0}, {550, 1}};
    for(int i = 0; i < qcp->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(qcp->plottable(i));
        double tick = bar->data()->at(0)->mainKey();
        try {
            tickHeights.at(tick);
        }
        catch(std::out_of_range) {
            std::string errStr = "Invalid bar tick: " + std::to_string(tick);
            QFAIL(errStr.c_str());
        }
        QCOMPARE(bar->data()->at(0)->mainValue(), tickHeights[tick]);
    }
}

void Test_Histogram::cleanupTestCase() {
    qDebug() << "Tests ended";
    qDebug() << "Deinitializing values...";

    delete H;
    delete qcp;

    qDebug() << "Done";
}
