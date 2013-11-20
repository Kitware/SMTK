#ifndef __smtk_model_testing_helpers_h
#define __smtk_model_testing_helpers_h

#include "smtk/util/UUID.h"
#include "smtk/model/Storage.h"

namespace smtk {
  namespace model {
    namespace testing {

smtk::util::UUIDArray createTet(smtk::model::Storage& sm);

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
