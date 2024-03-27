#include "Histogram.h"

Histogram::Histogram(QCustomPlot* customPlot) {
    this->customPlot = customPlot;
    data = std::vector<Val>();
}

Histogram::~Histogram() {};

void Histogram::loadData(std::vector<Val> data) {
    this->data = data;
}

void Histogram::drawHistogram() {
    std::vector<QCPBars*> bars(data.size());
    std::vector<int> ticks(data.size());
    std::vector<int> values(data.size());
    for(int i = 0; i < bars.size(); i++) {
        bars[i] = new QCPBars(customPlot->xAxis, customPlot->yAxis);
        bars[i]->setData({i + 1.}, {1});
    }
    customPlot->xAxis->setLabel("x");
    customPlot->yAxis->setLabel("y");
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}
