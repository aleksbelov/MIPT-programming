#include <stdio.h>
#include <iostream>
#include <math.h>

double f(double x){
	return atan(x);
}

using namespace std;

int main(){
	double h, eps;
	int N;
	cin >> N;
	cout.precision(10);
	cout.width(5);
	cout.setf(ios::left);
	double a = 0, b = 2;
	
	cout << "Результат интегрирования:" << endl;
	for (int n = 1; n <= N; n ++){
		double integral = 0;
		h = (b - a)/n;
		eps = (b - a)/2*h*h/24;
		for (int i = 0; i < n; i++)
			integral += f(a + i*h + h/2);
		integral *= h;
		cout << "N = " << n << "; h = " << h << "; result: " << integral
			 << " eps = " << eps << endl;
		
	}
	return 0;
}