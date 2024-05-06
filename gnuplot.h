#ifndef GNUPLOT_H
#define GNUPLOT_H

#include <iostream>
#include <string>
#include <cstdio>
using namespace std;

class gnuplot {
public:
    gnuplot();
    ~gnuplot();
    void operator () (const string& command);
protected:
    FILE *gnuplotpipe;
};

gnuplot::gnuplot() {
    gnuplotpipe = _popen("gnuplot -persist", "w");
    if (!gnuplotpipe) {
        cerr << "Gnuplot not found" << endl;
    }
}

gnuplot::~gnuplot() {
    if (gnuplotpipe) {
        fprintf(gnuplotpipe, "exit\n");
        _pclose(gnuplotpipe);
    }
}

void gnuplot::operator()(const string& command) {
    if (gnuplotpipe) {
        fprintf(gnuplotpipe, "%s\n", command.c_str());
        fflush(gnuplotpipe);
    }
}

#endif
