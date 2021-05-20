//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_core_MeshSet_h
#define __smtk_mesh_core_MeshSet_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/Handle.h"
#include "smtk/mesh/core/PointSet.h"
#include "smtk/mesh/core/QueryTypes.h"
#include "smtk/mesh/core/TypeSet.h"

#include "smtk/model/EntityRef.h"

#include "smtk/common/UUID.h"

namespace smtk
{
namespace mesh
{
class CellField;
class Component;
class PointField;

typedef std::vector<smtk::mesh::MeshSet> MeshList;
typedef std::set<smtk::mesh::MeshSet> MeshSets;

//Represents a collection of meshes that have been constructed by a Resource
//We represent the collection of meshes by holding onto the parent entity
//and a vector/range of mesh entities
class SMTKCORE_EXPORT MeshSet
{
  friend SMTKCORE_EXPORT MeshSet set_intersect(const MeshSet& a, const MeshSet& b);
  friend SMTKCORE_EXPORT MeshSet set_difference(const MeshSet& a, const MeshSet& b);
  friend SMTKCORE_EXPORT MeshSet set_union(const MeshSet& a, const MeshSet& b);
  friend SMTKCORE_EXPORT void for_each(const MeshSet& a, MeshForEach& filter);
  friend class Resource; //required for deletion of meshes
public:
  //default constructor generates an invalid MeshSet
  MeshSet();

  //construct a MeshSet that represents all meshes that are children
  //of the handle
  MeshSet(const smtk::mesh::ResourcePtr& parent, smtk::mesh::Handle handle);
  MeshSet(const smtk::mesh::ConstResourcePtr& parent, smtk::mesh::Handle handle);

  //construct a MeshSet that represents an arbitrary unkown subset meshes that
  //are children of the handle.
  MeshSet(
    const smtk::mesh::ResourcePtr& parent,
    smtk::mesh::Handle handle,
    const smtk::mesh::HandleRange& range);
  MeshSet(
    const smtk::mesh::ConstResourcePtr& parent,
    smtk::mesh::Handle handle,
    const smtk::mesh::HandleRange& range);

  //Copy Constructor required for rule of 3
  MeshSet(const MeshSet& other);

  //required to be in the cpp file as we hold a HandleRange
  ~MeshSet();

  //Copy assignment operator required for rule of 3
  MeshSet& operator=(const MeshSet& other);

  //Comparison operators
  bool operator==(const MeshSet& other) const;
  bool operator!=(const MeshSet& other) const;
  bool operator<(const MeshSet& other) const;

  //append another MeshSet to this MeshSet.
  //If both MeshSets have valid parent pointers they must be to the same parent
  //for the append to occur.
  //If the lhs MeshSet parent is nullptr and the rhs parent isn't we will copy
  //the rhs parent to be the lhs parent.
  //This is done so the following works:
  //
  // smtk::mesh::MeshSet ms;
  // for( int i=0; i < size; ++i ) { ms.append( query.meshes() ) }
  bool append(const MeshSet& other);

  //determine if the given MeshSet is valid.
  bool isValid() const;

  bool is_empty() const;

  //set/get the name of this meshset
  std::string name() const;
  bool setName(const std::string&);

  //number of meshes
  std::size_t size() const;

  //get all the current domains, dirichlet, neumanns for all meshes in this meshset
  std::vector<smtk::mesh::Domain> domains() const;
  std::vector<smtk::mesh::Dirichlet> dirichlets() const;
  std::vector<smtk::mesh::Neumann> neumanns() const;

  //set the domainl, dirichlet, or neumanns for all meshes in this meshset
  bool setDomain(const smtk::mesh::Domain& d);
  bool setDirichlet(const smtk::mesh::Dirichlet& d);
  bool setNeumann(const smtk::mesh::Neumann& n);

  const smtk::common::UUID& id() const;
  void setId(const smtk::common::UUID&);
  smtk::common::UUIDArray modelEntityIds() const;

  //append the passed EntityRefArray with the model entities associated with
  //this meshset, and return true on success. If the MeshSet's parent resource
  //does not have its ModelManager set, this method will fail even though
  //modelEntityIds() will still be valid.
  bool modelEntities(smtk::model::EntityRefArray&) const;
  bool setModelEntityId(const smtk::common::UUID&);
  bool setModelEntity(const smtk::model::EntityRef&);

  std::vector<std::string> names() const;
  smtk::mesh::TypeSet types() const;
  smtk::mesh::CellSet cells() const; //all cells of the meshset
  //all points of the meshset; if boundary_only is true then higher-order points
  //are omitted.
  smtk::mesh::PointSet points(bool boundary_only = false) const;
  smtk::mesh::PointConnectivity pointConnectivity()
    const; //all point connectivity info for all cells

  //we should be able to extract the points or cells of the meshes.
  smtk::mesh::CellSet cells(smtk::mesh::CellType cellType) const;
  smtk::mesh::CellSet cells(smtk::mesh::CellTypes cTypes) const;
  smtk::mesh::CellSet cells(smtk::mesh::DimensionType dim) const;

  //subset this MeshSet by a dimension, or a property such as
  //Domain, Dirichlet, or Neumann
  smtk::mesh::MeshSet subset(smtk::mesh::DimensionType dim) const;
  smtk::mesh::MeshSet subset(const smtk::mesh::Domain& d) const;
  smtk::mesh::MeshSet subset(const smtk::mesh::Dirichlet& d) const;
  smtk::mesh::MeshSet subset(const smtk::mesh::Neumann& n) const;

  //subset this MeshSet given an index into entity sets (m_range)
  smtk::mesh::MeshSet subset(std::size_t ith) const;

  //Extract the shell ( exterior (D-1)-dimensional elements ) of this set of
  //meshes. This operation might create new cells if no shell already exists
  //for the given meshset. If not already present, the resulting meshset will
  //be added to the database so that the shell is saved.
  //Will return an empty set when no shell can be found
  smtk::mesh::MeshSet extractShell() const;

  //Extract the shell ( exterior (D-1)-dimensional elements ) of this set of
  //meshes. This operation might create new cells if no shell already exists
  //for the given meshset. The input field <created> is set to reflect
  //whether or not the resulting meshset is added to the database.
  //Will return an empty set when no shell can be found
  smtk::mesh::MeshSet extractShell(bool& created) const;

  //Extract the adjacency cells of this set of meshes for a given dimension.
  //This operation might create new cells if they do not already exist
  //for the given meshset. The resulting meshset will be added to the
  //database so that the adjacencies are saved.
  //Will return an empty set when no adjacencies can be found
  smtk::mesh::MeshSet extractAdjacenciesOfDimension(int dimension) const;

  //Extract the adjacency cells of this set of meshes for a given dimension.
  //This operation might create new cells if they do not already exist
  //for the given meshset. The input field <created> is set to reflect
  //whether or not the resulting meshset is added to the database.
  //Will return an empty set when no adjacencies can be found
  smtk::mesh::MeshSet extractAdjacenciesOfDimension(int dimension, bool& created) const;

  //Merge all duplicate points contained within this meshset.
  //Will return true when any points have been merged
  //Will cause any existing PointConnectivity and PointSet's to become
  //invalid, and using them will cause any undefined behavior
  bool mergeCoincidentContactPoints(double tolerance = 1.0e-6);

  //Create a new cell field with the given name, dimension and data. The number
  //of values in <data> must be the # of cells in the meshset multiplied by the
  //dimension of the cell field.
  smtk::mesh::CellField createCellField(
    const std::string& name,
    int dimension,
    const smtk::mesh::FieldType& type,
    const void* field = nullptr);
  smtk::mesh::CellField cellField(const std::string& name) const;
  std::set<smtk::mesh::CellField> cellFields() const;
  //Remove the dataset from this meshset.
  bool removeCellField(smtk::mesh::CellField cellfield);

  //Create a new point field with the given name, dimension and data. The number
  //of values in <data> must be the # of points in the meshset multiplied by the
  //dimension of the point field.
  smtk::mesh::PointField createPointField(
    const std::string& name,
    int dimension,
    const smtk::mesh::FieldType& type,
    const void* field = nullptr);
  smtk::mesh::PointField pointField(const std::string& name) const;
  std::set<smtk::mesh::PointField> pointFields() const;
  //Remove the dataset from this meshset.
  bool removePointField(smtk::mesh::PointField pointfield);

  //get the underlying HandleRange that this MeshSet represents
  const smtk::mesh::HandleRange& range() const { return m_range; }

  //get the underlying resource that this MeshSet belongs to
  const smtk::mesh::ResourcePtr& resource() const;

private:
  friend std::shared_ptr<Component> Component::create(
    const ResourcePtr&,
    const smtk::common::UUID&);
  friend std::shared_ptr<Component> Component::create(const MeshSet&);

  smtk::mesh::ResourcePtr m_parent;
  smtk::mesh::Handle m_handle{};
  smtk::mesh::HandleRange m_range; //range of entity sets
  mutable smtk::common::UUID m_id;
};

//Function that provide set operations on MeshSets

//intersect two mesh sets, placing the results in the return mesh set. The
//intersection is done at the mesh id level, not at the cell id, or at
//the point usage level. If you need to find the result of a cell id intersection
//you should use CellSet. If you need to find the result of a intersection
//based on the shared points you want to use CellSet and the point_intersect
//call.
//Note: If the meshsets come from different resources the result will
//always be empty
SMTKCORE_EXPORT MeshSet set_intersect(const MeshSet& a, const MeshSet& b);

//subtract two mesh sets, placing the results in the return mesh set. The
//difference is done at the mesh id level, not at the cell id, or at
//the point usage level. If you need to find the result of a cell id subtraction
//you should use CellSet. If you need to find the result of a difference
//based on the shared points you want to use CellSet and the point_difference
//call.
//Note: If the meshsets come from different resources the result will
//always be empty
SMTKCORE_EXPORT MeshSet set_difference(const MeshSet& a, const MeshSet& b);

//union two mesh sets, placing the results in the return mesh set
//Note: If the meshsets come from different resources the result will
//always be empty
SMTKCORE_EXPORT MeshSet set_union(const MeshSet& a, const MeshSet& b);

//apply a for_each mesh operator on all meshes of a given set.
SMTKCORE_EXPORT void for_each(const MeshSet& a, MeshForEach& filter);

} // namespace mesh
} // namespace smtk

#endif
