#ifndef __smtk_model_testing_helpers_h
#define __smtk_model_testing_helpers_h

#include "smtk/util/UUID.h"
#include "smtk/model/Storage.h"

#include <ostream>

namespace smtk {
  namespace model {
    namespace testing {

smtk::util::UUIDArray createTet(smtk::model::Storage& sm);

class hexconst
{
public:
  hexconst(long long c) { m_val = c; }
  long long m_val;
};

std::ostream& operator << (std::ostream& os, const hexconst& x);

/// A timer for benchmarking.
class Timer
{
public:
  Timer();
  ~Timer();

  void mark();
  double elapsed();

protected:
  class Internal;
  Internal* P;
};

    } // namespace testing
  } // namespace model
} // namespace smtk

#endif // __smtk_model_testing_helpers_h
