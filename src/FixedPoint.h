#pragma once

template <typename IntType> class FixedPoint
{
    IntType rep_;
    public:
    FixedPoint(IntType i=0) : rep_(i) {}

    IntType& operator+=(IntType const &rhs)
    { rep_ += rhs.rep_; return *this; }

    bool operator<(IntType const &rhs) const
    { return rep_ < rhs.rep_; }

    IntType operator+(IntType const &lhs, IntType const &rhs)
    { FixedPoint tmp = lhs; tmp += rhs; return tmp; }

    bool operator>=(IntType const &lhs, IntType const &rhs)
    { return !(lhs < rhs); }
};
