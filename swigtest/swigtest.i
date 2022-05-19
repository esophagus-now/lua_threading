%module swigtest
%{
#include "swigtest.h"
#include <cmath>
%}

%include "swigtest.h"

float atan2(float y, float x);
