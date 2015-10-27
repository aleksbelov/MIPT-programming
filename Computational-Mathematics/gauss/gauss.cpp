#include <Eigen/Dense>
#include <Eigen/Core>
#include <iostream>


using namespace Eigen;
using namespace std;

int main(){
	// Input
	int D;
	double eps = 0;
	cin >> D >> eps;

	MatrixXd m(D,D);
	VectorXd f(D), x(D);

	for (int i = 0; i < D; i++)
		for (int j = 0; j < D; j++)
			cin >> m(i,j);
	for (int i = 0; i < D; i++){
		cin >> f(i);
		x(i) = 0;
	}

	//cout << "m =" << endl << m << "\n------------" << endl;
	//cout << "f =" << endl << f << "\n------------" << endl;

	// Direct flow
	
	double weight = 0;
	for (int i = 0; i < D; i++)// вычитаем эту строку
		for (int j = i+1; j < D; j++){//из этих строк
			if (m(i,i) != 0) {
				weight = m(j,i)/m(i,i);// вес
				//cout << "w =" << weight << " " << m(j,i) << "/" << m(i,i) << " "
				//		<< i << " " << j << "\n------------" << endl;
				for (int k = i; k < D; k++)// вычитание строк
					m(j,k) = m(j,k) - m(i,k)*weight;
			
				f(j) = f(j) - f(i)*weight;
			}
		}
	
	// Reversal flow
	
	for (int i = D-1; i >=0; i--){
		for (int j = i+1; j < D; j++)
			x(i) = x(i) + m(i,j)*x(j);
		x(i) = (f(i)- x(i))/m(i,i);
	}
	//cout << "x" << " =" << endl << x << "\n------------" << endl;

	return 0;
}
