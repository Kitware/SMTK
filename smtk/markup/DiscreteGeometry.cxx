//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/DiscreteGeometry.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/markup/Resource.h"

#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkDataSet.h"
#include "vtkPointData.h"

namespace smtk
{
namespace markup
{

using namespace smtk::string::literals; // for ""_token

namespace
{

bool updateFieldArrays(
  vtkFieldData* arrays,
  smtk::string::Token fieldType,
  std::unordered_map<smtk::string::Token, const Field*>& existingFields,
  DiscreteGeometry* self,
  smtk::operation::Operation::Result trackedChanges,
  const std::set<smtk::string::Token>& ignore = std::set<smtk::string::Token>())
{
  bool didChange = false;
  if (!arrays)
  {
    smtkWarningMacro(smtk::io::Logger::instance(), "No " << fieldType.data() << " array data.");
    return didChange;
  }
  auto* resource = dynamic_cast<Resource*>(self->parentResource());
  auto created = trackedChanges->findComponent("created");
  auto modified = trackedChanges->findComponent("modified");
  int numArrays = arrays->GetNumberOfArrays();
  for (int ii = 0; ii < numArrays; ++ii)
  {
    if (auto* arr = arrays->GetArray(ii))
    {
      if (ignore.find(smtk::string::Token(arr->GetName())) != ignore.end())
      {
        continue; // Skip fields we are told to ignore.
      }
      didChange = true;
      auto it = existingFields.find(arr->GetName());
      if (it == existingFields.end())
      {
        // Search the resource for a matching field with the proper domain and name.
        // TODO: Really, we should be able to ask the domain for component fields that apply to it:
        // auto domain = resource->domains().find(arr->GetName(), fieldType);
        bool didFind = false;
        auto fields =
          resource->filterAs<std::set<smtk::markup::Field::Ptr>>("'smtk::markup::Field'");
        for (const auto& candidate : fields)
        {
          if (candidate->fieldType() == fieldType && candidate->name() == arr->GetName())
          {
            candidate->shapes().connect(self);
            modified->appendValue(candidate);
            didFind = true;
            break;
          }
        }
        if (!didFind)
        {
          // Add a new field
          auto field = resource->createNode<Field>(arr->GetName(), fieldType);
          field->shapes().connect(self);
          created->appendValue(field);
        }
      }
      else
      {
        // Re-use the existing field (marking it modified)
        modified->appendValue(const_cast<Field*>(it->second)->shared_from_this());
        // Remove from oldFields so we don't destroy the node.
        existingFields.erase(it);
      }
    }
  }
  return didChange;
}

} // anonymous namespace

DiscreteGeometry::~DiscreteGeometry() = default;

vtkSmartPointer<vtkDataObject> DiscreteGeometry::shape() const
{
  vtkSmartPointer<vtkDataObject> empty;
  return empty;
}

bool DiscreteGeometry::updateChildren(
  vtkSmartPointer<vtkDataObject> newShape,
  ShapeOptions& options)
{
  bool didChange = false;
  if (this->shape() == newShape)
  {
    return didChange;
  }

  auto& trackedChanges = options.trackedChanges;
  if (auto* dset = vtkDataSet::SafeDownCast(newShape))
  {
    auto* resource = dynamic_cast<Resource*>(this->parentResource());
    std::unordered_map<smtk::string::Token, std::unordered_map<smtk::string::Token, const Field*>>
      oldFields;
    this->incoming<arcs::FieldsToShapes>().visit(
      [&](const Field* field) { oldFields[field->fieldType()][field->name()] = field; });

    auto* dsa = dset->GetAttributes(vtkDataObject::POINT);
    auto fieldType = "points"_token;
    didChange |= updateFieldArrays(
      dsa,
      fieldType,
      oldFields[fieldType],
      this,
      trackedChanges,
      { { "vtkOriginalPointIds"_token } });

    dsa = dset->GetAttributes(vtkDataObject::CELL);
    fieldType = "cells"_token;
    didChange |= updateFieldArrays(
      dsa,
      fieldType,
      oldFields[fieldType],
      this,
      trackedChanges,
      { { "vtkOriginalCellIds"_token } });

    auto* fda = dset->GetAttributesAsFieldData(vtkDataObject::FIELD);
    fieldType = "global"_token;
    didChange |= updateFieldArrays(fda, fieldType, oldFields[fieldType], this, trackedChanges);

    auto expunged = trackedChanges->findComponent("expunged");
    for (const auto& fieldsOfType : oldFields)
    {
      for (const auto& entry : fieldsOfType.second)
      {
        /// Remove fields no longer referenced.
        auto field =
          std::dynamic_pointer_cast<Field>(const_cast<Field*>(entry.second)->shared_from_this());
        if (resource->remove(field))
        {
          expunged->appendValue(field);
          didChange = true;
        }
      }
    }
  }
  if (didChange)
  {
    trackedChanges->findComponent("modified")->appendValue(shared_from_this());
  }
  return didChange;
}

} // namespace markup
} // namespace smtk
