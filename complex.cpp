#include <math.h>
namespace{
	class Complex {
		double rl, im; //real, imaginary
	public:
		Complex() {

		}
		Complex(double r, double i) {
			rl = r;
			im = i;
		}
		Complex& operator = (Complex& val){ return val; };
		Complex(Complex& val) {
			rl = val.get_rl();
			im = val.get_im();
		}

		void set_vals(double, double);
		const double get_rl();
		const double get_im();
		double get_mag();
		static Complex &mult(Complex& cm1, Complex& cm2) {
			double a = cm1.get_rl();
			double b = cm1.get_im();
			double c = cm2.get_rl();
			double d = cm2.get_im();
			double r = a*c - b*d;
			double i = b*c + a*d;
			Complex& out = Complex(r, i);
			return out;
		}
		static Complex &add(Complex& cm1, Complex& cm2) {
			double r = cm1.get_rl() + cm2.get_rl();
			double i = cm1.get_im() + cm2.get_im();
			Complex& out = Complex(r, i);
			return out;
		}
	};

	void Complex::set_vals(double real, double imaginary) {
		rl = real;
		im = imaginary;
	}
	const double Complex::get_rl() {
		return rl;
	}

	const double Complex::get_im() {
		return im;
	}

	double Complex::get_mag() {
		double mag = im*im + rl*rl;
		return sqrt(mag);
	}
}
