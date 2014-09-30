//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_bridge_cgm_Bridge_h
#define __smtk_bridge_cgm_Bridge_h

#include "smtk/bridge/cgm/cgmSMTKExports.h"
#include "smtk/bridge/cgm/PointerDefs.h"
#include "smtk/model/Bridge.h"

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
class RefEntity;
class SenseEntity;
class GroupingEntity;

namespace smtk {
  namespace bridge {
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
  */
class CGMSMTK_EXPORT Bridge : public smtk::model::Bridge
{
public:
  smtkDeclareModelingKernel();
  typedef smtk::shared_ptr<Bridge> Ptr;
  typedef smtk::model::BridgedInfoBits BridgedInfoBits;
  static BridgePtr create();
  virtual ~Bridge();

  virtual BridgedInfoBits allSupportedInformation() const;

  static bool addManagerEntityToCGM(const smtk::model::Cursor& ent);

protected:
  friend class ImportSolid;

  Bridge();

  virtual BridgedInfoBits transcribeInternal(
    const smtk::model::Cursor& entity, BridgedInfoBits requestedInfo);

  BridgedInfoBits addCGMEntityToManager(const smtk::model::Cursor& entity, RefEntity* refEnt, BridgedInfoBits requestedInfo);
  BridgedInfoBits addCGMEntityToManager(const smtk::model::Cursor& entity, GroupingEntity* refEnt, BridgedInfoBits requestedInfo);
  BridgedInfoBits addCGMEntityToManager(const smtk::model::Cursor& entity, SenseEntity* refEnt, BridgedInfoBits requestedInfo);

  BridgedInfoBits addBodyToManager(const smtk::model::ModelEntity&, Body*, BridgedInfoBits requestedInfo);
  BridgedInfoBits addVolumeUseToManager(const smtk::model::VolumeUse&, CoVolume*, BridgedInfoBits requestedInfo);
  BridgedInfoBits addFaceUseToManager(const smtk::model::FaceUse&, CoFace*, BridgedInfoBits requestedInfo);
  BridgedInfoBits addEdgeUseToManager(const smtk::model::EdgeUse&, CoEdge*, BridgedInfoBits requestedInfo);
  BridgedInfoBits addVertexUseToManager(const smtk::model::VertexUse&, CoVertex*, BridgedInfoBits requestedInfo);
  BridgedInfoBits addShellToManager(const smtk::model::Shell&, ::Shell*, BridgedInfoBits requestedInfo);
  BridgedInfoBits addLoopToManager(const smtk::model::Loop&, ::Loop*, BridgedInfoBits requestedInfo);
  BridgedInfoBits addChainToManager(const smtk::model::Chain&, ::Chain*, BridgedInfoBits requestedInfo);
  BridgedInfoBits addVolumeToManager(const smtk::model::Volume&, RefVolume*, BridgedInfoBits requestedInfo);
  BridgedInfoBits addFaceToManager(const smtk::model::Face&, RefFace*, BridgedInfoBits requestedInfo);
  BridgedInfoBits addEdgeToManager(const smtk::model::Edge&, RefEdge*, BridgedInfoBits requestedInfo);
  BridgedInfoBits addVertexToManager(const smtk::model::Vertex&, RefVertex*, BridgedInfoBits requestedInfo);
  BridgedInfoBits addGroupToManager(const smtk::model::GroupEntity&, RefGroup*, BridgedInfoBits requestedInfo);

  static void colorPropFromIndex(smtk::model::Cursor&, int);

private:
  Bridge(const Bridge&); // Not implemented.
  void operator = (const Bridge&); // Not implemented.
};

} // namespace cgm
  } //namespace bridge
} // namespace smtk

#endif // __smtk_bridge_cgm_Bridge_h
