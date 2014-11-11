//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_ex_bridge_h
#define __smtk_ex_bridge_h

#include "smtk/model/Bridge.h"
#include "smtk/common/UUID.h"

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
/**\brief Implement a bridge from VTK unstructured grids to SMTK.
  */
class Bridge : public smtk::model::Bridge
{
public:
  smtkDeclareModelingKernel();
  typedef smtk::shared_ptr<Bridge> Ptr;
  typedef smtk::model::BridgedInfoBits BridgedInfoBits;
  static BridgePtr create();
  virtual ~Bridge();
  virtual BridgedInfoBits allSupportedInformation() const;

  EntityHandle toEntity(const smtk::model::Cursor& eid);
  smtk::model::Cursor toCursor(const EntityHandle& ent);

  static int staticSetup(
    const std::string& optName,
    const smtk::model::StringList& optVal);
  virtual int setup(
    const std::string& optName,
    const smtk::model::StringList& optVal);

protected:
  Bridge();

  virtual BridgedInfoBits transcribeInternal(
    const smtk::model::Cursor& entity,
    BridgedInfoBits requestedInfo);

  vtkSmartPointer<vtkUnstructuredGrid> Model;
  // ... };
  // -- 1 --

  void addRelations(
    smtk::model::Cursor& cursor,
    std::vector<EntityHandle>& rels,
    BridgedInfoBits requestedInfo,
    int depth);
  bool addTessellation(
    const smtk::model::Cursor&,
    const EntityHandle&);

private:
  Bridge(const Bridge&); // Not implemented.
  void operator = (const Bridge&); // Not implemented.
};

    } // namespace tutorial
  } // namespace bridge
} // namespace smtk

#endif //  __smtk_ex_bridge_h
