#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <vector>
#include <QDebug>
#include <QObject>
#include <map>
#include <set>
#include "qcustomplot/qcustomplot.h"
#include <MyQCP.h>

using MsgNumbersMap = std::map<int, std::set<int>>; // <sourcenum, set with msgnums>
Q_DECLARE_METATYPE(MsgNumbersMap);

struct Val {
    double val;
    int msgnum;
};

struct Plot {
    std::vector<Val> vec;
    int sourcenum;
};

struct AllPlotInfo
{
    double start = 0;
    double interval = 0;

    std::vector<Plot> PlotVec = std::vector<Plot>();
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
        MsgNumbersMap getIntervalData() const;
    private:
        MsgNumbersMap msgnumbers;
};

class Histogram : public QObject {
    Q_OBJECT
public:
        Histogram(QCustomPlot* customPlot);
        Histogram() = default;
        void setUIPlot(QCustomPlot* customPlot);

        void setIntervals(const double start, const double interval);
        void addPlot(const Plot);
        void drawHistogram();

        void move(int key);

        ~Histogram();
    private:
        AllPlotInfo allData;
        std::vector<Interval> intervals;
        QCustomPlot* customPlot = nullptr;
    public slots:
        void getData();
    signals:
        void dataSignal(MsgNumbersMap);
};

#endif // HISTOGRAM_H
