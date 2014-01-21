#ifndef __smtk_cgm_TDUUID_h
#define __smtk_cgm_TDUUID_h

#include "ToolData.hpp"

#include "CubitDefines.h"
#include "CubitGeomConfigure.h"

#include "smtk/options.h"
#include "smtk/cgmSMTKExports.h"
#include "smtk/util/UUIDGenerator.h"

#ifdef SMTK_HASH_STORAGE
#  include "sparsehash/sparse_hash_map"
#else
#  include <map>
#endif // SMTK_HASH_STORAGE

namespace cgmsmtk {
  namespace cgm {

#ifdef SMTK_HASH_STORAGE
/// Map UUIDs to CGM entity pointers
typedef google::sparse_hash_map<smtk::util::UUID,ToolDataUser*> UUIDToCGMRef;
#else
/// Map UUIDs to CGM entity pointers
typedef std::map<smtk::util::UUID,ToolDataUser*> UUIDToCGMRef;
#endif // SMTK_HASH_STORAGE

class TDUUID : public ToolData
{
public:
  TDUUID(ToolDataUser* entity, const smtk::util::UUID& uid = smtk::util::UUID());
  virtual ~TDUUID();

  smtk::util::UUID entityId() const;

  virtual ToolData* propogate(ToolDataUser* new_td_user);
  virtual ToolData* merge(ToolDataUser* other_td_user);

  static ToolDataUser* findEntityById(const smtk::util::UUID& uid);
  static int isTDUUID(const ToolData* td);
  static TDUUID* ofEntity(ToolDataUser* entity, bool createNew = true);

protected:
  smtk::util::UUID m_entityId;
  static UUIDToCGMRef s_reverseLookup;
  static smtk::util::UUIDGenerator s_uuidGenerator;

  static void checkForCollision(ToolDataUser* entity, const smtk::util::UUID& uid);
};

  } // namespace cgm
} // namespace cgmsmtk

#endif // __smtk_cgm_TDUUID_h
