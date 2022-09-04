//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_operation_operators_CoordinateTransform_h
#define smtk_operation_operators_CoordinateTransform_h

#include "smtk/operation/XMLOperation.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
namespace operation
{

/**\brief Set (or remove) a coordinate transform on components.
  *
  * The coordinate transform is specified as a pair of coordinate frames:
  * one indicating a starting location and orientation (usually this is
  * placed on or inside the object to be transformed) and one indicating
  * the destination location and orientation.
  *
  * The transform will translate the origin of the "from" coordinate-frame
  * onto the "to" coordinate-frame's origin and rotate the "from" axes to
  * align with the "to" axes.
  *
  * Note that the transform is not applied to the component's geometry
  * by this operation. Instead, the transform is applied when rendering
  * but otherwise leaves the geometric data unmodified.
  *
  * The resulting transform is itself stored as a coordinate frame, with
  * a property named "smtk.geometry.transform".
  * In addition to the computed transform, the component will also have
  * coordinate-frame properties name "smtk.geometry.transform.from.frame"
  * and "smtk.geometry.transform.to.frame" set, so that the operation can
  * serve as an editor.
  *
  * Finally, provenance information may be set as additional string
  * properties named:
  * + "smtk.geometry.transform.from.id"
  * + "smtk.geometry.transform.to.id" – the UUID of a component
  *   with a coordinate-frame property used as the "from" or "to"
  *   frame, respectively.
  * + "smtk.geometry.transform.from.name"
  * + "smtk.geometry.transform.to.name" – either the name of
  *   a coordinate-frame property on the "from.id" (or "to.id",
  *   respectively) above or user-provided text describing the
  *   coordinate frame used to generate the transform.
  */
class SMTKCORE_EXPORT CoordinateTransform : public XMLOperation
{
public:
  smtkTypeMacro(smtk::operation::CoordinateTransform);
  smtkSharedPtrCreateMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  bool removeTransform(
    const std::shared_ptr<smtk::attribute::ReferenceItem>& associations,
    Result& result);

  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace operation
} // namespace smtk

#endif
