//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/geometry/DistanceTo.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/extension/vtk/geometry/Backend.h"
#include "smtk/extension/vtk/geometry/Geometry.h"
#include "smtk/extension/vtk/model/vtkAuxiliaryGeometryExtension.h"

#include "smtk/geometry/Geometry.h"
#include "smtk/geometry/Resource.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Entity.h"

#include "smtk/operation/queries/SynchronizedCache.h"

#include <vtkCellLocator.h>
#include <vtkDataSet.h>
#include <vtkGenericCell.h>
#include <vtkPointSet.h>
#include <vtkSmartPointer.h>

namespace
{
struct CellLocatorCache : public smtk::operation::SynchronizedCache
{
  void synchronize(const smtk::operation::Operation&, const smtk::operation::Operation::Result&)
    override;

  std::unordered_map<smtk::common::UUID, vtkSmartPointer<vtkCellLocator>> m_caches;
};

void CellLocatorCache::synchronize(
  const smtk::operation::Operation&,
  const smtk::operation::Operation::Result& result)
{
  for (const auto& component :
       { result->findComponent("expunged"), result->findComponent("modified") })
  {
    for (std::size_t i = 0; i < component->numberOfValues(); ++i)
    {
      m_caches.erase(component->value(i)->id());
    }
  }
}
} // namespace

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace geometry
{
std::pair<double, std::array<double, 3>> DistanceTo::operator()(
  const smtk::resource::ComponentPtr& component,
  const std::array<double, 3>& input) const
{
  static constexpr const double nan = std::numeric_limits<double>::quiet_NaN();
  std::pair<double, std::array<double, 3>> returnValue(nan, { { nan, nan, nan } });

  smtk::geometry::Resource::Ptr resource =
    std::dynamic_pointer_cast<smtk::geometry::Resource>(component->resource());

  if (!resource)
  {
    return returnValue;
  }

  vtkDataSet* data = nullptr;

  smtk::extension::vtk::geometry::Backend vtk;
  const auto& geometry = resource->geometry(vtk);
  if (geometry)
  {
    try
    {
      const auto& vtkGeometry =
        dynamic_cast<const smtk::extension::vtk::geometry::Geometry&>(*geometry);

      data = vtkDataSet::SafeDownCast(vtkGeometry.data(component));
    }
    catch (std::bad_cast&)
    {
      return returnValue;
    }
  }

  if (data == nullptr)
  {
    return returnValue;
  }

  vtkSmartPointer<vtkDataObject> cachedAuxData; // Keep here so it stays in scope

  // TODO: Handle composite data, not just vtkPointSet data.
  vtkPointSet* pdata = vtkPointSet::SafeDownCast(data);
  if (pdata)
  {
    smtk::model::Entity::Ptr entity = std::dynamic_pointer_cast<smtk::model::Entity>(component);
    if (entity && entity->isAuxiliaryGeometry())
    { // It may be that we don't have a tessellation yet; create one if we can
      smtk::model::AuxiliaryGeometry aux(entity);
      std::vector<double> bbox;
      auto agext = vtkAuxiliaryGeometryExtension::create();
      if (agext->canHandleAuxiliaryGeometry(aux, bbox))
      {
        cachedAuxData = vtkAuxiliaryGeometryExtension::fetchCachedGeometry(aux);
        pdata = vtkPointSet::SafeDownCast(cachedAuxData);
      }
    }

    CellLocatorCache& pointLocatorCache = resource->queries().cache<CellLocatorCache>();

    auto search = pointLocatorCache.m_caches.find(component->id());
    if (search == pointLocatorCache.m_caches.end())
    {
      search =
        pointLocatorCache.m_caches.emplace(component->id(), vtkSmartPointer<vtkCellLocator>::New())
          .first;
      search->second->SetDataSet(pdata);
      search->second->BuildLocator();
    }

    vtkSmartPointer<vtkGenericCell> cell = vtkSmartPointer<vtkGenericCell>::New();

    vtkIdType cellId;
    int subId;

    search->second->FindClosestPoint(
      input.data(), returnValue.second.data(), cell, cellId, subId, returnValue.first);
    returnValue.first = sqrt(returnValue.first);
  }

  return returnValue;
};
} // namespace geometry
} // namespace vtk
} // namespace extension
} // namespace smtk
