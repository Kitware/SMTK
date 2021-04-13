//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_core_ForEachTypes_h
#define __smtk_mesh_core_ForEachTypes_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/core/CellTypes.h"
#include "smtk/mesh/core/Handle.h"

namespace smtk
{
namespace mesh
{

//forward declare of CellSet and MeshSet
class CellSet;
class MeshSet;

class SMTKCORE_EXPORT MeshForEach
{
public:
  virtual ~MeshForEach();

  virtual void forMesh(smtk::mesh::MeshSet& singleMesh) = 0;

  smtk::mesh::ResourcePtr m_resource;
};

class SMTKCORE_EXPORT CellForEach
{
public:
  CellForEach(bool wantCoordinates = true);

  virtual ~CellForEach();

  virtual void
  forCell(const smtk::mesh::Handle& cellId, smtk::mesh::CellType cellType, int numPointIds) = 0;

  //returns true if the CellForEach visitor wants its coordinates member
  //variable filled
  bool wantsCoordinates() const { return m_wantsCoordinates; }

  const smtk::mesh::Handle* pointIds() const { return m_pointIds; }

  smtk::mesh::Handle pointId(int index) const { return m_pointIds[index]; }

  const std::vector<double>& coordinates() const { return *m_coords; }

  smtk::mesh::ResourcePtr resource() const { return m_resource; }

  //Set the coords for the visitor. This should be only be called by
  //smtk::mesh::Interface implementations
  void coordinates(std::vector<double>* coords) { m_coords = coords; }

  //Set the pointIds for the visitor. This should be only be called by
  //smtk::mesh::Interface implementations
  void pointIds(const smtk::mesh::Handle* ptIds) { m_pointIds = ptIds; }

  //Set the resource() for the visitor. This should be only be called by
  //smtk::mesh::Interface implementations
  void resource(smtk::mesh::ResourcePtr r) { m_resource = r; }

private:
  smtk::mesh::ResourcePtr m_resource;
  const smtk::mesh::Handle* m_pointIds;
  std::vector<double>* m_coords;
  bool m_wantsCoordinates;
};

class SMTKCORE_EXPORT PointForEach
{
public:
  virtual ~PointForEach();

  // PointForEach allows read access to the point ids and coordinates and write
  // access to the coordinates ( x, y, z ) it is iterating. This is different
  // than Cell and Mesh iteration which are read only.
  //
  // If the forPoints needs to modify the coordinates, you have to explicitly
  // set coordinatesModified to be true, otherwise the modified coordinates will
  // be ignored.
  //
  // Note: by default coordinatesModified is set to false
  //
  //
  virtual void forPoints(
    const smtk::mesh::HandleRange& pointIds,
    std::vector<double>& xyz,
    bool& coordinatesModified) = 0;

  smtk::mesh::ResourcePtr m_resource;
};
} // namespace mesh
} // namespace smtk

#endif
