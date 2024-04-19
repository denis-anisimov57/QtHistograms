#include "test_histogram.h"
#include <QTest>
#include "Histogram.h"

Test_Histogram::Test_Histogram(QObject* parent): QObject(parent) {}

void Test_Histogram::setIntervals() {
    Histogram H;
    QCustomPlot* qcp = new QCustomPlot();
    H.setUIPlot(qcp);
    std::vector<Val> vec = {{1, 1}, {2, 2}, {3, 3}};

    QVERIFY_EXCEPTION_THROWN(H.addPlot({vec, 1}), std::runtime_error);
    QVERIFY_EXCEPTION_THROWN(H.setIntervals(0, -10), std::runtime_error);

    H.setIntervals(1, 1);
    try {
        H.addPlot({vec, 1});
    }
    catch(std::runtime_error& ex) {
        QFAIL("Incorrect excpetion: interval length is positive");
    }
    delete qcp;
}
