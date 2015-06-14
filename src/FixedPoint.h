#pragma once

template<typename IntType, const int FRACTION_BITS>
class FixedPoint
{
public:
    typedef FixedPoint<IntType, FRACTION_BITS> T;

    FixedPoint(){}
    FixedPoint(double d) {v = (IntType)(d*scale);}
    FixedPoint(const T &rhs) : v(rhs.v) { }
    T& operator=(const T &rhs) {v = rhs.v;return *this;}
    double toDouble() const { return double(v)/scale;}


    friend T operator+(T lhs, const T &rhs) {T r;r.v = lhs.v + rhs.v;return r;}
    friend T operator-(T lhs, const T &rhs) {T r;r.v = lhs.v - rhs.v;return r;}
    friend T operator*(T lhs, const T &rhs) {T r;r.v = (lhs.v * rhs.v) / scale;return r;}
    friend T operator/(T lhs, const T &rhs) {T r;r.v = (lhs.v * scale)/rhs.v;return r;}
    friend bool operator==(const T &lhs, const T &rhs) {return lhs.v == rhs.v;}
protected:
    int v;
    enum
    {
        scale  = 1<<FRACTION_BITS,
    };
};
