#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <vector>
#include <QDebug>
#include <QObject>
#include <map>
#include <set>
#include "qcustomplot/qcustomplot.h"

struct Val {
    double val;
    int msgnum;
};

struct Plot {
    std::vector<Val> vec;
    double start;
    double interval;
    int sourcenum;
};

class Interval {
    public:
        Interval() = default;
        Interval(double start, double end);
        ~Interval() = default;

        bool inInterval(double val) const;
        bool addMsg(Val val, int sourcenum);
        double start = 0;
        double end = 0;

        double length() const;
        unsigned long long msgCount() const;
        std::map<int, std::set<int>> getIntervalData() const;
    private:
        std::map<int, std::set<int>> msgnumbers; // <sourcenum, set with msgnums>
};

class Histogram : public QObject {
    Q_OBJECT
public:
        Histogram(QCustomPlot* customPlot);
        Histogram() = default;
        void setPlot(QCustomPlot* customPlot);

        void loadIntervals(const std::vector<Interval> interavals);
        void loadPlotData(const Plot);
        void loadData(const std::vector<Val> data);
        void drawHistogram();

        ~Histogram();

    private:
        std::vector<Val> data;
        std::vector<Interval> intervals;
        QCustomPlot* customPlot = nullptr;
    public slots:
        void getData();
};

#endif // HISTOGRAM_H
