#pragma once

template<typename IntType, int Precision>
class FixedPoint
{
public:
    typedef FixedPoint<IntType, Precision> T;
    FixedPoint(double d) : v(static_cast<IntType>(d * FACTOR)) {}
    FixedPoint(const T &rhs) : v(rhs.v) { }
    T& operator+=(const T &rhs) {v += rhs.v; return *this;}
    T& operator-=(const T &rhs) {v -= rhs.v; return *this;}
    T& operator*=(const T &rhs) {v *= rhs.v; v >>= Precision; return *this;}
    T& operator/=(const T &rhs) {v /= rhs.v; v *= FACTOR; return *this;}
    T& operator=(const T &rhs) {v = rhs.v;return *this;}
    double toDouble() const { return double(v) / FACTOR;}

    friend T operator+(T lhs, const T &rhs) {return lhs += rhs;}
    friend T operator-(T lhs, const T &rhs) {return lhs -= rhs;}
    friend T operator*(T lhs, const T &rhs) {return lhs *= rhs;}
    friend T operator/(T lhs, const T &rhs) {return lhs /= rhs;}
    friend bool operator==(const T &lhs, const T &rhs) {return lhs.v == rhs.v;}
    friend bool operator!=(const T &lhs, const T &rhs) {return lhs.v != rhs.v;}
    friend bool operator>(const T &lhs, const T &rhs) {return lhs.v > rhs.v;}
    friend bool operator<(const T &lhs, const T &rhs) {return lhs.v < rhs.v;}
    friend bool operator>=(const T &lhs, const T &rhs) {return lhs.v >= rhs.v;}
    friend bool operator<=(const T &lhs, const T &rhs) {return lhs.v <= rhs.v;}
protected:
    const int FACTOR = 1 << Precision;
    int v;
};
