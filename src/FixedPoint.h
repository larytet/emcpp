#pragma once

template <typename IntType> class D
{
    int rep_;
    public:
    D(int i=0) : rep_(i) {/*empty*/}

    IntType& operator+=(IntType const& rhs)
    { rep_ += rhs.rep_; return *this; }
    bool operator<(IntType const& rhs) const
    { return rep_ < rhs.rep_; }

    IntType operator+(IntType const& lhs, IntType const& rhs)
    { D tmp = lhs; tmp += rhs; return tmp; }

    bool operator>=(IntType const& lhs, IntType const& rhs)
    { return !(lhs < rhs); }
};
