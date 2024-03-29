#include "Histogram.h"

Interval::Interval(double start, double end) {
    if(start > end) {
        double tmp = end;
        end = start;
        start = tmp;
    }
    this->start = start;
    this->end = end;
}

bool Interval::inInterval(double val) {
    return ((val > start) && (val <= end));
}

bool Interval::addMsg(Val val) {
    if(inInterval(val.val)) {
        if(msgnumbers.count(val.msgnum)) {
            msgnumbers[val.msgnum]++;
        }
        else {
            msgnumbers[val.msgnum] = 1;
        }

        return true;
    }
    return false;
}

unsigned long long Interval::msgCount() {
//    return std::accumulate(msgnumbers.begin(), msgnumbers.end(), []())//
    unsigned long long count = 0;
    for(auto it = msgnumbers.begin(); it != msgnumbers.end(); it++) {
        count += it->second;
    }
    return count;
}

Histogram::Histogram(QCustomPlot* customPlot) {
    this->customPlot = customPlot;
    data = std::vector<Val>();
}

Histogram::~Histogram() {};

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
    for(int i = 0; i < intervals.size(); i++) {
        double tick = 0, value = 0;
        for(int j = 0; j < data.size(); j++) {
//            if(intervals[i].inInterval(data[j].val)) {
//                value++;
//            }
            intervals[i].addMsg(data[j]);
        }
        bars[i] = new QCPBars(customPlot->xAxis, customPlot->yAxis);
        bars[i]->setWidth(intervals[i].end - intervals[i].start);
        tick = (intervals[i].start + intervals[i].end) / 2;
        bars[i]->setData({tick}, {intervals[i].msgCount() + 0.});
    }
    customPlot->xAxis->setLabel("x");
    customPlot->yAxis->setLabel("y");
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}
