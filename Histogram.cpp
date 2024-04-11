#include "Histogram.h"
#include <QDebug>

Interval::Interval(double start, double end) {
    if(start > end) {
        double tmp = end;
        end = start;
        start = tmp;
    }
    this->start = start;
    this->end = end;
}

bool Interval::inInterval(double val) const {
    return ((val > start) && (val <= end));
}

bool Interval::addMsg(Val val, int sourcenum) {
    if(inInterval(val.val)) {
        if(!msgnumbers.count(sourcenum)) {
            msgnumbers[sourcenum] = std::set<int>();
        }
        if(msgnumbers[sourcenum].count(val.msgnum)) {
            qDebug() << "Interval[" << start << ", " << end << "]: Trying to add msgnum = " << val.msgnum << " which already exists\n";
        }
        else {
            msgnumbers[sourcenum].insert(val.msgnum);
        }
        return true;
    }
    return false;
}

double Interval::length() const {
    return end - start;
}

unsigned long long Interval::msgCount() const {
    unsigned long long count = 0;
    for(auto it = msgnumbers.begin(); it != msgnumbers.end(); it++) {
        count += it->second.size();
    }
    return count;
}

std::map<int, std::set<int>> Interval::getIntervalData() const {
    return msgnumbers;
}

Histogram::Histogram(QCustomPlot* customPlot) {
    this->customPlot = customPlot;
    data = std::vector<Val>();
    customPlot->legend->setVisible(true);
    customPlot->legend->setSelectableParts(QCPLegend::spItems);
    customPlot->legend->setWrap(10);
}

Histogram::~Histogram() {};

void Histogram::setPlot(QCustomPlot *customPlot) {
    this->customPlot = customPlot;
    data = std::vector<Val>();
    customPlot->legend->setVisible(true);
    customPlot->legend->setSelectableParts(QCPLegend::spItems);
    customPlot->legend->setWrap(10);
}

void Histogram::loadPlotData(const Plot plotData) {
    this->data = plotData.vec;
    double max = data[0].val;
    for(Val val : data) {
        if(val.val > max) {
            max = val.val;
        }
    }
    double intCount = (max - plotData.start) / plotData.interval;
    if(intCount - int(intCount) != 0) intCount++;

    this->intervals = std::vector<Interval>(intCount);

    //first interval
    intervals[0] = Interval(plotData.start, plotData.start + plotData.interval);
    for(unsigned long long j = 0; j < data.size(); j++) {
        intervals[0].addMsg(data[j], plotData.sourcenum);
    }

    //other intervals
    for(unsigned long long i = 1; i < intervals.size(); i++) {
        intervals[i] = Interval(intervals[i - 1].end, intervals[i - 1].end + plotData.interval);
        for(unsigned long long j = 0; j < data.size(); j++) {
            intervals[i].addMsg(data[j], plotData.sourcenum);
        }
    }
}

void Histogram::loadIntervals(const std::vector<Interval> intervals) {
    this->intervals = intervals;
}

void Histogram::loadData(std::vector<Val> data) {
    this->data = data;
}

void Histogram::drawHistogram() {
    std::vector<QCPBars*> bars(intervals.size());
    std::vector<int> ticks(intervals.size());
    std::vector<int> values(intervals.size());
    for(unsigned long long i = 0; i < intervals.size(); i++) {
        double tick = 0;
        bars[i] = new QCPBars(customPlot->xAxis, customPlot->yAxis);
        bars[i]->setWidth(intervals[i].length());
        tick = (intervals[i].start + intervals[i].end) / 2;
        bars[i]->setData({tick}, {intervals[i].msgCount() + 0.});
        bars[i]->setName(QString::number(tick));
    }
    customPlot->xAxis->setLabel("x");
    customPlot->yAxis->setLabel("y");

    QSharedPointer<QCPAxisTickerFixed> fixedYTicker(new QCPAxisTickerFixed);
    customPlot->yAxis->setTicker(fixedYTicker);
    fixedYTicker->setTickStep(1.0);
    fixedYTicker->setScaleStrategy(QCPAxisTickerFixed::ssMultiples);

    QSharedPointer<QCPAxisTickerFixed> fixedXTicker(new QCPAxisTickerFixed);
    customPlot->xAxis->setTicker(fixedXTicker);
    fixedXTicker->setTickStep(intervals[0].length());
    fixedXTicker->setTickOrigin(intervals[0].start);
    fixedXTicker->setScaleStrategy(QCPAxisTickerFixed::ssMultiples);

    customPlot->setInteractions(QCP::iRangeDrag |
                                QCP::iRangeZoom |
                                QCP::iSelectPlottables |
                                QCP::iSelectLegend |
                                QCP::iMultiSelect);
}

void Histogram::getData() {
    qDebug() << "Get to the slot!\n";
    std::map<int, std::set<int>> plotData;
    for(int i = 0; i < customPlot->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(customPlot->plottable(i));
        if(bar->selected()) {
            unsigned long long intervalIndex = int((bar->name().toDouble() - intervals[0].start) / bar->width());
            auto data = intervals[intervalIndex].getIntervalData();
            for(auto& src : data) {
                for(auto& msgnum : src.second) {
                    plotData[src.first].insert(msgnum);
                }
            }
        }
    }
    for(auto& src : plotData) {
        qDebug() << "Source: " << src.first << "\nMessage numbers: ";
        for(auto& msgnum : src.second) {
            qDebug() << msgnum << " ";
        }
        qDebug() << "\n";
    }
    emit dataSignal(plotData);
}
