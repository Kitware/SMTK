//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_cgm_TDUUID_h
#define __smtk_session_cgm_TDUUID_h

#include "ToolData.hpp"

#include "CubitDefines.h"
#include "CubitGeomConfigure.h"

#include "smtk/Options.h"
#include "smtk/bridge/cgm/cgmSMTKExports.h"
#include "smtk/common/UUIDGenerator.h"

#ifdef SMTK_HASH_STORAGE
#  if defined(_MSC_VER) // Visual studio
#    pragma warning (push)
#    pragma warning (disable : 4996)  // Overeager "unsafe" parameter check
#  endif
#  include "sparsehash/sparse_hash_map"
#  if defined(_MSC_VER) // Visual studio
#    pragma warning (pop)
#  endif
#else
#  include <map>
#endif // SMTK_HASH_STORAGE

namespace smtk {
  namespace bridge {
    namespace cgm {

#ifdef SMTK_HASH_STORAGE
/// Map UUIDs to CGM entity pointers
typedef google::sparse_hash_map<smtk::common::UUID,ToolDataUser*> UUIDToCGMRef;
#else
/// Map UUIDs to CGM entity pointers
typedef std::map<smtk::common::UUID,ToolDataUser*> UUIDToCGMRef;
#endif // SMTK_HASH_STORAGE

class CGMSMTK_EXPORT TDUUID : public ToolData
{
public:
  TDUUID(ToolDataUser* entity, const smtk::common::UUID& uid = smtk::common::UUID());
  virtual ~TDUUID();

  smtk::common::UUID entityId() const;

  virtual ToolData* propogate(ToolDataUser* new_td_user);
  virtual ToolData* merge(ToolDataUser* other_td_user);

  static ToolDataUser* findEntityById(const smtk::common::UUID& uid);
  static int isTDUUID(const ToolData* td);
  static TDUUID* ofEntity(ToolDataUser* entity, bool createNew = true);

protected:
  smtk::common::UUID m_entityId;
  static UUIDToCGMRef s_reverseLookup;
  static smtk::common::UUIDGenerator s_uuidGenerator;

  static void checkForCollision(ToolDataUser* entity, const smtk::common::UUID& uid);
};

    } // namespace cgm
  } //namespace bridge
} // namespace smtk
#endif // __smtk_session_cgm_TDUUID_h
