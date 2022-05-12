//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Field_h
#define smtk_markup_Field_h

#include "smtk/markup/Component.h"

namespace smtk
{
namespace markup
{
namespace arcs
{
struct FieldsToShapes;
}

/**\brief A function defined over some shape's space.
  *
  * A field has a name and a type.
  * The name must be unique across all fields of the same type.
  * The type indicates a domain owned by the spatial data
  * (usually "points" or "cells", but other function spaces are permitted
  * as long as the shape has AssignedIds for that domain).
  *
  * Note that without an arc connecting a Field to an instance of
  * SpatialData, the field is ill-defined.
  */
class SMTKMARKUP_EXPORT Field : public smtk::markup::Component
{
public:
  smtkTypeMacro(smtk::markup::Field);
  smtkSuperclassMacro(smtk::markup::Component);

  template<typename... Args>
  Field(Args&&... args)
    : smtk::markup::Component(std::forward<Args>(args)...)
  {
  }

  ~Field() override;

  /// Provide initializers for resources to call after construction with
  /// excess arguments (see Args in constructor above).
  void initialize(const std::string& name, smtk::string::Token fieldType);
  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;

  smtk::string::Token fieldType() const { return m_fieldType; }

  /// Arc to the spatial data over which this field's function is defined (its domain).
  ArcEndpointInterface<arcs::FieldsToShapes, ConstArc, OutgoingArc> shapes() const;
  ArcEndpointInterface<arcs::FieldsToShapes, NonConstArc, OutgoingArc> shapes();

protected:
  smtk::string::Token m_fieldType;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Field_h
