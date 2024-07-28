CXX = g++

CXXFLAGS= -O3 -Wall -Wextra -fopenmp # -pg -g

LIBS = -lGL -lGLU -lglut -lboost_system -lboost_thread

TARGET = hpfolder

OBJS = $(subst .cpp,.o,$(wildcard *.cpp))

all: $(TARGET)

hpfolder: $(OBJS)
	g++ $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# hpfolder deps:

Conformation.o: Conformation.cpp Conformation.hpp Protein.hpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

main.o: main.cpp Conformation.hpp Protein.hpp Population.hpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

Population.o: Population.cpp Population.hpp Protein.hpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

Protein.o: Protein.cpp Protein.hpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f *.o $(TARGET) *.out
