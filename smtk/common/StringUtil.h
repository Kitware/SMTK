#ifndef __smtk_common_StringUtil_h
#define __smtk_common_StringUtil_h

#include "smtk/SMTKCoreExports.h"

#include <string>
#include <vector>

namespace smtk {
  namespace common {

class SMTKCORE_EXPORT StringUtil
{
public:
  static std::string& trim(std::string& s);
  static std::string& trimLeft(std::string& s);
  static std::string& trimRight(std::string& s);

  static std::string& lower(std::string& s);
  static std::string& upper(std::string& s);

  static std::vector<std::string> split(
    const std::string& s, const std::string& sep,
    bool omitEmpty, bool trim);
};

  } // namespace common
} // namespace smtk

#endif // __smtk_common_StringUtil_h
