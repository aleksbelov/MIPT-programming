#include <stdio.h>
#include <iostream>
#include <math.h>

double f(double x){
	if (x == 0)
		return 0;
	return sin(x)/sqrt(x);
}

using namespace std;

int main(){
	double h, eps;
	int N;
	cin >> eps;
	cout.precision(10);
	cout.width(5);
	cout.setf(ios::left);
	double a = 0 /*exp(log(eps/2.0)*2/3)*0.9*/, b = exp(log(6/eps)*2)*0.05;
	h = exp(log(2880*2*eps/b)/4);
	N = round(b/h);
	cout << "a = " << a << "\nb = " << b << "\nN = " << N << endl;
	int n = 0;
	cout << "Результат интегрирования:" << endl;
	double integral = 0;
	while (a + h < b){
		integral = integral + f(a)+4*f(a+h/2)+f(a+h);
		a = a + h;
		n++;
	}
	
	integral = integral * h / 6;
	
	cout << "\nN = " << n << "; h = " << h << "; result: " << integral
		 << " eps = " << eps << endl;
	
	return 0;
}