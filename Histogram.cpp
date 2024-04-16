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

MsgNumbersMap Interval::getIntervalData() const {
    return msgnumbers;
}

Histogram::Histogram(QCustomPlot* customPlot) {
    this->customPlot = customPlot;
    allData = AllPlotInfo();
    intervals = std::vector<Interval>();
    customPlot->legend->setVisible(true);
    customPlot->legend->setSelectableParts(QCPLegend::spItems);
    customPlot->legend->setWrap(10);
}

Histogram::~Histogram() {};

void Histogram::setUIPlot(QCustomPlot *customPlot) {
    this->customPlot = customPlot;
    allData = AllPlotInfo();
    intervals = std::vector<Interval>();
    customPlot->legend->setVisible(true);
    customPlot->legend->setSelectableParts(QCPLegend::spItems);
    customPlot->legend->setWrap(10);
}

void Histogram::addPlot(const Plot plotData) {
    if(allData.interval <= 0) {
        throw(std::runtime_error("Invalid interval length"));//???
    }
    allData.PlotVec.push_back(plotData);
    std::vector<Val> data = plotData.vec;
    double max = data[0].val;
    for(Val val : data) {
        if(val.val > max) {
            max = val.val;
        }
    }
    if(intervals.empty() || max > intervals[intervals.size() - 1].end) {
        double intCount = (max - allData.start) / allData.interval;
        if(intCount - int(intCount) != 0) intCount++;
        this->intervals.resize(intCount);
    }

    //first interval
    if(!intervals[0].length()) {
        intervals[0] = Interval(allData.start, allData.start + allData.interval);
    }
    for(unsigned long long j = 0; j < data.size(); j++) {
        intervals[0].addMsg(data[j], plotData.sourcenum);
    }

    //other intervals
    for(unsigned long long i = 1; i < intervals.size(); i++) {
        if(!intervals[i].length()) {
            intervals[i] = Interval(intervals[i - 1].end, intervals[i - 1].end + allData.interval);
        }
        for(unsigned long long j = 0; j < data.size(); j++) {
            intervals[i].addMsg(data[j], plotData.sourcenum);
        }
    }
}

void Histogram::setIntervals(const double start, const double interval) {
    allData.start = start;
    allData.interval = interval;
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
    MsgNumbersMap plotData;
    for(int i = 0; i < customPlot->plottableCount(); i++) {
        QCPBars* bar = dynamic_cast<QCPBars*>(customPlot->plottable(i));
        if(bar->selected()) {
            unsigned long long intervalIndex = int((bar->data()->at(0)->mainKey() - allData.start) / bar->width());
            auto data = intervals[intervalIndex].getIntervalData();
            for(auto& src : data) {
                for(auto& msgnum : src.second) {
                    plotData[src.first].insert(msgnum);
                }
            }
        }
    }
    emit dataSignal(plotData);
}
