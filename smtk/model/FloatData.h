//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_FloatData_h
#define __smtk_model_FloatData_h

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

    typedef double Float;
    typedef std::vector<Float> FloatList;
#ifdef SMTK_HASH_STORAGE
    typedef google::sparse_hash_map<std::string,FloatList> FloatData;
    typedef google::sparse_hash_map<smtk::common::UUID,FloatData> UUIDsToFloatData;
#else // SMTK_HASH_STORAGE
    typedef std::map<std::string,FloatList> FloatData;
    typedef std::map<smtk::common::UUID,FloatData> UUIDsToFloatData;
#endif // SMTK_HASH_STORAGE

    typedef UUIDsToFloatData::iterator UUIDWithFloatProperties;
    typedef FloatData::iterator PropertyNameWithFloats;
    typedef FloatData::const_iterator PropertyNameWithConstFloats;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_FloatData_h
