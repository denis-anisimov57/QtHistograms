#ifndef TEST_HISTOGRAM_H
#define TEST_HISTOGRAM_H
#include <QObject>

class Test_Histogram: public QObject {
    Q_OBJECT
    public:
        Test_Histogram(QObject* parent = 0);
    private slots:
        void setIntervals();

};

#endif // TEST_HISTOGRAM_H
