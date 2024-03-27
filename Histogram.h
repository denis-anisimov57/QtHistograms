#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <vector>
#include "qcustomplot/qcustomplot.h"

struct Val {
    double val;
    int sourcenum;
    int msgnum;
};

class Histogram {
    public:
        Histogram(QCustomPlot* customPlot);

        void loadData(const std::vector<Val> data);
        void drawHistogram();

        ~Histogram();

    private:
        std::vector<Val> data;
        QCustomPlot* customPlot = nullptr;
};

#endif // HISTOGRAM_H
