//
//  integers.h
//  
//
//  Created by Philip Gressman on 12/21/18.
//

#ifndef integers_h
#define integers_h


int complexity(int thenum) {
    int tally = 1, factor = 1, logfac = 1;
    if (thenum < 0) {thenum = -thenum;}
    if (thenum == 0) return 0;
    
    // while (factor < thenum) {factor = factor * 2; logfac++;}
    
    while (thenum % 10 == 0) {tally += 1; thenum = thenum / 10;}

    
    return tally * thenum;
}

int gcd(int i,int j)
{
    if (i==0||(j==0)) return 0;
    if (i < 0) return gcd(-i,j);
    if (j < 0) return gcd(i,-j);
    if (i > j) return gcd (j,i);
    if (i==j) return i;
    if ((j%i)==0) return i;
    return gcd(i,j % i); // was previously j - i
}

int small_power_factors(int i, int power)
{
    // Assumes power >= 1
    int j,k,base;
    for(j=2;j<30;j++) {
        base = 1;
        for(k=0;k<power;k++) {base = base * j; if (base > 1000000) {base = 1; k = power;}}
        if ((base != 1) && (i % base == 0)) return j * small_power_factors(i/base,power);
    }
    return 1;
}

class Number {
    public:
    Number() {numerator = 0; denominator = 1;}
    Number(int a) {numerator = a; denominator = 1;}
    Number operator+(Number b) {
        Number result;
        result.numerator = (this->numerator) * b.denominator + (this->denominator) * b.numerator;
        result.denominator = (this->denominator) * b.denominator;
        result.reduce();
        return result;
    }
    Number operator-(Number b) {
        Number result;
        result.numerator = (this->numerator) * b.denominator - (this->denominator) * b.numerator;
        result.denominator = (this->denominator) * b.denominator;
        result.reduce();
        return result;
    }
    Number operator*(Number b) {
        Number result;
        result.numerator = (this->numerator) * b.numerator;
        result.denominator = (this->denominator) * b.denominator;
        result.reduce();
        return result;
    }
    Number operator/(Number b) {
        Number result;
        result.numerator = (this->numerator) * b.denominator;
        result.denominator = (this->denominator) * b.numerator;
        result.reduce();
        return result;
    }
    Number operator^(Number b) {
        Number result = 1;
        int counter,times,j;
        if (numerator == 0) {return 0;}
        if (b.numerator == 0) {return result;}
        times = b.numerator; if (times < 0) {times = - times;}
        for(j=0;j < times;j++) {result.numerator *= numerator; result.denominator *= denominator;}
        if (b.numerator < 0) {result.reciprocal();}
        if (b.denominator != 1) {
            result.numerator = small_power_factors(result.numerator,b.denominator);
            result.denominator = small_power_factors(result.denominator,b.denominator);
        }
        return result;
    }
    Number operator%(Number b) {
        Number result;
        result.numerator = (numerator * b.denominator) % (b.numerator * denominator);
        result.denominator = denominator * b.denominator;
        result.reduce();
        return result;
    }
    Number abs() {
        Number result;
        result.numerator = numerator; result.denominator = denominator;
        if (result.numerator < 0) {result.numerator = -numerator;}
        return result;
    }
    Number complexity() {
        Number result;
        result.numerator = numerator; if (result.numerator < 0) {result = -numerator;}
        result.numerator += denominator;
        result.denominator = 1;
        return result;
    }
    void reciprocal() {int temp = numerator; numerator = denominator; denominator = temp;}
    void reduce() {
        if (denominator < 0){denominator = - denominator; numerator = - numerator;}
        if (numerator == 0) {denominator = 1;}
        if (numerator != 0) {
            int result = gcd(numerator,denominator);
            if (result != 0) {numerator = numerator / result; denominator = denominator/result;}
        }
        if ((denominator == 0)&&(numerator > 0)) {numerator =1;}
        if ((denominator == 0)&&(numerator < 0)) {numerator=-1;}
    }
    bool operator<(Number b) {
        return (numerator * b.denominator < b.numerator * denominator);
    }
    bool operator>(Number b) {
        return (numerator * b.denominator > b.numerator * denominator);
    }
    bool operator<=(Number b) {
        return (numerator * b.denominator <= b.numerator * denominator);
    }
    bool operator>=(Number b) {
        return (numerator * b.denominator >= b.numerator * denominator);
    }
    bool operator==(Number b) {return (numerator * b.denominator == denominator * b.numerator);}
    bool operator!=(Number b) {return (numerator * b.denominator != denominator * b.numerator);}
    int floor() {
        return numerator / denominator;
    }
    string to_string() {
        reduce();
        if (denominator == 1) { return ::to_string(numerator); }
        return ::to_string(numerator) + "/" + ::to_string(denominator);
        // return "\\frac{"+::to_string(numerator) + "}{" + ::to_string(denominator) + "}";
    }
    string to_sympy_string() {
        reduce();
        if (denominator == 1) { return ::to_string(numerator); }
        return "Rational(" + ::to_string(numerator) + "," + ::to_string(denominator) + ")";
        // return "\\frac{"+::to_string(numerator) + "}{" + ::to_string(denominator) + "}";
    }
    bool is_integer() {return denominator == 1;}
    bool is_undefined() {return denominator == 0;}
    operator int () {return numerator;}
    long numerator;
    long denominator;
};



#endif /* integers_h */
