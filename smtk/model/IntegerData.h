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

#include "sparsehash/sparse_hash_map"

#include <string>
#include <vector>

namespace smtk {
  namespace model {

    typedef long Integer;
    typedef std::vector<long> IntegerList;
    typedef google::sparse_hash_map<std::string,IntegerList> IntegerData;
    typedef google::sparse_hash_map<smtk::common::UUID,IntegerData> UUIDsToIntegerData;

    typedef UUIDsToIntegerData::iterator UUIDWithIntegerProperties;
    typedef IntegerData::iterator PropertyNameWithIntegers;
    typedef IntegerData::const_iterator PropertyNameWithConstIntegers;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_IntegerData_h
