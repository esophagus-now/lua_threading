#include "swigtest.h"
#include <cmath>

float atan2(mystruct const&t) {
    return atan2(t.y, t.x);
}