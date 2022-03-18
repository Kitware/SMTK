//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/operators/ExportEdgesToVTK.h"

#include "smtk/model/Session.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Edge.h"
#include "smtk/model/EntityIterator.h"
#include "smtk/model/Model.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Vertex.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"

#include "vtkCellArray.h"
#include "vtkNew.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkXMLPolyDataWriter.h"

#include "smtk/extension/vtk/operators/ExportEdgesToVTK_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace model
{

void collectVertices(EntityRef& model, vtkPoints* pts, std::map<EntityRef, vtkIdType>& pointMap)
{
  EntityIterator eit;
  eit.traverse(model, ITERATE_MODELS);
  for (eit.begin(); !eit.isAtEnd(); eit.advance())
  {
    if (eit->isVertex())
    {
      double* posn = eit->as<Vertex>().coordinates();
      vtkIdType pid = pts->InsertNextPoint(posn);
      pointMap[*eit] = pid;
    }
  }
}

void insertEdges(
  EntityRef& model,
  vtkPolyData* pdt,
  vtkPoints* pts,
  std::map<EntityRef, vtkIdType>& pointMap)
{
  vtkNew<vtkCellArray> lines;
  pdt->SetLines(lines.GetPointer());

  EntityIterator eit;
  eit.traverse(model, ITERATE_MODELS);
  for (eit.begin(); !eit.isAtEnd(); eit.advance())
  {
    if (eit->isEdge())
    {
      Edge edge(*eit);
      Vertices verts = edge.vertices();
      bool hasEndVert = !verts.empty();
      bool isFirstPoint = true;
      const Tessellation* tess = eit->hasTessellation();
      Tessellation::size_type i;
      const std::vector<double>& coords(tess->coords());
      std::vector<vtkIdType> vconn;
      vtkIdType pid;
      if (tess)
      {
        if (hasEndVert)
        {
          vconn.push_back(pointMap[*verts.begin()]);
        }
        // Insert:
        //   start of edge (not tess cell) and no model vertices
        //   all interior points
        // Don't insert:
        //   start of edge (not tess cell) and model vertices bound edge (it's a model vertex point
        //     ID)
        //   end of edge (not tess cell) (it's repeated connectivity or a model vertex point ID)

        // Loop over tessellation cells. It could be many single-segment lines or one polyline or
        // anywhere between.
        for (i = tess->begin(); i != tess->end(); i = tess->nextCellOffset(i))
        {
          std::vector<int> conn;
          tess->vertexIdsOfCell(i, conn);
          for (std::vector<int>::iterator tpid = conn.begin(); tpid != conn.end(); ++tpid)
          {
            std::vector<int>::iterator nxt(tpid);
            ++nxt;
            bool shouldAdd = (nxt != conn.end());
            if (isFirstPoint)
            {
              isFirstPoint = false;
              shouldAdd &= !hasEndVert;
            }
            if (shouldAdd)
            {
              pid = pts->InsertNextPoint(&coords[3 * (*tpid)]);
              vconn.push_back(pid);
            }
          }
        }
        if (hasEndVert)
        {
          vconn.push_back(pointMap[*verts.rbegin()]);
        }
        else if (!vconn.empty())
        { // no model vertices; repeat the first point to close loop since we skipped inserting a
          // duplicate point above.
          vconn.push_back(vconn[0]);
        }
        //std::cout << edge.name() << ": " << vconn.size() << "\n";
        if (!vconn.empty())
        {
          lines->InsertNextCell(static_cast<vtkIdType>(vconn.size()), &vconn[0]);
        }
      }
      else
      {
        std::cout << edge.name() << ": no tess\n";
      }
    }
  }
}

ExportEdgesToVTK::Result ExportEdgesToVTK::operateInternal()
{
  smtk::attribute::FileItemPtr filenameItem = this->parameters()->findFile("filename");

  auto associations = this->parameters()->associations();
  auto entities = associations->as<Models>([](smtk::resource::PersistentObjectPtr obj) {
    return smtk::model::Model(std::dynamic_pointer_cast<smtk::model::Entity>(obj));
  });
  if (entities.empty())
  {
    smtkErrorMacro(this->log(), "No valid models selected for export.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  std::string filename = filenameItem->value();
  if (filename.empty())
  {
    smtkErrorMacro(this->log(), "A filename must be provided.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  vtkNew<vtkPolyData> pdt;
  vtkNew<vtkPoints> pts;
  vtkNew<vtkXMLPolyDataWriter> wri;
  pdt->SetPoints(pts.GetPointer());
  std::map<EntityRef, vtkIdType> pointMap;

  collectVertices(*entities.begin(), pts.GetPointer(), pointMap);
  insertEdges(*entities.begin(), pdt.GetPointer(), pts.GetPointer(), pointMap);

  wri->SetInputDataObject(pdt.GetPointer());
  wri->SetFileName(filename.c_str());
  bool ok = (wri->Write() != 0);

  return this->createResult(
    ok ? smtk::operation::Operation::Outcome::SUCCEEDED
       : smtk::operation::Operation::Outcome::FAILED);
}

const char* ExportEdgesToVTK::xmlDescription() const
{
  return ExportEdgesToVTK_xml;
}

} //namespace model
} // namespace smtk
