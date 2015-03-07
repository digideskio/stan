#ifndef STAN__MATH__REV__SCAL__FUN__CALCULATE_CHAIN_HPP
#define STAN__MATH__REV__SCAL__FUN__CALCULATE_CHAIN_HPP

#include <valarray>

namespace stan {
  namespace agrad {
    inline double calculate_chain(const double& x, const double& val) {
      return std::exp(x - val); // works out to inv_logit(x)
    }
  }
}
#endif
