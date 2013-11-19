#ifndef __smtk_model_StringData_h
#define __smtk_model_StringData_h

#include "smtk/util/UUID.h"

#include "sparsehash/sparse_hash_map"

#include <string>
#include <vector>

namespace smtk {
  namespace model {

    typedef std::vector<std::string> StringList;
    typedef google::sparse_hash_map<std::string,StringList> StringData;
    typedef google::sparse_hash_map<smtk::util::UUID,StringData> UUIDsToStringData;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_StringData_h
