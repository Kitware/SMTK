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
#include "smtk/bridge/cgm/Exports.h"
#include "smtk/common/UUID.h"

#include <map>

namespace smtk
{
namespace bridge
{
namespace cgm
{

/// Map UUIDs to CGM entity pointers
typedef std::map<smtk::common::UUID, ToolDataUser*> UUIDToCGMRef;

class SMTKCGMSESSION_EXPORT TDUUID : public ToolData
{
public:
  TDUUID(ToolDataUser* entity, const smtk::common::UUID& uid = smtk::common::UUID());
  ~TDUUID() override;

  smtk::common::UUID entityId() const;

  ToolData* propogate(ToolDataUser* new_td_user) override;
  ToolData* merge(ToolDataUser* other_td_user) override;

  static ToolDataUser* findEntityById(const smtk::common::UUID& uid);
  static int isTDUUID(const ToolData* td);
  static TDUUID* ofEntity(ToolDataUser* entity, bool createNew = true);

protected:
  smtk::common::UUID m_entityId;
  static UUIDToCGMRef s_reverseLookup;

  static void checkForCollision(ToolDataUser* entity, const smtk::common::UUID& uid);
};

} // namespace cgm
} //namespace bridge
} // namespace smtk
#endif // __smtk_session_cgm_TDUUID_h
