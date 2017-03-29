//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_ex_session_h
#define __smtk_ex_session_h

#include "smtk/common/UUID.h"
#include "smtk/model/Session.h"

#include "vtkSmartPointer.h"

#include <map>

class vtkUnstructuredGrid;

namespace smtk {
  namespace bridge {
    namespace tutorial {

// ++ 2 ++
/// The types of entities in a VTK "model"
enum EntityType
{
  VTK_POINT,        // The entity is a point
  VTK_CELL,         // The entity is a vertex, edge, face, or volume
  VTK_GROUP,        // The entity is a group (of points and/or cells)
  VTK_POINT_DATUM,  // The entity is a property associated with a point
  VTK_CELL_DATUM    // The entity is a property associated with a cell
};

/// A "handle" for a VTK entity (point, cell, property, etc.)
struct EntityHandle {
  EntityType entityType;   //!< Describes the type of VTK entity
  int entityRelation;      //!< A modifier for the entity type (to select an array or boundary)
  vtkIdType entityId;      //!< The offset in the array of entities describing this entity.
};
// -- 2 --

// ++ 1 ++
/**\brief Implement a session from VTK unstructured grids to SMTK.
  */
class Session : public smtk::model::Session
{
public:
  // This is required of every session:
  smtkDeclareModelingKernel();
  typedef smtk::shared_ptr<Session> Ptr;
  typedef smtk::model::SessionInfoBits SessionInfoBits;
  static SessionPtr create();
  virtual ~Session();
  virtual SessionInfoBits allSupportedInformation() const;

  // These are specific to each session but required in some form:
  EntityHandle toEntity(const smtk::model::EntityRef& eid);
  smtk::model::EntityRef toEntityRef(const EntityHandle& ent);

  // These methods may be provided as needed.
  static int staticSetup(
    const std::string& optName,
    const smtk::model::StringList& optVal);
  virtual int setup(
    const std::string& optName,
    const smtk::model::StringList& optVal);

protected:
  Session();

  // This is required of every session:
  virtual SessionInfoBits transcribeInternal(
    const smtk::model::EntityRef& entity,
    SessionInfoBits requestedInfo);

  vtkSmartPointer<vtkUnstructuredGrid> Model;
  // ... };
  // -- 1 --

  void addRelations(
    smtk::model::EntityRef& entityref,
    std::vector<EntityHandle>& rels,
    SessionInfoBits requestedInfo,
    int depth);
  bool addTessellation(
    const smtk::model::EntityRef&,
    const EntityHandle&);

private:
  Session(const Session&); // Not implemented.
  void operator = (const Session&); // Not implemented.
};

    } // namespace tutorial
  } // namespace bridge
} // namespace smtk

#endif //  __smtk_ex_session_h
