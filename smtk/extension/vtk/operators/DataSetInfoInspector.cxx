//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================

#include "smtk/io/Logger.h"

#include "smtk/extension/vtk/geometry/Backend.h"
#include "smtk/extension/vtk/geometry/Geometry.h"
#include "smtk/extension/vtk/operators/DataSetInfoInspector.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/geometry/Resource.h"
#include "smtk/geometry/queries/BoundingBox.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/resource/Component.h"

#include "vtkCellIterator.h"
#include "vtkCellType.h"
#include "vtkCellTypes.h"
#include "vtkDataArray.h"
#include "vtkDataSet.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkExtractSurface.h"
#include "vtkPolyData.h"

#include "smtk/extension/vtk/operators/DataSetInfoInspector_xml.h"

#include <array>

namespace smtk
{
namespace geometry
{

// Hold summary information for cell- and point-data arrays on renderable geometry.
struct ArrayInfo
{
  std::string name;
  int numberOfComponents;
};

// Hold summary information on renderable geometry mapped to each component.
struct DataSetInfo
{
  DataSetInfo(const smtk::resource::Component::Ptr& comp, vtkSmartPointer<vtkDataObject> data)
    : component(comp)
  {
    this->numberOfPoints = data->GetNumberOfElements(vtkDataObject::POINT);
    // Reset cell counts (by type):
    for (auto& count : cellCounts)
    {
      count = 0;
    }
    if (auto* dataset = vtkDataSet::SafeDownCast(data))
    {
      dataset->GetBounds(this->bounds.data());
      auto* it = dataset->NewCellIterator();
      for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextCell())
      {
        ++cellCounts[it->GetCellType()];
      }
      it->Delete();
    }
    // TODO: Array statistics
  }

  DataSetInfo(const DataSetInfo&) = default;

  bool operator<(const DataSetInfo& other) const
  {
    return this->component->id() < other.component->id();
  }

  smtk::resource::Component::Ptr component;
  vtkIdType numberOfPoints;
  std::vector<ArrayInfo> arraySummaries;
  std::array<vtkIdType, VTK_NUMBER_OF_CELL_TYPES> cellCounts;
  std::array<double, 6> bounds;
};

// Given the associated components, fetch their renderable geometry and summarize it.
void createSummary(
  const smtk::attribute::ReferenceItem::Ptr& assoc,
  std::set<DataSetInfo>& summaries)
{
  smtk::extension::vtk::geometry::Backend vtk;
  for (std::size_t ii = 0; ii < assoc->numberOfValues(); ++ii)
  {
    if (assoc->isSet(ii))
    {
      if (const auto& comp = assoc->valueAs<smtk::resource::Component>(ii))
      {
        auto* geometryResource = dynamic_cast<smtk::geometry::Resource*>(comp->parentResource());
        if (geometryResource)
        {
          const auto& geom = geometryResource->geometry(vtk);
          if (geom)
          {
            const auto& vtkGeom =
              dynamic_cast<const smtk::extension::vtk::geometry::Geometry&>(*geom);
            auto data = vtkGeom.data(comp);
            if (data)
            {
              summaries.emplace(comp, data);
            }
          }
        }
      }
    }
  }
}

// Given the summary info, pack it into a result attribute.
void prepareResult(DataSetInfoInspector::Result& result, std::set<DataSetInfo>& summaries)
{
  using namespace smtk::attribute;
  auto info = result->findGroup("information");
  info->reset();
  info->setNumberOfGroups(summaries.size());
  std::size_t ii = 0;
  for (const auto& summary : summaries)
  {
    info->findAs<ComponentItem>(ii, "component")->setValue(summary.component);
    auto boundsItem = info->findAs<DoubleItem>(ii, "bounds");
    boundsItem->setValues(summary.bounds.data(), summary.bounds.data() + 6);
    info->findAs<IntItem>(ii, "point count")->setValue(summary.numberOfPoints);
    int jj = 0;
    for (const auto& count : summary.cellCounts)
    {
      if (count != 0)
      {
        std::string cellTypeName = vtkCellTypes::GetClassNameFromTypeId(jj);
        std::string itemName = cellTypeName + " count";
        if (auto item = info->findAs<IntItem>(ii, itemName))
        {
          item->setValue(count);
        }
      }
      ++jj;
    }
    ++ii;
  }
}

DataSetInfoInspector::Result DataSetInfoInspector::operateInternal()
{
  auto result = this->createResult(smtk::operation::Operation::Outcome::FAILED);

  std::set<DataSetInfo> summary;
  createSummary(this->parameters()->associations(), summary);
  prepareResult(result, summary);

  result->findInt("outcome")->setValue(static_cast<int>(DataSetInfoInspector::Outcome::SUCCEEDED));
  return result;
}

const char* DataSetInfoInspector::xmlDescription() const
{
  return DataSetInfoInspector_xml;
}
} // namespace geometry
} // namespace smtk
