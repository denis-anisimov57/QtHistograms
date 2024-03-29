#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <vector>
#include <map>
#include "qcustomplot/qcustomplot.h"

struct Val {
    double val;
    int sourcenum;
    int msgnum;
};

class Interval {
    public:
        Interval() = default;
        Interval(double start, double end);
        ~Interval() = default;
        bool inInterval(double val);
        bool addMsg(Val val);
        unsigned long long msgCount();
        double start = 0;
        double end = 0;
    private:
        std::map<int, int> msgnumbers; // <msgnum, count in interval>
};

class Histogram {
    public:
        Histogram(QCustomPlot* customPlot);

        void loadIntervals(const std::vector<Interval> interavals);
        void loadData(const std::vector<Val> data);
        void drawHistogram();

        ~Histogram();

    private:
        std::vector<Val> data;
        std::vector<Interval> intervals;
        double barWidth = 0.5;
        QCustomPlot* customPlot = nullptr;
};

#endif // HISTOGRAM_H
