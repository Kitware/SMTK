//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/operators/CoordinateTransform.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/operation/MarkGeometry.h"
#include "smtk/resource/Component.h"
#include "smtk/resource/properties/CoordinateFrame.h"

#include "smtk/operation/CoordinateTransform_xml.h"

#include "Eigen/Core"
#include "Eigen/Dense"

#include <iostream>

using smtk::attribute::ReferenceItem;
using smtk::resource::Component;
using smtk::resource::properties::CoordinateFrame;

namespace smtk
{
namespace operation
{

bool CoordinateTransform::removeTransform(
  const std::shared_ptr<smtk::attribute::ReferenceItem>& associations,
  Result& result)
{
  bool didRemove = false;
  smtk::attribute::ComponentItemPtr modifiedItem = result->findComponent("modified");
  for (std::size_t ii = 0; ii < associations->numberOfValues(); ++ii)
  {
    if (const auto& source = associations->value(ii))
    {
      auto frameProps = source->properties().get<CoordinateFrame>();
      auto names = frameProps.keys();
      bool modified = false;
      if (names.find("transform") != names.end())
      {
        frameProps.erase("transform");
        modified = true;
      }
      if (names.find("smtk.geometry.transform") != names.end())
      {
        frameProps.erase("smtk.geometry.transform");
        modified = true;
      }
      if (modified)
      {
        if (auto component = std::dynamic_pointer_cast<smtk::resource::Component>(source))
        {
          modifiedItem->appendValue(component);
        }
        MarkGeometry().markModified(source);
      }
      didRemove |= modified;
    }
  }
  return didRemove;
}

CoordinateTransform::Result CoordinateTransform::operateInternal()
{
  smtk::attribute::ItemPtr removeItem = this->parameters()->find("remove");
  bool removeProperty = removeItem->isEnabled();

  auto associations = this->parameters()->associations();

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  if (removeProperty)
  {
    bool didRemove = this->removeTransform(associations, result);
    if (!didRemove)
    {
      smtkWarningMacro(this->log(), "None of the input components had a transform.");
      result->findInt("outcome")->setValue(
        static_cast<int>(smtk::operation::Operation::Outcome::FAILED));
    }
    return result;
  }

  // Now we know we are adding or replacing a transform.
  auto fromFrameGroup = this->parameters()->findGroup("from");
  auto toFrameGroup = this->parameters()->findGroup("to");

  // TODO: Need to handle case where fromFrame and/or toFrame
  //       have a non-null parent component. This can be addressed
  //       by left-/right-multiplying the from-parent's inverse
  //       and/or to-parent's transform, respectively.

  smtk::resource::properties::CoordinateFrame fromFrame(
    fromFrameGroup, "origin", "x axis", "y axis", "z axis", "parent");
  smtk::resource::properties::CoordinateFrame toFrame(
    toFrameGroup, "origin", "x axis", "y axis", "z axis", "parent");
  smtk::resource::properties::CoordinateFrame transform;
  Eigen::Matrix<double, 4, 4> inverseFrom;
  Eigen::Matrix<double, 4, 4> forwardTo;
#if 0
  // For debugging
  std::cout
    << "from\n"
    << "  origin " << fromFrame.origin[0] << " " << fromFrame.origin[1] << " " << fromFrame.origin[2] << "\n"
    << "  x axis " << fromFrame.xAxis[0] << " " << fromFrame.xAxis[1] << " " << fromFrame.xAxis[2] << "\n"
    << "  y axis " << fromFrame.yAxis[0] << " " << fromFrame.yAxis[1] << " " << fromFrame.yAxis[2] << "\n"
    << "  z axis " << fromFrame.zAxis[0] << " " << fromFrame.zAxis[1] << " " << fromFrame.zAxis[2] << "\n"
    << "to\n"
    << "  origin " << toFrame.origin[0] << " " << toFrame.origin[1] << " " << toFrame.origin[2] << "\n"
    << "  x axis " << toFrame.xAxis[0] << " " << toFrame.xAxis[1] << " " << toFrame.xAxis[2] << "\n"
    << "  y axis " << toFrame.yAxis[0] << " " << toFrame.yAxis[1] << " " << toFrame.yAxis[2] << "\n"
    << "  z axis " << toFrame.zAxis[0] << " " << toFrame.zAxis[1] << " " << toFrame.zAxis[2] << "\n"
    ;
#endif

  Eigen::Matrix4d translate1;
  translate1 = Eigen::Matrix4d::Identity();
  translate1(3, Eigen::seq(0, 2)) =
    -Eigen::Map<Eigen::Array<double, 1, 3>>(fromFrame.origin.data());
  // std::cout << "Translate1\n" << translate1 << "\n";

  inverseFrom = Eigen::Matrix4d::Identity();
  inverseFrom(Eigen::seq(0, 2), 0) = Eigen::Map<Eigen::Array<double, 3, 1>>(fromFrame.xAxis.data());
  inverseFrom(Eigen::seq(0, 2), 1) = Eigen::Map<Eigen::Array<double, 3, 1>>(fromFrame.yAxis.data());
  inverseFrom(Eigen::seq(0, 2), 2) = Eigen::Map<Eigen::Array<double, 3, 1>>(fromFrame.zAxis.data());
  // std::cout << "inverseFrom\n" << inverseFrom << "\n";

  forwardTo = Eigen::Matrix4d::Identity();
  forwardTo(0, Eigen::seq(0, 2)) = Eigen::Map<Eigen::Array<double, 1, 3>>(toFrame.xAxis.data());
  forwardTo(1, Eigen::seq(0, 2)) = Eigen::Map<Eigen::Array<double, 1, 3>>(toFrame.yAxis.data());
  forwardTo(2, Eigen::seq(0, 2)) = Eigen::Map<Eigen::Array<double, 1, 3>>(toFrame.zAxis.data());
  // std::cout << "forwardTo\n" << forwardTo << "\n";

  Eigen::Matrix4d translate2;
  translate2 = Eigen::Matrix4d::Identity();
  translate2(3, Eigen::seq(0, 2)) = Eigen::Map<Eigen::Array<double, 1, 3>>(toFrame.origin.data());
  // std::cout << "translate2\n" << translate2 << "\n";

  auto xfm = translate1 * inverseFrom * forwardTo * translate2;
  for (int ii = 0; ii < 3; ++ii)
  {
    transform.xAxis[ii] = xfm(0, ii);
    transform.yAxis[ii] = xfm(1, ii);
    transform.zAxis[ii] = xfm(2, ii);
    transform.origin[ii] = xfm(3, ii);
  }
#if 0
  // For debugging
  std::cout
    << "xfm\n" << xfm << "\n\n"
    << "origin " << transform.origin[0] << " " << transform.origin[1] << " " << transform.origin[2] << "\n"
    << "x axis " << transform.xAxis[0] << " " << transform.xAxis[1] << " " << transform.xAxis[2] << "\n"
    << "y axis " << transform.yAxis[0] << " " << transform.yAxis[1] << " " << transform.yAxis[2] << "\n"
    << "z axis " << transform.zAxis[0] << " " << transform.zAxis[1] << " " << transform.zAxis[2] << "\n"
    ;
#endif

  this->removeTransform(associations, result);
  smtk::attribute::ComponentItemPtr modifiedItem = result->findComponent("modified");
  for (smtk::resource::PersistentObjectPtr association : *associations)
  {
    if (auto component = std::dynamic_pointer_cast<Component>(association))
    {
      component->properties().insert("transform", transform);
      MarkGeometry().markModified(component);
      modifiedItem->appendValue(component);
    }
  }

  // TODO: Record where the coordinate frames came from (if they were named properties
  //       of any component... this is not information the parameters provide yet).

  return result;
}

const char* CoordinateTransform::xmlDescription() const
{
  return CoordinateTransform_xml;
}

} // namespace operation
} // namespace smtk
