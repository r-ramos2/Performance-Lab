#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include "Filter.h"
#include <omp.h> // OpenMP to run multiple processors at once
// This file took the heaviest processing (94%) according to perf tool

using namespace std;

#include "rdtsc.h"

//
// Forward declare the functions
//
Filter * readFilter(string filename);
double applyFilter(Filter *filter, cs1300bmp *input, cs1300bmp *output);

int
main(int argc, char **argv)
{

  if ( argc < 2) {
    fprintf(stderr,"Usage: %s filter inputfile1 inputfile2 .... \n", argv[0]);
  }

  //
  // Convert to C++ strings to simplify manipulation
  //
  string filtername = argv[1];

  //
  // remove any ".filter" in the filtername
  //
  string filterOutputName = filtername;
  string::size_type loc = filterOutputName.find(".filter");
  if (loc != string::npos) {
    //
    // Remove the ".filter" name, which should occur on all the provided filters
    //
    filterOutputName = filtername.substr(0, loc);
  }

  Filter *filter = readFilter(filtername);

  double sum = 0.0;
  int samples = 0;

  for (int inNum = 2; inNum < argc; inNum++) {
    string inputFilename = argv[inNum];
    string outputFilename = "filtered-" + filterOutputName + "-" + inputFilename;
    struct cs1300bmp *input = new struct cs1300bmp;
    struct cs1300bmp *output = new struct cs1300bmp;
    int ok = cs1300bmp_readfile( (char *) inputFilename.c_str(), input);

    if ( ok ) {
      double sample = applyFilter(filter, input, output);
      sum += sample;
      samples++;
      cs1300bmp_writefile((char *) outputFilename.c_str(), output);
    }
    delete input;
    delete output;
  }
  fprintf(stdout, "Average cycles per sample is %f\n", sum / samples);

}

struct Filter *
readFilter(string filename)
{
  ifstream input(filename.c_str());

  if ( ! input.bad() ) {
    int size = 0;
    input >> size;
    Filter *filter = new Filter(size);
    int div;
    input >> div;
    filter -> setDivisor(div);
    for (int i=0; i < size; i++) {
      for (int j=0; j < size; j++) {
        int value;
        input >> value;
        filter -> set(i,j,value);
      }
    }
    return filter;
  } else {
    cerr << "Bad input in readFilter:" << filename << endl;
    exit(-1);
  }
}


double
applyFilter(struct Filter *filter, cs1300bmp *input, cs1300bmp *output)
{
    long long cycStart, cycStop;
    cycStart = rdtscll();

    int columns = (input->width) -  1;
    int rows = (input->height) - 1;
    output -> width = columns + 1;
    output -> height = rows + 1;
    

    // Added read-write registers, and local variables, outside of loops
    // Store values into individual variables to reduce search time
    int size = filter -> getSize();
    int accum1, accum2, accum3, accum4, accum5, accum6, accum7, accum8, accum9 = 0;
    float divisor = filter -> getDivisor();
    int *arrayStart = (filter -> get(0, 0));
    int i = 0;
    
    // The following nested for loops allow for multiple processors at once (parallel)
    // OpenMP to run multiple processors at once
    #pragma omp parallel for
    for(int row = 1; row < rows; row++) {
        for(int col = 1; col < columns; col++) {
            accum1 = 0;
            accum2 = 0;
            accum3 = 0;
            accum4 = 0;
            accum5 = 0;
            accum6 = 0;
            accum7 = 0;
            accum8 = 0;
            accum9 = 0;
            
            // First unroll
	    // Repeat steps for each unroll and add +1 to each subsequent unroll
	    // i represents the width
            accum1 += (input -> color[0][row + i - 1][col - 1] * arrayStart[i * size]);
            accum2 += (input -> color[0][row + i][col - 1] * arrayStart[(i + 1) * size]);
            accum3 += (input -> color[0][row + i + 1][col - 1] * arrayStart[(i + 2) * size]);
            accum4 += (input -> color[1][row + i - 1][col - 1] * arrayStart[i * size]);
            accum5 += (input -> color[1][row + i][col - 1] * arrayStart[(i + 1) * size]);
            accum6 += (input -> color[1][row + i + 1][col - 1] * arrayStart[(i + 2) * size]);
            accum7 += (input -> color[2][row + i - 1][col - 1] * arrayStart[i * size]);
            accum8 += (input -> color[2][row + i][col - 1] * arrayStart[(i + 1) * size]);
            accum9 += (input -> color[2][row + i + 1][col - 1] * arrayStart[(i + 2) * size]);

            // Second unroll        
            accum1 += (input -> color[0][row + i - 1][col] * arrayStart[i * size + 1]);
            accum2 += (input -> color[0][row + i][col] * arrayStart[(i + 1) * size + 1]);
            accum3 += (input -> color[0][row + i + 1][col] * arrayStart[(i + 2) * size + 1]);
            accum4 += (input -> color[1][row + i - 1][col] * arrayStart[i * size + 1]);
            accum5 += (input -> color[1][row + i][col] * arrayStart[(i + 1) * size + 1]);
            accum6 += (input -> color[1][row + i + 1][col] * arrayStart[(i + 2) * size + 1]);
            accum7 += (input -> color[2][row + i - 1][col] * arrayStart[i * size + 1]);
            accum8 += (input -> color[2][row + i][col] * arrayStart[(i + 1) * size + 1]);
            accum9 += (input -> color[2][row + i + 1][col] * arrayStart[(i + 2) * size + 1]);
                 
            // Third unroll   
            accum1 += (input -> color[0][row + i - 1][col + 1] * arrayStart[i * size + 2]);
            accum2 += (input -> color[0][row + i][col + 1] * arrayStart[(i + 1) * size + 2]);
            accum3 += (input -> color[0][row + i + 1][col + 1] * arrayStart[(i + 2) * size + 2]);
            accum4 += (input -> color[1][row + i - 1][col + 1] * arrayStart[i * size + 2]);
            accum5 += (input -> color[1][row + i][col + 1] * arrayStart[(i + 1) * size + 2]);
            accum6 += (input -> color[1][row + i + 1][col + 1] * arrayStart[(i + 2) * size + 2]);
            accum7 += (input -> color[2][row + i - 1][col + 1] * arrayStart[i * size + 2]);
            accum8 += (input -> color[2][row + i][col + 1] * arrayStart[(i + 1) * size + 2]);
            accum9 += (input -> color[2][row + i + 1][col + 1] * arrayStart[(i + 2) * size + 2]);             
            
            accum1 = (accum1 + accum2 + accum3) / divisor;
            accum4 = (accum4 + accum5 + accum6) / divisor;
            accum7 = (accum7 + accum8 + accum9) / divisor;
            
            // Divide only if the divisor is not zero
	    // We can't go under 0 or over 255
	    // So create if stmt for each of the cases
            (accum1 < 0) ? accum1 = 0 : accum1 = accum1;
            (accum1 > 255) ? accum1 = 255 : accum1 = accum1;
            
            (accum4 < 0) ? accum4 = 0 : accum4 = accum4;
            (accum4 > 255) ? accum4 = 255 : accum4 = accum4;
            
            (accum7 < 0) ? accum7 = 0 : accum7 = accum7;
            (accum7 > 255) ? accum7 = 255 : accum7 = accum7;

	    // Store the output into accumulators
            output -> color[0][row][col] = accum1;
            output -> color[1][row][col] = accum4;
            output -> color[2][row][col] = accum7;
        }
    }
  cycStop = rdtscll();
  double diff = cycStop - cycStart;
  double diffPerPixel = diff / (output -> width * output -> height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
	  diff, diff / (output -> width * output -> height));
  return diffPerPixel;
}