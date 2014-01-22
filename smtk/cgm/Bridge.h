#ifndef __smtk_cgm_Bridge_h
#define __smtk_cgm_Bridge_h

#include "smtk/cgmSMTKExports.h"
#include "smtk/PublicPointerDefs.h"

class Body;
class CoVolume;
class CoFace;
class CoEdge;
class CoVertex;
class Shell;
class Loop;
class Chain;
class RefVolume;
class RefFace;
class RefEdge;
class RefVertex;
class RefGroup;

namespace cgmsmtk {
  namespace cgm {

/**\brief Methods that handle translation between CGM and SMTK instances.
  *
  * While the TDUUID class keeps a map from SMTK UUIDs to CGM ToolDataUser
  * pointers, this is not enough to handle everything SMTK provides:
  * there is no way to track cell-use or shell entities since they do
  * not inherit ToolDataUser instances. Also, some engines (e.g., facet)
  * do not appear to store some entity types (e.g., RefGroup).
  *
  * Also, simply loading a CGM file does not translate the entire model
  * into SMTK; instead, it assigns UUIDs to entities if they do not already
  * exist. This class (Bridge) provides a method for requesting the
  * entity, arrangement, and/or tessellation information for a UUID be
  * mapped into SMTK from CGM.
  *
  * Because CGM heavily relies on the singleton pattern, this
  * class only has static methods.
  */
class CGMSMTK_EXPORT Bridge
{
public:
  static bool addCGMEntityToStorage(
    const smtk::util::UUID& entity, smtk::model::StoragePtr storage);

  static bool addStorageEntityToCGM(const smtk::model::Cursor& ent);

protected:
  friend class ImportSolid;

  static void AddBodyToStorage(const smtk::util::UUID&, Body*, smtk::model::StoragePtr, bool addRels = true);
  static void AddVolumeUseToStorage(const smtk::util::UUID&, CoVolume*, smtk::model::StoragePtr, bool addRels = true);
  static void AddFaceUseToStorage(const smtk::util::UUID&, CoFace*, smtk::model::StoragePtr, bool addRels = true);
  static void AddEdgeUseToStorage(const smtk::util::UUID&, CoEdge*, smtk::model::StoragePtr, bool addRels = true);
  static void AddVertexUseToStorage(const smtk::util::UUID&, CoVertex*, smtk::model::StoragePtr, bool addRels = true);
  static void AddShellToStorage(const smtk::util::UUID&, Shell*, smtk::model::StoragePtr, bool addRels = true);
  static void AddLoopToStorage(const smtk::util::UUID&, Loop*, smtk::model::StoragePtr, bool addRels = true);
  static void AddChainToStorage(const smtk::util::UUID&, Chain*, smtk::model::StoragePtr, bool addRels = true);
  static void AddVolumeToStorage(const smtk::util::UUID&, RefVolume*, smtk::model::StoragePtr, bool addRels = true);
  static void AddFaceToStorage(const smtk::util::UUID&, RefFace*, smtk::model::StoragePtr, bool addRels = true);
  static void AddEdgeToStorage(const smtk::util::UUID&, RefEdge*, smtk::model::StoragePtr, bool addRels = true);
  static void AddVertexToStorage(const smtk::util::UUID&, RefVertex*, smtk::model::StoragePtr, bool addRels = true);
  static void AddGroupToStorage(const smtk::util::UUID&, RefGroup*, smtk::model::StoragePtr, bool addRels = true);
};

  } // namespace cgm
} // namespace cgmsmtk

#endif // __smtk_cgm_Bridge_h
