//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_CellField_h
#define __smtk_mesh_CellField_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/Handle.h"
#include "smtk/mesh/MeshSet.h"

#include <cstddef>
#include <string>
#include <vector>

namespace smtk
{
namespace mesh
{

//Represents cell-centered floating-point data associated with a meshset. We
//represent the CellField with a unique name and a reference to the meshset.
class SMTKCORE_EXPORT CellField
{
public:
  //Default constructor generates an invalid CellField
  CellField();

  //Construct a CellField for meshset <mesh> and unique name <name>
  CellField(const smtk::mesh::MeshSet& mesh, const std::string& name);

  //Copy Constructor required for rule of 3
  CellField(const CellField& other);

  //required to be in the cpp file as we hold a HandleRange
  ~CellField();

  //Copy assignment operator required for rule of 3
  CellField& operator=(const CellField& other);

  //Comparison operators so we can construct sets of CellFields
  bool operator==(const CellField& other) const;
  bool operator!=(const CellField& other) const;
  bool operator<(const CellField& other) const;

  //Return the unique name associated with the dataset
  std::string name() const { return this->m_name; }

  //Check if the dataset is represented in the mesh database
  bool isValid() const;

  //Return the number of data tuples.
  std::size_t size() const;

  //Return the number of components in each data tuple
  std::size_t dimension() const;

  //Return the meshset associated with the dataset
  const smtk::mesh::MeshSet& meshset() const { return this->m_meshset; }

  //Return the cells associated with the dataset
  smtk::mesh::CellSet cells() const;

  //Given a range of cells, get the data associated with the cells.
  std::vector<double> get(const smtk::mesh::HandleRange& cellIds) const;

  //Given a range of cells, set the data associated with the cells and return
  //a success flag. <values> must be at least cellIds.size() * dimension() in
  //length.
  bool set(const smtk::mesh::HandleRange& cellIds, const std::vector<double>& values);

  //Get the data associated with all of the cells in the meshset.
  std::vector<double> get() const;

  //Set the data associated with all of the cells in the meshset and return
  //a success flag. <values> must be at least size() * dimension() in length.
  bool set(const std::vector<double>& values);

#ifndef SHIBOKEN_SKIP
  //Get the data associated with all of the cells in the meshset and return
  //a success flag. <values> must be at least cellIds.size() * dimension() in
  //length.
  bool get(const smtk::mesh::HandleRange& cellIds, double* values) const;

  //Set the data associated with all of the cells in the meshset and return
  //a success flag. <values> must be at least cellIds.size() * dimension() in
  //length.
  bool set(const smtk::mesh::HandleRange& cellIds, const double* const values);

  //Get the data associated with all of the cells in the meshset and return
  //a success flag. <values> must be at least size() * dimension() in length.
  bool get(double* values) const;

  //Set the data associated with all of the cells in the meshset and return
  //a success flag. <values> must be at least size() * dimension() in length.
  bool set(const double* const values);
#endif

private:
  std::string m_name;
  smtk::mesh::MeshSet m_meshset;
};

} // namespace mesh
} // namespace smtk

#endif
