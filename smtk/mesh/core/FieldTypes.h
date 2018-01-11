//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_core_FieldTypes_h
#define __smtk_mesh_core_FieldTypes_h

namespace smtk
{
namespace mesh
{

enum class FieldType
{
  Integer,
  Double,
  MaxFieldType
};

template <typename T>
struct FieldTypeFor
{
  static constexpr FieldType type = FieldType::MaxFieldType;
};

template <>
struct FieldTypeFor<double>
{
  static constexpr FieldType type = FieldType::Double;
};

template <>
struct FieldTypeFor<int>
{
  static constexpr FieldType type = FieldType::Integer;
};
}
}

#endif
