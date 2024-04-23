#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <vector>
#include <QDebug>
#include <QObject>
#include <map>
#include <set>
#include "qcustomplot/qcustomplot.h"

using MsgNumbersMap = std::map<int, std::set<int>>; // <sourcenum, set with msgnums>
Q_DECLARE_METATYPE(MsgNumbersMap);

namespace hst {

const int legendRowCount = 10;
const double moveDelta = 0.01;
const double scaleCoeff = 1.01;
const double standartScale = 1.05;
const int statusNumberAccuracy = 2;
const double yTickStep = 1.0;

//constants for bar colors
const int rangeR = 128;
const int startR = 128;
const int rangeG = 50;
const int startG = 128;
const int barAlpha = 170;
const int penAlpha = 255;
const double penWidth = 1.5;

struct Message {
    double value;
    int msgnum;
};

struct Plot {
    std::vector<Message> vec;
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
        bool addMsg(Message msg, int sourcenum);
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
        Histogram(QCustomPlot* customPlot, QStatusBar* statusbar = nullptr);

        void setIntervals(const double start, const double interval);
        void addPlot(const Plot);
        void drawHistogram();

        void move(int key);
        void regenerateColors();

        ~Histogram();
    private:
        void calculateIntervals(const Plot);

        bool isDrawn = false;
        QVector<int> selectedBars = QVector<int>();
        AllPlotInfo allData;
        std::vector<Interval> intervals;
        QCustomPlot* customPlot = nullptr;
        QLabel* statusLabel = nullptr;
    public slots:
        void resetScale();
        void getData();
        void selectionChanged();
        void showMenu(const QPoint& pos);

    signals:
        void dataSignal(MsgNumbersMap);
};

}

#endif // HISTOGRAM_H
