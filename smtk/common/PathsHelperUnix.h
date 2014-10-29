#ifndef __smtk_common_PathsHelperUnix_h
#define __smtk_common_PathsHelperUnix_h

#include "smtk/SMTKCoreExports.h"

#include <set>
#include <string>

namespace smtk {
  namespace common {

class SMTKCORE_EXPORT PathsHelperUnix
{
public:
  PathsHelperUnix();

  static void AddSplitPaths(
    std::set<std::string>& splitPaths,
    const std::string& envVar);
};

  } // namespace common
} // namespace smtk

#endif // __smtk_common_PathsHelperUnix_h
