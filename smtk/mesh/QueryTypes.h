//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_QueryTypes_h
#define __smtk_mesh_QueryTypes_h

//Query Types is a convenience header, whose goal is to make it easier
//for users to query a manager
#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/CellTypes.h"
#include "smtk/mesh/DimensionTypes.h"
#include "smtk/mesh/Handle.h"

namespace smtk {
namespace mesh {

typedef int Points;

//forward declare of CellSet and MeshSet
class CellSet;
class MeshSet;

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT IntegerTag
{
public:
  explicit IntegerTag(int value):
     m_value(value)
    {
    }

  int value() const { return m_value; }

  //custom operators to make comparing tags easy
  bool operator<(const IntegerTag& other) const
    { return this->m_value < other.m_value; }
  bool operator==(const IntegerTag& other) const
    { return this->m_value == other.m_value; }
  bool operator!=(const IntegerTag& other) const
    { return this->m_value != other.m_value; }

private:
  int m_value;
};

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT Domain : public IntegerTag
{
public:
  explicit Domain(int value) : IntegerTag(value) {}
};

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT Dirichlet : public IntegerTag
{
public:
  explicit Dirichlet(int value) : IntegerTag(value) {}
};

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT Neumann : public IntegerTag
{
public:
  explicit Neumann(int value) : IntegerTag(value) {}
};

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT MeshForEach
{
public:
  virtual void operator()(const smtk::mesh::MeshSet& singleMesh)=0;

  smtk::mesh::CollectionPtr m_collection;
};

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT CellForEach
{
public:
  virtual void operator()(int numPts,
                          const smtk::mesh::Handle* const pointIds,
                          const double* const coords)=0;

  smtk::mesh::CollectionPtr m_collection;
};


//----------------------------------------------------------------------------
enum ContainmentType
{
  PartiallyContained=1,
  FullyContained=2
};

}
}

#endif
