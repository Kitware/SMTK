//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/polygon/vtk/Geometry.h"

#include "smtk/session/polygon/internal/Edge.h"
#include "smtk/session/polygon/internal/Model.h"
#include "smtk/session/polygon/internal/Model.txx"
#include "smtk/session/polygon/internal/Vertex.h"

#include "smtk/session/polygon/Resource.h"

#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"

#include "smtk/geometry/Generator.h"

#include "vtkCellArray.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataObject.h"
#include "vtkNew.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"

namespace poly = boost::polygon;
using namespace boost::polygon::operators;

namespace smtk
{
namespace session
{
namespace polygon
{

namespace
{
template<typename T>
void preparePointsForBoost(
  T& ppts,
  internal::Coord& denx,
  internal::Coord& deny,
  bool useExistingDenominators)
{
  // If we aren't given denx + deny, loop through ppts to find them:
  if (!useExistingDenominators)
  {
    if (ppts.empty())
    {
      denx = 1;
      deny = 1;
    }
    else
    {
      internal::Coord xblo, xbhi;
      internal::Coord yblo, ybhi;
      auto pit = ppts.begin();
      xbhi = pit->x();
      xblo = xbhi;
      ybhi = pit->y();
      yblo = ybhi;
      for (++pit; pit != ppts.end(); ++pit)
      {
        if (xbhi < pit->x())
        {
          xbhi = pit->x();
        }
        else if (xblo > pit->x())
        {
          xblo = pit->x();
        }
        if (ybhi < pit->y())
        {
          ybhi = pit->y();
        }
        else if (yblo > pit->y())
        {
          yblo = pit->y();
        }
      }
      internal::Coord dx = xbhi;
      if (xblo < 0 && -xblo > dx)
      {
        dx = -xblo;
      }
      internal::Coord dy = ybhi;
      if (yblo < 0 && -yblo > dy)
      {
        dy = -yblo;
      }
      double lx = dx > 0 ? (std::log(dx) / std::log(2.0)) : 1.0;
      double ly = dy > 0 ? (std::log(dy) / std::log(2.0)) : 1.0;
      denx = lx > 31 ? (1 << static_cast<int>(std::ceil(lx - 31))) : 1;
      deny = ly > 31 ? (1 << static_cast<int>(std::ceil(ly - 31))) : 1;
    }
  }
  bool denom = denx > 1 || deny > 1;
  // If we need to truncate points, loop through them and do it:
  if (denom)
  {
    for (auto fpit = ppts.begin(); fpit != ppts.end(); ++fpit)
    {
      fpit->x(fpit->x() / denx);
      fpit->y(fpit->y() / deny);
    }
  }
}
} // anonymous namespace

namespace vtk
{

Geometry::Geometry(const std::shared_ptr<smtk::session::polygon::Resource>& parent)
  : m_parent(parent)
{
}

smtk::geometry::Resource::Ptr Geometry::resource() const
{
  return std::dynamic_pointer_cast<smtk::geometry::Resource>(m_parent.lock());
}

void Geometry::queryGeometry(const smtk::resource::PersistentObject::Ptr& obj, CacheEntry& entry)
  const
{
  auto ent = std::dynamic_pointer_cast<smtk::model::Entity>(obj);
  if (!ent)
  {
    entry.m_generation = Invalid;
    return;
  }
  auto resource = m_parent.lock();
  auto polyEnt = resource->findStorage<internal::entity>(ent->id());
  auto* polyModel = polyEnt ? polyEnt->parentAs<internal::pmodel>() : nullptr;
  if (std::dynamic_pointer_cast<internal::pmodel>(polyEnt))
  {
    // The model itself does not have geometry; its children do.
    entry.m_generation = Invalid;
    return;
  }
  switch (ent->dimension())
  {
    case 0:
    {
      auto vert = std::dynamic_pointer_cast<internal::vertex>(polyEnt);
      if (vert)
      {
        this->updateVertex(polyModel, vert, entry);
        if (entry.m_geometry && ent->properties().contains<std::vector<double>>("color"))
        {
          Geometry::addColorArray(
            entry.m_geometry, ent->properties().at<std::vector<double>>("color"));
        }
        Geometry::addTransformArrayIfPresent(entry.m_geometry, ent);
        return;
      } // else this->eraseCache(ent->id()); ???
      break;
    }
    case 1:
    {
      // auto edge = resource->findStorage<internal::edge>(ent->id());
      auto edge = std::dynamic_pointer_cast<internal::edge>(polyEnt);
      if (edge)
      {
        this->updateEdge(polyModel, edge, entry);
        if (entry.m_geometry && ent->properties().contains<std::vector<double>>("color"))
        {
          Geometry::addColorArray(
            entry.m_geometry, ent->properties().at<std::vector<double>>("color"));
        }
        Geometry::addTransformArrayIfPresent(entry.m_geometry, ent);
        return;
      }
      break;
    }
    case 2:
    {
      this->updateFace(polyModel, ent, entry);
      if (entry.m_geometry && ent->properties().contains<std::vector<double>>("color"))
      {
        Geometry::addColorArray(
          entry.m_geometry, ent->properties().at<std::vector<double>>("color"));
      }
      Geometry::addTransformArrayIfPresent(entry.m_geometry, ent);
      return;
      break;
    }
  }
  entry.m_generation = Invalid;
}

int Geometry::dimension(const smtk::resource::PersistentObject::Ptr& obj) const
{
  auto ent = std::dynamic_pointer_cast<smtk::model::Entity>(obj);
  if (ent)
  {
    return ent->dimension();
  }
  return 0;
}

Geometry::Purpose Geometry::purpose(const smtk::resource::PersistentObject::Ptr& obj) const
{
  auto ent = std::dynamic_pointer_cast<smtk::model::Entity>(obj);
  if (ent)
  {
    return ent->isInstance() ? Geometry::Glyph : Geometry::Surface;
  }
  return Geometry::Surface;
}

void Geometry::update() const
{
  // Do nothing. Polygon operations set content as needed.
}

void Geometry::geometricBounds(const DataType& geom, BoundingBox& bbox) const
{
  auto* pset = vtkPointSet::SafeDownCast(geom);
  if (pset)
  {
    pset->GetBounds(bbox.data());
    return;
  }
  auto* comp = vtkCompositeDataSet::SafeDownCast(geom);
  if (comp)
  {
    comp->GetBounds(bbox.data());
    return;
  }

  // Invalid bounding box:
  bbox[0] = bbox[2] = bbox[4] = 0.0;
  bbox[1] = bbox[3] = bbox[5] = -1.0;
}

void Geometry::updateVertex(const PolyModel& polyModel, const VertexPtr& vv, CacheEntry& entry)
  const
{
  if (!vv)
  {
    entry.m_generation = Invalid;
    return;
  }
  entry.m_generation += 1; // This works even when m_generation is Invalid.

  vtkPoints* points;
  vtkSmartPointer<vtkPolyData> polydata = vtkPolyData::SafeDownCast(entry.m_geometry);
  if (polydata)
  {
    points = polydata->GetPoints();
  }
  else
  {
    polydata = vtkSmartPointer<vtkPolyData>::New();
    entry.m_geometry = polydata;
    vtkNew<vtkPoints> pts;
    vtkNew<vtkCellArray> verts;
    polydata->SetVerts(verts);
    polydata->SetPoints(pts);
    pts->SetNumberOfPoints(1);
    vtkIdType ptid = 0;
    polydata->InsertNextCell(VTK_VERTEX, 1, &ptid);
    points = pts;
  }
  double snappedPt[3];
  polyModel->liftPoint(vv->point(), snappedPt);
  points->SetPoint(0, snappedPt);
  polydata->Modified();
}

void Geometry::updateEdge(const PolyModel& polyModel, const EdgePtr& ee, CacheEntry& entry) const
{
  if (!ee)
  {
    entry.m_generation = Invalid;
    return;
  }
  entry.m_generation += 1;

  vtkPoints* points;
  vtkCellArray* lines;
  vtkSmartPointer<vtkPolyData> polydata = vtkPolyData::SafeDownCast(entry.m_geometry);
  if (polydata)
  {
    points = polydata->GetPoints();
    lines = polydata->GetLines();
  }
  else
  {
    polydata = vtkSmartPointer<vtkPolyData>::New();
    entry.m_geometry = polydata;
    vtkNew<vtkPoints> pts;
    vtkNew<vtkCellArray> cells;
    polydata->SetPoints(pts);
    polydata->SetLines(cells);
    points = pts;
    lines = cells;
  }
  std::size_t numPts = ee->pointsSize();
  internal::PointSeq::const_iterator ptIt;
  std::vector<vtkIdType> conn;
  conn.reserve(numPts);
  // conn.push_back(static_cast<int>(numPts));
  bool isPeriodic = (*ee->pointsBegin() == *ee->pointsRBegin());
  std::size_t numUniquePts = isPeriodic ? numPts - 1 : numPts;
  points->SetNumberOfPoints(static_cast<vtkIdType>(numUniquePts));
  std::size_t ii;
  for (ptIt = ee->pointsBegin(), ii = 0; ii < numUniquePts; ++ptIt, ++ii)
  {
    double coords[3];
    polyModel->liftPoint(*ptIt, coords);
    conn.push_back(ii);
    points->SetPoint(ii, coords);
  }
  if (isPeriodic)
  {
    conn.push_back(conn[0]); // repeat initial point instead of adding a duplicate.
  }
  lines->Reset();
  lines->InsertNextCell(static_cast<vtkIdType>(conn.size()), conn.data());
  polydata->Modified();
}

void Geometry::updateFace(
  const PolyModel&,
  const smtk::model::EntityPtr& modelFace,
  CacheEntry& entry) const
{
  if (!modelFace)
  {
    entry.m_generation = Invalid;
    return;
  }
  entry.m_generation += 1;

  vtkPoints* points;
  vtkCellArray* polys;
  vtkSmartPointer<vtkPolyData> polydata = vtkPolyData::SafeDownCast(entry.m_geometry);
  if (polydata)
  {
    points = polydata->GetPoints();
    polys = polydata->GetPolys();
  }
  else
  {
    polydata = vtkSmartPointer<vtkPolyData>::New();
    entry.m_geometry = polydata;
    vtkNew<vtkPoints> pts;
    vtkNew<vtkCellArray> cells;
    polydata->SetPoints(pts);
    polydata->SetPolys(cells);
    points = pts;
    polys = cells;
  }
  auto faceRec = modelFace->referenceAs<smtk::model::Face>();
  smtk::model::Model model = faceRec.owningModel();
  auto polyResource = m_parent.lock();
  auto polyModel = polyResource->findStorage<internal::pmodel>(model.entity());
  poly::polygon_set_data<internal::Coord> bpolys;
  poly::polygon_data<internal::Coord> pface;
  smtk::model::Loops outerLoops = faceRec.positiveUse().loops();
  polys->Reset();
  // smtk::model::Tessellation* smtkTess = faceRec.resetTessellation();
  //std::cout << "Tessellate " << faceRec.name() << "\n";
  for (smtk::model::Loops::iterator lit = outerLoops.begin(); lit != outerLoops.end(); ++lit)
  {
    smtk::model::Loops innerLoops = lit->containedLoops();
    int npp = 1 + static_cast<int>(innerLoops.size());
    std::vector<std::vector<internal::Point>> pp2(npp);
    int ll = 0;
    //std::cout << "  Loop " << lit->name() << "\n";
    polyModel->pointsInLoopOrder(pp2[ll], *lit);
    internal::Coord denx, deny;
    preparePointsForBoost(pp2[ll], denx, deny, false);
    bool denom = denx > 1 || deny > 1;
    pface.set(pp2[ll].rbegin(), pp2[ll].rend()); // boost likes its loops backwards
    poly::assign(bpolys, pface);
    ++ll;
    for (smtk::model::Loops::iterator ilit = innerLoops.begin(); ilit != innerLoops.end();
         ++ilit, ++ll)
    {
      //std::cout << "    Inner Loop " << ilit->name() << "\n";
      polyModel->pointsInLoopOrder(pp2[ll], *ilit);
      preparePointsForBoost(pp2[ll], denx, deny, true);
      poly::polygon_data<internal::Coord> loop;
      loop.set(pp2[ll].rbegin(), pp2[ll].rend());
      bpolys -= loop;
    }

    // Add the component to the face tessellation:
    std::vector<poly::polygon_data<internal::Coord>> tess;
    bpolys.get_trapezoids(tess);
    std::vector<poly::polygon_data<internal::Coord>>::const_iterator pit;
    double smtkPt[3];
    internal::Point ipt;
    for (pit = tess.begin(); pit != tess.end(); ++pit)
    {
      poly::polygon_data<internal::Coord>::iterator_type pcit;
      pcit = poly::begin_points(*pit);
      std::array<vtkIdType, 3> triConn;
      ipt = !denom ? *pcit : internal::Point(pcit->x() * denx, pcit->y() * deny);
      polyModel->liftPoint(ipt, smtkPt);
      triConn[0] = points->InsertNextPoint(smtkPt);
      //std::cout << "  " << triConn[1] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
      ++pcit;
      ipt = !denom ? *pcit : internal::Point(pcit->x() * denx, pcit->y() * deny);
      polyModel->liftPoint(ipt, smtkPt);
      triConn[2] = points->InsertNextPoint(smtkPt);
      ++pcit;
      //std::cout << "  " << triConn[3] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
      for (; pcit != poly::end_points(*pit); ++pcit)
      {
        triConn[1] = triConn[2];
        ipt = !denom ? *pcit : internal::Point(pcit->x() * denx, pcit->y() * deny);
        polyModel->liftPoint(ipt, smtkPt);
        triConn[2] = points->InsertNextPoint(smtkPt);
        //std::cout << "  " << triConn[3] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
        polys->InsertNextCell(3, triConn.data());
      }
      //std::cout << "\n";
      //faceRec.setColor(1., 1., 1., 1.);
    }
  }
  polydata->Modified();
}

class RegisterPolygonVTKBackend : public smtk::geometry::Supplier<RegisterPolygonVTKBackend>
{
public:
  bool valid(const Specification& in) const override
  {
    smtk::extension::vtk::geometry::Backend backend;
    return std::get<1>(in).index() == backend.index();
  }

  GeometryPtr operator()(const Specification& in) override
  {
    auto rsrc = std::dynamic_pointer_cast<smtk::session::polygon::Resource>(std::get<0>(in));
    if (rsrc)
    {
      auto* provider = new Geometry(rsrc);
      return GeometryPtr(provider);
    }
    throw std::invalid_argument("Not a polygon resource.");
    return nullptr;
  }
};

static bool polygonVTKBackend = RegisterPolygonVTKBackend::registerClass();

} // namespace vtk
} // namespace polygon
} // namespace session
} // namespace smtk
