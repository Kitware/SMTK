#ifndef __smtk_model_StringData_h
#define __smtk_model_StringData_h

#include "smtk/util/UUID.h"
#include "smtk/util/SystemConfig.h"

#include "sparsehash/sparse_hash_map"

#include <string>
#include <vector>

namespace smtk {
  namespace model {

    typedef std::string String;
    typedef std::vector<String> StringList;
    typedef google::sparse_hash_map<std::string,StringList> StringData;
    typedef google::sparse_hash_map<smtk::util::UUID,StringData> UUIDsToStringData;

    typedef UUIDsToStringData::iterator UUIDWithStringProperties;
    typedef StringData::iterator PropertyNameWithStrings;
    typedef StringData::const_iterator PropertyNameWithConstStrings;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_StringData_h
