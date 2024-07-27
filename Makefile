CXX=g++

CXXFLAGS=-O3 -pg # -fopenmp

LIBS=-lGL -lGLU -lglut -lboost_system -lboost_thread

TARGET=hpfolder

OBJS=Protein.o Conformation.o Population.o main.o

all: $(TARGET)

Protein.o: Protein.hpp Protein.cpp
	$(CXX) $(CXXFLAGS) -c Protein.cpp

Conformation.o: Conformation.hpp Protein.hpp Conformation.cpp
	$(CXX) $(CXXFLAGS) -c Conformation.cpp

Population.o: Protein.hpp Population.hpp Population.cpp
	$(CXX) $(CXXFLAGS) -c Population.cpp

main.o: Protein.hpp Conformation.hpp Population.hpp main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

hpfolder: $(OBJS)
	g++ $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

clean:
	rm *.o $(TARGET) *.out
