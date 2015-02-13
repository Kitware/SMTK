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
#include "smtk/mesh/CellTypes.h"
#include "smtk/mesh/DimensionTypes.h"


namespace smtk {
namespace mesh {

typedef int Points;

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT IntegerTag
{
public:
  explicit IntegerTag(int value):
     m_value(value)
    {
    }

  int value() const { return m_value; }

  //custom operators to make comparing materials easy
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
class SMTKCORE_EXPORT Material : public IntegerTag
{
public:
  explicit Material(int value) : IntegerTag(value) {}
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
