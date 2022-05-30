//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_mesh_core_PointField_h
#define smtk_mesh_core_PointField_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/core/Handle.h"
#include "smtk/mesh/core/MeshSet.h"

#include <cstddef>
#include <string>
#include <vector>

namespace smtk
{
namespace mesh
{

//Represents point-centered floating-point data associated with a meshset. We
//represent the PointField with a unique name and a reference to the meshset.
class SMTKCORE_EXPORT PointField
{
public:
  //Default constructor generates an invalid PointField
  PointField();

  //Construct a PointField for meshset <mesh> and unique name <name>
  PointField(const smtk::mesh::MeshSet& mesh, const std::string& name);

  //Copy Constructor required for rule of 3
  PointField(const PointField& other);

  //required to be in the cpp file as we hold a HandleRange
  ~PointField();

  //Copy assignment operator required for rule of 3
  PointField& operator=(const PointField& other);

  //Comparison operators so we can construct sets of PointFields
  bool operator==(const PointField& other) const;
  bool operator!=(const PointField& other) const;
  bool operator<(const PointField& other) const;

  //Return the unique name associated with the dataset
  std::string name() const { return m_name; }

  //Check if the dataset is represented in the mesh database
  bool isValid() const;

  //Return the number of data tuples.
  std::size_t size() const;

  //Return the number of components in each data tuple
  std::size_t dimension() const;

  //Return the field type
  smtk::mesh::FieldType type() const;

  //Return the meshset associated with the dataset
  const smtk::mesh::MeshSet& meshset() const { return m_meshset; }

  //Return the points associated with the dataset
  smtk::mesh::PointSet points() const;

  //Get the data associated with all of the cells in the meshset and return
  //a success flag. <values> must be at least
  //sizeof(type()) * cellIds.size() * dimension() in size.
  bool get(const smtk::mesh::HandleRange& cellIds, void* values) const;

  //Set the data associated with all of the cells in the meshset and return
  //a success flag. <values> must be at least
  //sizeof(type()) * cellIds.size() * dimension() in size.
  bool set(const smtk::mesh::HandleRange& cellIds, const void* values);

  //Get the data associated with all of the cells in the meshset and return
  //a success flag. <values> must be at least
  //sizeof(type()) * cellIds.size() * dimension() in size.
  bool get(void* values) const;

  //Set the data associated with all of the cells in the meshset and return
  //a success flag. <values> must be at least
  //sizeof(type()) * cellIds.size() * dimension() in size.
  bool set(const void* values);

  //Convenience method for accessing field data.
  template<typename T>
  std::vector<T> get() const
  {
    if (type() != FieldTypeFor<T>::type)
    {
      return std::vector<T>();
    }
    std::vector<T> values(size() * dimension());
    if (!get(values.data()))
    {
      return std::vector<T>();
    }
    return values;
  }

  //Convenience method for accessing field data.
  template<typename T>
  std::vector<T> get(const smtk::mesh::HandleRange& cellIds) const
  {
    if (type() != FieldTypeFor<T>::type)
    {
      return std::vector<T>();
    }
    std::vector<T> values(cellIds.size() * dimension());
    if (!get(cellIds, values.data()))
    {
      return std::vector<T>();
    }
    return values;
  }

  //Convenience method for setting field data.
  template<typename T>
  bool set(const std::vector<T>& values)
  {
    if (type() != FieldTypeFor<T>::type)
    {
      return false;
    }
    return set(values.data());
  }

  //Convenience method for setting field data.
  template<typename T>
  bool set(const smtk::mesh::HandleRange& cellIds, const std::vector<T>& values)
  {
    if (type() != FieldTypeFor<T>::type)
    {
      return false;
    }
    return set(cellIds, values.data());
  }

private:
  std::string m_name;
  smtk::mesh::MeshSet m_meshset;
};

} // namespace mesh
} // namespace smtk

#endif
