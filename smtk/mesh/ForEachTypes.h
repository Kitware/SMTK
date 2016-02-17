//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_ForEachTypes_h
#define __smtk_mesh_ForEachTypes_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/CellTypes.h"
#include "smtk/mesh/Handle.h"

namespace smtk {
namespace mesh {

//forward declare of CellSet and MeshSet
class CellSet;
class MeshSet;

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT MeshForEach
{
public:
  virtual ~MeshForEach();

  virtual void forMesh(const smtk::mesh::MeshSet& singleMesh) = 0;

  smtk::mesh::CollectionPtr m_collection;
};

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT CellForEach
{
public:
  CellForEach(bool wantCoordinates=true);

  virtual ~CellForEach();

  virtual void forCell(const smtk::mesh::Handle& cellId,
                       smtk::mesh::CellType cellType,
                       int numPointIds) = 0;

  //returns true if the CellForEach visitor wants its coordinates member
  //variable filled
  bool wantsCoordinates() const
    { return this->m_wantsCoordinates; }

  const smtk::mesh::Handle* pointIds() const
    { return this->m_pointIds; }

  smtk::mesh::Handle pointId(int index) const
    { return this->m_pointIds[index]; }

  const std::vector<double>& coordinates() const
    { return *this->m_coords; }

  smtk::mesh::CollectionPtr collection() const
    { return this->m_collection;}

  //Set the coords for the visitor. This should be only be called by
  //smtk::mesh::Interface implementations
  void coordinates(std::vector<double>* coords)
    { this->m_coords = coords; }

  //Set the pointIds for the visitor. This should be only be called by
  //smtk::mesh::Interface implementations
  void pointIds(const smtk::mesh::Handle* ptIds)
    { this->m_pointIds = ptIds; }

  //Set the collection for the visitor. This should be only be called by
  //smtk::mesh::Interface implementations
  void collection(smtk::mesh::CollectionPtr c)
    { this->m_collection = c; }

private:
  smtk::mesh::CollectionPtr m_collection;
  const smtk::mesh::Handle* m_pointIds;
  std::vector<double>* m_coords;
  bool m_wantsCoordinates;
};

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT PointForEach
{
public:
  virtual ~PointForEach();

  virtual void forPoint(const smtk::mesh::Handle& pointId, double x, double y, double z)=0;

  smtk::mesh::CollectionPtr m_collection;
};

}
}

#endif
