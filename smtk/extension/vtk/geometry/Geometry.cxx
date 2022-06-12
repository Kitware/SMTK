//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/geometry/Geometry.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Properties.h"
#include "smtk/resource/Resource.h"
#include "smtk/resource/properties/CoordinateFrame.h"

#include "vtkDataObject.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkUnsignedCharArray.h"

#include "vtk_eigen.h"
#include VTK_EIGEN(Eigenvalues)
#include VTK_EIGEN(Geometry)

#include "smtk/io/Logger.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace geometry
{

namespace
{

Eigen::Matrix<double, 4, 4> concatenatedParentTransforms(
  const smtk::resource::ResourcePtr& resource,
  const smtk::resource::properties::CoordinateFrame& frame,
  std::set<const smtk::resource::properties::CoordinateFrame*>& visited)
{
  if (!visited.insert(&frame).second)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Coordinate-frame parent objects form a " << visited.size()
                                                << "-cycle. "
                                                   "Results will be incorrect.");
    // Return the identity matrix since this means the overall result will be
    // the concatenation of every entry in the cycle with no repeats.
    return Eigen::Matrix<double, 4, 4>::Identity();
  }

  std::array<double, 16> transformRaw{ frame.xAxis[0],  frame.xAxis[1],  frame.xAxis[2],  0.0,
                                       frame.yAxis[0],  frame.yAxis[1],  frame.yAxis[2],  0.0,
                                       frame.zAxis[0],  frame.zAxis[1],  frame.zAxis[2],  0.0,
                                       frame.origin[0], frame.origin[1], frame.origin[2], 1.0 };
  Eigen::Matrix<double, 4, 4> transform(transformRaw.data());

  smtk::resource::Component::Ptr component =
    (resource->id() == frame.parent) ? nullptr : resource->find(frame.parent);
  // TODO: Handle case where UUID is an external reference.
  if (!!component || resource->id() == frame.parent)
  {
    const auto& frameProps = component
      ? component->properties().get<smtk::resource::properties::CoordinateFrame>()
      : resource->properties().get<smtk::resource::properties::CoordinateFrame>();

    // See if the parent component has a transform frame. If so, concatenate.
    // Accept either "transform" or "smtk.geometry.transform".
    // The simpler "transform" is preferred since users are more likely to enter it manually.
    std::array<std::string, 2> propertyNames{ "transform", "smtk.geometry.transform" };
    for (const auto& propertyName : propertyNames)
    {
      if (frameProps.contains(propertyName))
      {
        const auto& parentFrame = frameProps.at(propertyName);
        if (&parentFrame != &frame)
        { // Prevent reference loops where parent == self
          auto parentTransform = concatenatedParentTransforms(resource, parentFrame, visited);
          transform = parentTransform * transform;
          break;
        }
      }
    }
  }
  return transform;
}

} // anonymous namespace

void Geometry::addColorArray(
  vtkDataObject* data,
  const std::vector<double>& rgba,
  const std::string& arrayName)
{
  if (rgba.size() != 4 || rgba[3] <= 0.0)
  {
    return;
  }
  if (!data)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Bad object passed to addColorArray.");
    return;
  }

  vtkNew<vtkUnsignedCharArray> colorArray;
  colorArray->SetNumberOfComponents(4);
  colorArray->SetNumberOfTuples(1);
  colorArray->SetName(arrayName.c_str());
  for (int i = 0; i < 4; ++i)
  {
    colorArray->FillComponent(i, rgba[i] * 255.0);
  }
  data->GetFieldData()->AddArray(colorArray.GetPointer());
}

void addTransformArray(
  const smtk::resource::ResourcePtr& resource,
  vtkDataObject* data,
  const smtk::resource::properties::CoordinateFrame& frame,
  const std::string& outputArrayName)
{
  std::set<const smtk::resource::properties::CoordinateFrame*> visited;
  auto transform = concatenatedParentTransforms(resource, frame, visited);

  vtkNew<vtkDoubleArray> transformArray;
  transformArray->SetName(outputArrayName.c_str());
  transformArray->SetNumberOfTuples(16);
  for (int jj = 0; jj < 4; ++jj)
  {
    for (int ii = 0; ii < 4; ++ii)
    {
      transformArray->SetTuple1(4 * ii + jj, transform(ii, jj));
    }
  }
  data->GetFieldData()->AddArray(transformArray.GetPointer());
}

bool Geometry::addTransformArrayIfPresent(
  vtkDataObject* data,
  const smtk::resource::PersistentObjectPtr& object,
  const std::string& outputArrayName)
{
  // See if the object has a transform frame.
  // Accept either "transform" or "smtk.geometry.transform".
  // The simpler "transform" is preferred since users may be unaware of "smtk.geometry.transform".
  const auto& frameProps = object->properties().get<smtk::resource::properties::CoordinateFrame>();
  if (frameProps.contains("transform"))
  {
    const auto& objectFrame = frameProps.at("transform");
    auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(object);
    if (!resource)
    {
      auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(object);
      resource = comp ? comp->resource() : nullptr;
    }
    addTransformArray(resource, data, objectFrame, outputArrayName);
    return true;
  }
  else if (frameProps.contains("smtk.geometry.transform"))
  {
    const auto& objectFrame = frameProps.at("smtk.geometry.transform");
    auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(object);
    if (!resource)
    {
      auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(object);
      resource = comp ? comp->resource() : nullptr;
    }
    addTransformArray(resource, data, objectFrame, outputArrayName);
    return true;
  }
  return false;
}

} // namespace geometry
} // namespace vtk
} // namespace extension
} // namespace smtk
