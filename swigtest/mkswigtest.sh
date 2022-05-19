swig -lua swigtest.i
g++ -fPIC -shared -o swigtest.so swigtest_wrap.c swigtest.cpp