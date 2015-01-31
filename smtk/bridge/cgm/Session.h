//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_cgm_Session_h
#define __smtk_session_cgm_Session_h

#include "smtk/bridge/cgm/cgmSMTKExports.h"
#include "smtk/bridge/cgm/PointerDefs.h"
#include "smtk/model/Session.h"

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
template<class X> class DLIList;

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
  * exist. This class (Session) provides a method for requesting the
  * entity, arrangement, and/or tessellation information for a UUID be
  * mapped into SMTK from CGM.
  */
class CGMSMTK_EXPORT Session : public smtk::model::Session
{
public:
  smtkTypeMacro(Session);
  smtkSuperclassMacro(smtk::model::Session);
  smtkSharedFromThisMacro(smtk::model::Session);
  smtkCreateMacro(smtk::model::Session);
  smtkDeclareModelingKernel();
  typedef smtk::model::SessionInfoBits SessionInfoBits;
  virtual ~Session();

  virtual SessionInfoBits allSupportedInformation() const;

  static bool addManagerEntityToCGM(const smtk::model::EntityRef& ent);

  static int staticSetup(const std::string& optName, const smtk::model::StringList& optVal);
  virtual int setup(const std::string& optName, const smtk::model::StringList& optVal);

  double maxRelChordErr() const { return this->m_maxRelChordErr; }
  double maxAngleErr() const { return this->m_maxAngleErr; }

protected:
  friend class ImportSolid;

  Session();

  virtual SessionInfoBits transcribeInternal(
    const smtk::model::EntityRef& entity, SessionInfoBits requestedInfo);

  SessionInfoBits addCGMEntityToManager(const smtk::model::EntityRef& entity, RefEntity* refEnt, SessionInfoBits requestedInfo);
  SessionInfoBits addCGMEntityToManager(const smtk::model::EntityRef& entity, GroupingEntity* refEnt, SessionInfoBits requestedInfo);
  SessionInfoBits addCGMEntityToManager(const smtk::model::EntityRef& entity, SenseEntity* refEnt, SessionInfoBits requestedInfo);

  SessionInfoBits addBodyToManager(const smtk::model::Model&, Body*, SessionInfoBits requestedInfo);
  SessionInfoBits addVolumeUseToManager(const smtk::model::VolumeUse&, CoVolume*, SessionInfoBits requestedInfo);
  SessionInfoBits addFaceUseToManager(const smtk::model::FaceUse&, CoFace*, SessionInfoBits requestedInfo);
  SessionInfoBits addEdgeUseToManager(const smtk::model::EdgeUse&, CoEdge*, SessionInfoBits requestedInfo);
  SessionInfoBits addVertexUseToManager(const smtk::model::VertexUse&, CoVertex*, SessionInfoBits requestedInfo);
  SessionInfoBits addShellToManager(const smtk::model::Shell&, ::Shell*, SessionInfoBits requestedInfo);
  SessionInfoBits addLoopToManager(const smtk::model::Loop&, ::Loop*, SessionInfoBits requestedInfo);
  SessionInfoBits addChainToManager(const smtk::model::Chain&, ::Chain*, SessionInfoBits requestedInfo);
  SessionInfoBits addVolumeToManager(const smtk::model::Volume&, RefVolume*, SessionInfoBits requestedInfo);
  SessionInfoBits addFaceToManager(const smtk::model::Face&, RefFace*, SessionInfoBits requestedInfo);
  SessionInfoBits addEdgeToManager(const smtk::model::Edge&, RefEdge*, SessionInfoBits requestedInfo);
  SessionInfoBits addVertexToManager(const smtk::model::Vertex&, RefVertex*, SessionInfoBits requestedInfo);
  SessionInfoBits addGroupToManager(const smtk::model::Group&, RefGroup*, SessionInfoBits requestedInfo);

  void addRelations(
    smtk::model::EntityRef& entityref,
    DLIList<RefEntity*>& rels,
    SessionInfoBits requestedInfo,
    int depth);
  bool addTessellation(const smtk::model::EntityRef&, RefFace*);
  bool addTessellation(const smtk::model::EntityRef&, RefEdge*);
  bool addTessellation(const smtk::model::EntityRef&, RefVertex*);
  bool addNamesIfAny(smtk::model::EntityRef&, RefEntity*);

  static void colorPropFromIndex(smtk::model::EntityRef&, int);

  double m_maxRelChordErr;
  double m_maxAngleErr;

private:
  Session(const Session&); // Not implemented.
  void operator = (const Session&); // Not implemented.
};

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

#endif // __smtk_session_cgm_Session_h
