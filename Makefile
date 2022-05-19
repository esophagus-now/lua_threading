all: thing.so clean-deps

CXX = clang++
override CXXFLAGS += -g -O0 -Wno-everything -fPIC

SRCS = $(shell find . -name '.ccls-cache' -type d -prune -o -type f -name '*.cpp' -print | sed -e 's/ /\\ /g')
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

%.d: %.cpp
	@set -e; rm -f "$@"; \
	$(CXX) -MM $(CXXFLAGS) "$<" > "$@.$$$$"; \
	sed 's,\([^:]*\)\.o[ :]*,\1.o \1.d : ,g' < "$@.$$$$" > "$@"; \
	rm -f "$@.$$$$"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c "$<" -o "$@"

include $(DEPS)

thing.so: $(OBJS)
	$(CXX) -shared $(CXXFLAGS) $(OBJS) -o "$@" -pthread

clean:
	rm -f $(OBJS) $(DEPS) thing.so

clean-deps:
	rm -f $(DEPS)
	rm -f $(OBJS)