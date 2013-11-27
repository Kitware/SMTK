#ifndef __smtk_model_IntegerData_h
#define __smtk_model_IntegerData_h

#include "smtk/util/UUID.h"
#include "smtk/util/SystemConfig.h"

#include "sparsehash/sparse_hash_map"

#include <string>
#include <vector>

namespace smtk {
  namespace model {

    typedef long Integer;
    typedef std::vector<long> IntegerList;
    typedef google::sparse_hash_map<std::string,IntegerList> IntegerData;
    typedef google::sparse_hash_map<smtk::util::UUID,IntegerData> UUIDsToIntegerData;

    typedef UUIDsToIntegerData::iterator UUIDWithIntegerProperties;
    typedef IntegerData::iterator PropertyNameWithIntegers;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_IntegerData_h
