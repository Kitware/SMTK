//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_StringData_h
#define __smtk_model_StringData_h

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

    typedef std::string String;
    typedef std::vector<String> StringList;
#ifdef SMTK_HASH_STORAGE
    typedef google::sparse_hash_map<std::string,StringList> StringData;
    typedef google::sparse_hash_map<smtk::common::UUID,StringData> UUIDsToStringData;
#else // SMTK_HASH_STORAGE
    typedef std::map<std::string,StringList> StringData;
    typedef std::map<smtk::common::UUID,StringData> UUIDsToStringData;
#endif // SMTK_HASH_STORAGE

    typedef UUIDsToStringData::iterator UUIDWithStringProperties;
    typedef StringData::iterator PropertyNameWithStrings;
    typedef StringData::const_iterator PropertyNameWithConstStrings;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_StringData_h
