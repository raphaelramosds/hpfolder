CXX = g++

CXXFLAGS= -Wall -Wextra -fopenmp -O3 # -g -pg

LIBS = -lm -lmpascalops #-lGL -lGLU -lglut -lboost_system -lboost_thread

INCLUDE_DIRS = $$HOME/.local/include $$HOME/projetos/pascal-releases-master/include

INCLUDE = $(patsubst %,-I%,$(INCLUDE_DIRS))

TARGET = hpfolder

OBJS = $(subst .cpp,.o,$(wildcard *.cpp))

all: $(TARGET)

hpfolder: $(OBJS)
	g++ $(CXXFLAGS) -o $(TARGET) $(OBJS) $(INCLUDE) $(LIBS)

# hpfolder deps:

Conformation.o: Conformation.cpp Conformation.hpp Protein.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $< $(LIBS)

main.o: main.cpp Conformation.hpp Protein.hpp Population.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $< $(LIBS)

Population.o: Population.cpp Population.hpp Protein.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $< $(LIBS)

Protein.o: Protein.cpp Protein.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $< $(LIBS)

clean:
	rm -f *.o $(TARGET) *.out
