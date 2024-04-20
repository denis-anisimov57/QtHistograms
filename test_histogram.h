#ifndef TEST_HISTOGRAM_H
#define TEST_HISTOGRAM_H
#include <QObject>
#include "Histogram.h"

class Test_Histogram: public QObject {
    Q_OBJECT
    public:
        Test_Histogram(QObject* parent = 0);
    private:
        Histogram* H = nullptr;
        QCustomPlot* qcp = nullptr;
    private slots:
        void initTestCase();
        void setIntervals();
        void addPlot();
        void selectBars();
        void dataInIntervals();
        void changeIntervals();
        void cleanupTestCase();

};

#endif // TEST_HISTOGRAM_H
