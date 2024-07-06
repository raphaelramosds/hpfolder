CXX=g++

CXXFLAGS=-O3

LIBS=-lGL -lGLU -lglut -lboost_system -lboost_thread -pg

TARGET=hpfolder

OBJS=Protein.o Conformation.o Population.o main.o

all: $(TARGET)

Protein.o: Protein.hpp
	$(CXX) $(CXXFLAGS) -c Protein.cpp

Conformation.o: Protein.o Conformation.hpp
	$(CXX) $(CXXFLAGS) -c Conformation.cpp

Population.o: Protein.o Population.hpp
	$(CXX) $(CXXFLAGS) -c Population.cpp

main.o: Protein.o Conformation.o Population.o
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

hpfolder: $(OBJS)
	g++ -o $(TARGET) $(OBJS) $(LIBS)

clean:
	rm *.o $(TARGET) *.out