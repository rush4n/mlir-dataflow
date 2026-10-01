//===- ZeroDomain.h - The abstract domain ---------------------------------===//
//
// A four-point lattice recording whether an integer value is known to be zero.
//
//        Top          nothing is known
//       /   \
//    Zero  NonZero
//       \   /
//       Bottom       unreachable, or not yet analyzed
//
// This is the file to replace first when building a different analysis.  MLIR's
// dataflow framework asks only three things of a lattice value:
//
//   * a default constructor, which must produce the bottom element, because the
//     solver starts every value optimistically and lowers it as facts arrive;
//   * a static join(), which must be commutative, associative, idempotent, and
//     monotone -- assertions in Lattice<> check monotonicity in debug builds;
//   * operator== and print().
//
//===----------------------------------------------------------------------===//

#ifndef SIGN_DOMAIN_H
#define SIGN_DOMAIN_H

#include "llvm/Support/raw_ostream.h"

namespace sign {

enum class Kind {
  Bottom,
  Zero,
  One,
  Positive,
  Negative,
  NonNegative,
  NonPositive,
  Top
};

inline const char *name(Kind kind) {
  switch (kind) {
  case Kind::Bottom:
    return "bottom";
  case Kind::Zero:
    return "zero";
  case Kind::One:
    return "one";
  case Kind::Positive:
    return "positive";
  case Kind::Negative:
    return "negative";
  case Kind::NonNegative:
    return "nonnegative";
  case Kind::NonPositive:
    return "nonpositive";
  case Kind::Top:
    return "top";
  }
  return "top";
}

struct SignState {
  Kind kind = Kind::Bottom;

  SignState() = default;
  /* implicit */ SignState(Kind kind) : kind(kind) {}

  static SignState bottom() { return Kind::Bottom; }
  static SignState top() { return Kind::Top; }

  bool isBottom() const { return kind == Kind::Bottom; }

  static SignState join(const SignState &lhs, const SignState &rhs) {
    using K = Kind;

    static constexpr K joinTable[8][8] = {
        {K::Bottom, K::Zero, K::One, K::Positive, K::Negative, K::NonNegative,
         K::NonPositive, K::Top},

        {K::Zero, K::Zero, K::NonNegative, K::NonNegative, K::NonPositive,
         K::NonNegative, K::NonPositive, K::Top},

        {K::One, K::NonNegative, K::One, K::Positive, K::Top, K::NonNegative,
         K::Top, K::Top},

        {K::Positive, K::NonNegative, K::Positive, K::Positive, K::Top,
         K::NonNegative, K::Top, K::Top},

        {K::Negative, K::NonPositive, K::Top, K::Top, K::Negative, K::Top,
         K::NonPositive, K::Top},

        {K::NonNegative, K::NonNegative, K::NonNegative, K::NonNegative, K::Top,
         K::NonNegative, K::Top, K::Top},

        {K::NonPositive, K::NonPositive, K::Top, K::Top, K::NonPositive, K::Top,
         K::NonPositive, K::Top},

        {K::Top, K::Top, K::Top, K::Top, K::Top, K::Top, K::Top, K::Top},
    };

    unsigned lhsIndex = static_cast<unsigned>(lhs.kind);
    unsigned rhsIndex = static_cast<unsigned>(rhs.kind);

    return SignState(joinTable[lhsIndex][rhsIndex]);
  }

  bool operator==(const SignState &other) const { return kind == other.kind; }
  bool operator!=(const SignState &other) const { return kind != other.kind; }

  void print(llvm::raw_ostream &os) const { os << name(kind); }
};

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                     const SignState &state) {
  state.print(os);
  return os;
}

} // namespace sign

#endif
