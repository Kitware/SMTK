#ifndef __smtk_common_UUIDGenerator_h
#define __smtk_common_UUIDGenerator_h

#include "smtk/common/UUID.h"

namespace smtk {
  namespace common {

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

  } // namespace common
} // namespace smtk

#endif // __smtk_common_UUIDGenerator_h
