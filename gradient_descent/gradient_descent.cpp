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
	VectorXd f(D), x(D), r(D);

	for (int i = 0; i < D; i++)
		for (int j = 0; j < D; j++)
			cin >> m(i,j);
	for (int i = 0; i < D; i++){
		cin >> f(i);
		x(i) = 1;//x0 - initial approximation
	}

	//cout << "m =" << endl << m << "\n------------" << endl;
	//cout << "f =" << endl << f << "\n------------" << endl;

	// Gradient descent method
	r = m*x -f;// r0

	int Grad_iter = 0;
	while (r.norm() > eps){
		double t = r.dot(r)/r.dot(m*r);
		VectorXd tmpx(D);
		tmpx = x - t*r;
		x = tmpx;
		r = m*x -f;
		Grad_iter++;
		//cout << "x" << Grad_iter << " =" << endl << x << endl << r.norm() << "\n------------" << endl;
		if (Grad_iter > 10000){
			cout << "Rashodimsya!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			break;
		}
	}
	
	//cout << "x" << Grad_iter << " =" << endl << x << "\n------------" << endl;

	return 0;
}
