//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/opencascade/vtk/Geometry.h"

#include "smtk/session/opencascade/Resource.h"
#include "smtk/session/opencascade/Shape.h"

#include "smtk/geometry/Generator.h"

#include "smtk/io/Logger.h"

#include "vtkCellArray.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataObject.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"
#include "vtkSmartPointer.h"

#include "BRepAdaptor_Curve.hxx"
#include "BRepAdaptor_Surface.hxx"
#include "BRepMesh_CurveTessellator.hxx"
#include "BRepMesh_FastDiscret.hxx"
#include "BRepMesh_IncrementalMesh.hxx"
#include "BRepMesh_MeshAlgoFactory.hxx"
#include "BRep_Tool.hxx"
#include "BndLib_Add3dCurve.hxx"
#include "BndLib_AddSurface.hxx"
#include "Bnd_Box.hxx"
#include "GCPnts_TangentialDeflection.hxx"
#include "Poly_Polygon3D.hxx"
#include "Precision.hxx"
#include "TColgp_Array1OfPnt.hxx"
#include "TopoDS.hxx"
#include "gp_Dir.hxx"
#include "gp_Pnt.hxx"
#include "gp_Pnt2d.hxx"
#include "gp_Trsf.hxx"

namespace smtk
{
namespace session
{
namespace opencascade
{
namespace vtk
{

Geometry::Geometry(const std::shared_ptr<smtk::session::opencascade::Resource>& parent)
  : m_parent(parent)
{
}

smtk::geometry::Resource::Ptr Geometry::resource() const
{
  return std::dynamic_pointer_cast<smtk::geometry::Resource>(m_parent.lock());
}

void Geometry::queryGeometry(
  const smtk::resource::PersistentObject::Ptr& obj, CacheEntry& entry) const
{
  auto node = std::dynamic_pointer_cast<smtk::session::opencascade::Shape>(obj);
  if (!node)
  {
    entry.m_generation = Invalid;
    return;
  }
  auto resource = m_parent.lock();
  auto topoDS = resource->session()->findShape(node->id());
  if (!topoDS)
  {
    entry.m_generation = Invalid;
    return;
  }
  switch (topoDS->ShapeType())
  {
    case TopAbs_VERTEX:
    {
      auto& vert = TopoDS::Vertex(*topoDS);
      this->updateVertex(vert, entry);
      if (entry.m_geometry && node->properties().contains<std::vector<double> >("color"))
      {
        Geometry::addColorArray(
          entry.m_geometry, node->properties().at<std::vector<double> >("color"));
      }
      return;
    }
    break;
    case TopAbs_EDGE:
    {
      auto& edge = TopoDS::Edge(*topoDS);
      this->updateEdge(edge, entry);
      if (entry.m_geometry && node->properties().contains<std::vector<double> >("color"))
      {
        Geometry::addColorArray(
          entry.m_geometry, node->properties().at<std::vector<double> >("color"));
      }
      return;
    }
    break;
    case TopAbs_FACE:
    {
      auto& face = TopoDS::Face(*topoDS);
      this->updateFace(face, entry);
      if (entry.m_geometry && node->properties().contains<std::vector<double> >("color"))
      {
        Geometry::addColorArray(
          entry.m_geometry, node->properties().at<std::vector<double> >("color"));
      }
      return;
    }
    break;
    default:
      break;
  }
  // No geometry found. Either TopAbs::ShapeTypeToString(topoDS->ShapeType())
  // is an unsupported type or the mesher failed.
  entry.m_generation = Invalid;
}

int Geometry::dimension(const smtk::resource::PersistentObject::Ptr& obj) const
{
  auto resource = m_parent.lock();
  if (!resource)
  {
    return 0;
  }

  auto topoDS = resource->session()->findShape(obj->id());
  if (topoDS)
  {
    switch (topoDS->ShapeType())
    {
      case TopAbs_COMPSOLID:
        return 3;
      case TopAbs_SOLID:
        return 3;
      case TopAbs_SHELL:
        return 2;
      case TopAbs_FACE:
        return 2;
      case TopAbs_WIRE:
        return 1;
      case TopAbs_EDGE:
        return 1;
      case TopAbs_VERTEX:
        return 0;

      case TopAbs_COMPOUND:
      case TopAbs_SHAPE:
      default:
        break;
    }
  }
  return 0;
}

Geometry::Purpose Geometry::purpose(const smtk::resource::PersistentObject::Ptr& obj) const
{
  (void)obj;
  return Geometry::Surface;
}

void Geometry::update() const
{
  // Do nothing. Opencascade operations set content as needed.
}

void Geometry::geometricBounds(const DataType& geom, BoundingBox& bbox) const
{
  auto pset = vtkPointSet::SafeDownCast(geom);
  if (pset)
  {
    pset->GetBounds(bbox.data());
    return;
  }
  auto comp = vtkCompositeDataSet::SafeDownCast(geom);
  if (comp)
  {
    comp->GetBounds(bbox.data());
    return;
  }

  // Invalid bounding box:
  bbox[0] = bbox[2] = bbox[4] = 0.0;
  bbox[1] = bbox[3] = bbox[5] = -1.0;
}

void Geometry::updateVertex(const Vertex& vv, CacheEntry& entry) const
{
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
  auto coord = BRep_Tool::Pnt(vv);
  points->SetPoint(0, coord.X(), coord.Y(), coord.Z());
  polydata->Modified();
}

void Geometry::updateEdge(const Edge& ee, CacheEntry& entry) const
{
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

  TopLoc_Location location;
  auto poly3d = BRep_Tool::Polygon3D(ee, location);
  if (!poly3d)
  {
    IMeshTools_Parameters mesherParams;
    BRepMesh_IncrementalMesh mesher(ee, mesherParams);
    poly3d = BRep_Tool::Polygon3D(ee, location);
  }
  if (!poly3d)
  {
    entry.m_generation = Invalid;
    entry.m_geometry = nullptr;
    smtkWarningMacro(smtk::io::Logger::instance(), "Could not tessellate edge.");
    return;
  }

  int numPts = poly3d->NbNodes();
  vtkNew<vtkDoubleArray> nodeCoords;
  vtkNew<vtkDoubleArray> nodeParams;
  nodeCoords->SetName("point");
  nodeCoords->SetNumberOfComponents(3);
  nodeCoords->SetNumberOfTuples(numPts);
  bool havePCoords = poly3d->HasParameters();
  if (havePCoords)
  {
    nodeParams->SetName("u");
    nodeParams->SetNumberOfTuples(numPts);
  }
  std::vector<vtkIdType> conn;
  conn.reserve(numPts);
  const gp_Trsf& xform = location.Transformation();
  for (vtkIdType ii = 0; ii < numPts; ++ii)
  {
    // Yes, that's right, the discretizer uses 1-based indexing!
    gp_Pnt pp = poly3d->Nodes().Value(static_cast<Standard_Integer>(ii + 1));
    // Apply location transform to points
    pp.Transform(xform);
    nodeCoords->SetTuple3(ii, pp.X(), pp.Y(), pp.Z());
    if (havePCoords)
    {
      double pu = poly3d->Parameters().Value(static_cast<Standard_Integer>(ii + 1));
      nodeParams->SetTuple1(ii, pu);
    }
    conn.push_back(ii);
  }

  points->SetData(nodeCoords);
  if (havePCoords)
  {
    polydata->GetPointData()->AddArray(nodeParams);
  }
  lines->Reset();
  lines->InsertNextCell(static_cast<vtkIdType>(conn.size()), &conn[0]);
  polydata->Modified();
}

void Geometry::updateFace(const Face& ff, CacheEntry& entry) const
{
  entry.m_generation += 1;

  vtkPoints* points;
  vtkCellArray* polys;
  vtkSmartPointer<vtkPolyData> polydata = vtkPolyData::SafeDownCast(entry.m_geometry);
  if (polydata)
  {
    points = polydata->GetPoints();
    polys = polydata->GetPolys();
    polys->Reset();
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

  TopLoc_Location location;
  Handle_Poly_Triangulation facets = BRep_Tool::Triangulation(ff, location);
  if (!facets || facets->NbTriangles() == 0)
  {
    IMeshTools_Parameters mesherParams;
    mesherParams.Relative = true;
    // mesherParams.AdjustMinSize = true;
    // mesherParams.InParallel = true;
    mesherParams.Deflection = 0.02;
    BRepMesh_IncrementalMesh mesher(ff, mesherParams);
  }
  facets = BRep_Tool::Triangulation(ff, location);
  if (!facets || facets->NbTriangles() == 0)
  {
    entry.m_generation = Invalid;
    entry.m_geometry = nullptr;
    smtkWarningMacro(smtk::io::Logger::instance(), "Could not tessellate edge.");
    return;
  }

  const gp_Trsf& xform(location.Transformation());
  int numPts = facets->NbNodes();
  int numTris = facets->NbTriangles();
  points->SetNumberOfPoints(static_cast<vtkIdType>(numPts));
  polys->AllocateExact(numPts, 3 * numTris);
  const auto& nodes = facets->Nodes();
  const auto& tris = facets->Triangles();
  for (int ii = 0; ii < numPts; ++ii)
  {
    gp_Pnt pp = nodes(ii + 1);
    pp.Transform(xform);
    points->SetPoint(static_cast<vtkIdType>(ii), pp.X(), pp.Y(), pp.Z());
  }
  if (facets->HasUVNodes())
  {
    vtkNew<vtkDoubleArray> params;
    params->SetName("uv");
    params->SetNumberOfComponents(2);
    params->SetNumberOfTuples(numPts);
    for (int ii = 0; ii < numPts; ++ii)
    {
      const gp_Pnt2d& uv(facets->UVNode(ii + 1));
      params->SetTuple2(ii, uv.X(), uv.Y());
    }
    polydata->GetPointData()->SetTCoords(params);
  }
  for (int ii = 0; ii < numTris; ++ii)
  {
    vtkIdType triconn[3];
    const Poly_Triangle& tri(tris(ii + 1));
    int c0, c1, c2;
    tri.Get(c0, c1, c2);
    triconn[0] = static_cast<vtkIdType>(c0 - 1);
    triconn[1] = static_cast<vtkIdType>(c1 - 1);
    triconn[2] = static_cast<vtkIdType>(c2 - 1);
    polys->InsertNextCell(3, triconn);
  }
  if (facets->HasNormals())
  {
    vtkNew<vtkFloatArray> normals;
    normals->SetName("normal");
    normals->SetNumberOfComponents(3);
    normals->SetNumberOfTuples(numPts);
    for (int ii = 0; ii < numPts; ++ii)
    {
      const gp_Dir& nn(facets->Normal(ii + 1));
      normals->SetTuple3(ii, nn.X(), nn.Y(), nn.Z());
    }
    polydata->GetPointData()->SetNormals(normals);
  }
  else
  {
    vtkNew<vtkPolyDataNormals> computeNormals;
    computeNormals->SetInputDataObject(0, polydata);
    computeNormals->ComputePointNormalsOn();
    computeNormals->SplittingOff();
    computeNormals->ConsistencyOn();
    computeNormals->Update();
    polydata->GetPointData()->SetNormals(computeNormals->GetOutput()->GetPointData()->GetNormals());
  }

  polydata->Modified();
}

} // namespace vtk
} // namespace opencascade
} // namespace session
} // namespace smtk
