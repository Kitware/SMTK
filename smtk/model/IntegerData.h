//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_IntegerData_h
#define __smtk_model_IntegerData_h

#include "smtk/SystemConfig.h"

#include "smtk/common/UUID.h"

#ifdef SMTK_HASH_STORAGE
#  if defined(_MSC_VER) // Visual studio
#    pragma warning (push)
#    pragma warning (disable : 4996)  // Overeager "unsafe" parameter check
#  endif
#  include "sparsehash/sparse_hash_map"
#  if defined(_MSC_VER) // Visual studio
#    pragma warning (pop)
#  endif
#else // SMTK_HASH_STORAGE
#  include <map>
#endif // SMTK_HASH_STORAGE

#include <string>
#include <vector>

namespace smtk {
  namespace model {

    typedef long Integer;
    typedef std::vector<long> IntegerList;
#ifdef SMTK_HASH_STORAGE
    typedef google::sparse_hash_map<std::string,IntegerList> IntegerData;
    typedef google::sparse_hash_map<smtk::common::UUID,IntegerData> UUIDsToIntegerData;
#else // SMTK_HASH_STORAGE
    typedef std::map<std::string,IntegerList> IntegerData;
    typedef std::map<smtk::common::UUID,IntegerData> UUIDsToIntegerData;
#endif // SMTK_HASH_STORAGE

    typedef UUIDsToIntegerData::iterator UUIDWithIntegerProperties;
    typedef IntegerData::iterator PropertyNameWithIntegers;
    typedef IntegerData::const_iterator PropertyNameWithConstIntegers;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_IntegerData_h
