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
#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/CellTypes.h"
#include "smtk/mesh/DimensionTypes.h"
#include "smtk/mesh/ForEachTypes.h"
#include "smtk/mesh/Handle.h"

#include "smtk/common/UUID.h"

namespace smtk {
namespace mesh {

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
template<int S>
class SMTKCORE_EXPORT OpaqueTag
{
public:
  explicit OpaqueTag(const unsigned char* value):
    m_value(value, value + S)
    {
    }

  inline static int size() { return S; }

  const unsigned char* value() const { return &m_value[0]; }

  //custom operators to make comparing tags easy
  inline bool operator < (const OpaqueTag& other) const
    {
    for (int i = 0; i < S; ++i)
      if (this->m_value < other.m_value)
        return true;
      else if (this->m_value > other.m_value)
        return false;
    return false;
    }
  inline bool operator == (const OpaqueTag& other) const
    {
    for (int i = 0; i < S; ++i)
      if (this->m_value != other.m_value)
        return false;
    return true;
    }
  inline bool operator != (const OpaqueTag& other) const
    {
    for (int i = 0; i < S; ++i)
      if (this->m_value != other.m_value)
        return true;
    return false;
    }

private:
  std::vector<unsigned char> m_value;
};

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT UUIDTag : public OpaqueTag<smtk::common::UUID::SIZE>
{
public:
  explicit UUIDTag(const smtk::common::UUID& value):
    OpaqueTag<smtk::common::UUID::SIZE>(value.begin())
    {
    }

  smtk::common::UUID uuid() const
    {
    return smtk::common::UUID(
      this->value(), this->value() + smtk::common::UUID::SIZE);
    }
};

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT Model : public UUIDTag
{
public:
  explicit Model(const smtk::common::UUID& value) : UUIDTag(value) {}
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
