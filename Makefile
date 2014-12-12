
EIGEN_DIR = ~/workspace/Eigen/Eigen
FLAGS = -Wall

all : graddes

graddes : gradient_descent.cpp
	g++ gradient_descent.cpp $(FLAGS) -o graddes

.PHONY : clean

clean :
	rm graddes