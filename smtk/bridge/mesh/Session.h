//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_mesh_Session_h
#define __smtk_session_mesh_Session_h

#include "smtk/bridge/mesh/Exports.h"
#include "smtk/bridge/mesh/Topology.h"

#include "smtk/mesh/Collection.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Session.h"

#include "smtk/common/UUID.h"
#include "smtk/common/UUIDGenerator.h"

class vtkInformationDoubleKey;
class vtkInformationIntegerKey;
class vtkInformationStringKey;

namespace smtk {
namespace bridge {
namespace mesh {

class SMTKMESHSESSION_EXPORT Session : public smtk::model::Session
{
public:
  smtkTypeMacro(Session);
  smtkSuperclassMacro(smtk::model::Session);
  smtkSharedFromThisMacro(smtk::model::Session);
  smtkCreateMacro(smtk::model::Session);
  smtkDeclareModelingKernel();
  typedef smtk::model::SessionInfoBits SessionInfoBits;

  virtual ~Session() {}

  void addTopology(Topology t) { this->m_topologies.push_back(t); }
  Topology* const topology(smtk::model::Model& model);

protected:
  Session();

  virtual SessionInfoBits transcribeInternal(
    const smtk::model::EntityRef& entity,
    SessionInfoBits requestedInfo,
    int depth = -1);

  std::vector<Topology> m_topologies;
};


} // namespace mesh
} // namespace bridge
} // namespace smtk


#endif // __smtk_session_mesh_Session_h
