#ifndef __smtk_util_UUIDGenerator_h
#define __smtk_util_UUIDGenerator_h

#include "smtk/util/UUID.h"

namespace smtk {
  namespace util {

class SMTKCORE_EXPORT UUIDGenerator
{
public:
  UUIDGenerator();
  virtual ~UUIDGenerator();

  UUID random();
  UUID null();

protected:
  class Internal;
  Internal* P;
};

  } // namespace util
} // namespace smtk

#endif // __smtk_util_UUIDGenerator_h
