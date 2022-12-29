//-*-c++-*-
#ifndef _Filter_h_
#define _Filter_h_

using namespace std;

class Filter {
  int divisor;

// Moving items to public attributes increased perf
// These items are now ready to use and not hidden
// However, this exposes information security
public:
  Filter(int _dim);
  int dim;
  int *data;
  int* get(int r, int c);
  void set(int r, int c, int value);

  int getDivisor();
  void setDivisor(int value);

  int getSize();
  void info();
};

#endif
