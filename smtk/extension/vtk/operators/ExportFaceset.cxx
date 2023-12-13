//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================

#include "smtk/extension/vtk/operators/ExportFaceset.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/extension/vtk/geometry/Backend.h"
#include "smtk/extension/vtk/geometry/Geometry.h"
#include "smtk/io/Logger.h"
#include "smtk/model/Model.h"

#include "vtkAppendPolyData.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkErrorCode.h"
#include "vtkNew.h"
#include "vtkOBJWriter.h"
#include "vtkPLYWriter.h"
#include "vtkPolyData.h"
#include "vtkSTLWriter.h"
#include "vtkSmartPointer.h"
#include "vtkTriangleFilter.h"
#include "vtkUnstructuredGrid.h"

#include "smtk/extension/vtk/operators/ExportFaceset_xml.h"

#include <list>

namespace smtk
{
namespace geometry
{

// \todo Remove this function and replace with a call to vtkDataSet::GetMaxSpatialDimension().
// This will require an update of ParaView/VTK versions in the SMTK build and CMB-superbuild.
int DimensionFromCellType(int cellType)
{
  switch (cellType)
  {
    case VTK_VERTEX: // fall through
    case VTK_POLY_VERTEX:
      return 0;
    case VTK_LINE:             // fall through
    case VTK_POLY_LINE:        // fall through
    case VTK_QUADRATIC_EDGE:   // fall through
    case VTK_CUBIC_LINE:       // fall through
    case VTK_LAGRANGE_CURVE:   // fall through
    case VTK_PARAMETRIC_CURVE: // fall through
    case VTK_HIGHER_ORDER_EDGE:
      return 1;
    case VTK_TRIANGLE:                // fall through
    case VTK_TRIANGLE_STRIP:          // fall through
    case VTK_POLYGON:                 // fall through
    case VTK_PIXEL:                   // fall through
    case VTK_QUAD:                    // fall through
    case VTK_QUADRATIC_TRIANGLE:      // fall through
    case VTK_QUADRATIC_QUAD:          // fall through
    case VTK_QUADRATIC_POLYGON:       // fall through
    case VTK_BIQUADRATIC_QUAD:        // fall through
    case VTK_QUADRATIC_LINEAR_QUAD:   // fall through
    case VTK_BIQUADRATIC_TRIANGLE:    // fall through
    case VTK_LAGRANGE_TRIANGLE:       // fall through
    case VTK_LAGRANGE_QUADRILATERAL:  // fall through
    case VTK_PARAMETRIC_SURFACE:      // fall through
    case VTK_PARAMETRIC_TRI_SURFACE:  // fall through
    case VTK_PARAMETRIC_QUAD_SURFACE: // fall through
    case VTK_HIGHER_ORDER_TRIANGLE:   // fall through
    case VTK_HIGHER_ORDER_QUAD:       // fall through
    case VTK_HIGHER_ORDER_POLYGON:
      return 2;
    case VTK_TETRA:                            // fall through
    case VTK_VOXEL:                            // fall through
    case VTK_HEXAHEDRON:                       // fall through
    case VTK_WEDGE:                            // fall through
    case VTK_PYRAMID:                          // fall through
    case VTK_PENTAGONAL_PRISM:                 // fall through
    case VTK_HEXAGONAL_PRISM:                  // fall through
    case VTK_QUADRATIC_TETRA:                  // fall through
    case VTK_QUADRATIC_HEXAHEDRON:             // fall through
    case VTK_QUADRATIC_WEDGE:                  // fall through
    case VTK_QUADRATIC_PYRAMID:                // fall through
    case VTK_TRIQUADRATIC_HEXAHEDRON:          // fall through
    case VTK_QUADRATIC_LINEAR_WEDGE:           // fall through
    case VTK_BIQUADRATIC_QUADRATIC_WEDGE:      // fall through
    case VTK_BIQUADRATIC_QUADRATIC_HEXAHEDRON: // fall through
    case VTK_CONVEX_POINT_SET:                 // fall through
    case VTK_POLYHEDRON:                       // fall through
    case VTK_PARAMETRIC_TETRA_REGION:          // fall through
    case VTK_PARAMETRIC_HEX_REGION:            // fall through
    case VTK_HIGHER_ORDER_TETRAHEDRON:         // fall through
    case VTK_HIGHER_ORDER_WEDGE:               // fall through
    case VTK_HIGHER_ORDER_PYRAMID:             // fall through
    case VTK_HIGHER_ORDER_HEXAHEDRON:          // fall through
    case VTK_LAGRANGE_TETRAHEDRON:             // fall through
    case VTK_LAGRANGE_HEXAHEDRON:              // fall through
    case VTK_LAGRANGE_WEDGE:                   // fall through
    case VTK_LAGRANGE_PYRAMID:
      return 3;
    case VTK_EMPTY_CELL: // fall through
    default:
      break;
  }
  return -1;
}

int EstimateParametricDimension(const vtkSmartPointer<vtkDataSet>& dset)
{
  int dimension = -1;
  if (!dset)
  {
    return dimension;
  }

  vtkIdType nc = dset->GetNumberOfCells();
  auto ns = static_cast<double>(vtkMath::CeilLog2(nc));
  auto skip = static_cast<vtkIdType>(nc < 10000 ? 1 : (nc > 10e7 ? nc / 1.0e5 : nc / ns));
  for (vtkIdType ii = 0; ii < nc; ii += skip)
  {
    dimension = std::max(dimension, DimensionFromCellType(dset->GetCellType(ii)));
  }
  return dimension;
}

std::list<vtkSmartPointer<vtkPolyData>> ExtractFaceset(vtkDataSet* dset)
{
  std::list<vtkSmartPointer<vtkPolyData>> retValue;
  if (!dset)
  {
    return retValue;
  }

  // Identify the surface geometry we need to extractf rom this dataset.
  vtkSmartPointer<vtkDataSet> surfaceGeometry;
  int dimension = EstimateParametricDimension(dset);
  auto* ugrid = vtkUnstructuredGrid::SafeDownCast(dset);
  if (dimension == 3 && ugrid)
  {
    // Extract boundary of unstructured grid as a surface.
    vtkNew<vtkDataSetSurfaceFilter> surfaceFilter;
    surfaceFilter->SetInputData(ugrid);
    surfaceFilter->Update();
    surfaceGeometry = surfaceFilter->GetOutput();
  }
  else if (dimension == 2)
  {
    // The entire dataset is a surface geometry.
    surfaceGeometry = dset;
  }

  if (surfaceGeometry)
  {
    // Convert the surface geometry into triangles.
    vtkNew<vtkTriangleFilter> triangleFilter;
    triangleFilter->SetInputData(surfaceGeometry);
    triangleFilter->PassVertsOff();
    triangleFilter->PassLinesOff();
    triangleFilter->Update();
    if (auto* faceset = triangleFilter->GetOutput())
    {
      retValue.emplace_back(faceset);
    }
  }
  return retValue;
}

std::list<vtkSmartPointer<vtkPolyData>> ExtractFaceset(vtkDataObject* data)
{
  if (auto* dset = vtkDataSet::SafeDownCast(data))
  {
    return ExtractFaceset(dset);
  }

  std::list<vtkSmartPointer<vtkPolyData>> retValue;
  if (auto* cdata = vtkCompositeDataSet::SafeDownCast(data))
  {
    auto* it = cdata->NewIterator();
    it->SkipEmptyNodesOn();
    for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextItem())
    {
      auto* dobj = it->GetCurrentDataObject();
      std::list<vtkSmartPointer<vtkPolyData>> partFaceset = ExtractFaceset(dobj);
      retValue.insert(retValue.end(), partFaceset.begin(), partFaceset.end());
    }
    it->Delete();
  }

  return retValue;
}

ExportFaceset::Result ExportFaceset::operateInternal()
{
  smtk::attribute::FileItemPtr filenameItem = this->parameters()->findFile("filename");
  const std::string filename = filenameItem->value();
  auto assoc = this->parameters()->associations();
  if (filename.empty() || filename.size() < 5)
  {
    smtkErrorMacro(this->log(), "A valid filename and extension must be provided.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  if (!assoc)
  {
    smtkErrorMacro(this->log(), "No input associated.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  if (assoc->numberOfValues() > 1 || !assoc->isSet())
  {
    smtkErrorMacro(this->log(), "Expected single associated input resource.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  bool fileWritingCalled = false;
  unsigned long errorCode = 0;
  if (const auto& comp = assoc->valueAs<smtk::resource::Component>())
  {
    auto* geometryResource = dynamic_cast<smtk::geometry::Resource*>(comp->parentResource());
    if (geometryResource)
    {
      smtk::extension::vtk::geometry::Backend vtk;
      const auto& geom = geometryResource->geometry(vtk);
      if (geom)
      {
        const auto& vtkGeom = dynamic_cast<const smtk::extension::vtk::geometry::Geometry&>(*geom);
        if (auto data = vtkGeom.data(comp))
        {
          auto listOfSurfaces = ExtractFaceset(data);
          auto appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
          std::for_each(
            listOfSurfaces.begin(),
            listOfSurfaces.end(),
            [appendFilter](vtkSmartPointer<vtkPolyData> s) { appendFilter->AddInputData(s); });
          appendFilter->Update();
          if (auto* finalOutput = appendFilter->GetOutput())
          {
            // Extract last three chars of the filename as file extension,
            // and convert to lower case for subsequent non-case-sensitive comparisons.
            auto fileExt = filename.substr(filename.size() - 4, 4);
            std::for_each(fileExt.begin(), fileExt.end(), [](char& c) { c = std::tolower(c); });
            if (fileExt == ".stl")
            {
              vtkNew<vtkSTLWriter> stlWriter;
              stlWriter->SetInputData(finalOutput);
              stlWriter->SetFileName(filename.c_str());
              stlWriter->Write();
              errorCode = stlWriter->GetErrorCode();
              fileWritingCalled = true;
            }
            else if (fileExt == ".obj")
            {
              vtkNew<vtkOBJWriter> objWriter;
              objWriter->SetInputData(finalOutput);
              objWriter->SetFileName(filename.c_str());
              objWriter->Write();
              errorCode = objWriter->GetErrorCode();
              fileWritingCalled = true;
            }
            else if (fileExt == ".ply")
            {
              vtkNew<vtkPLYWriter> plyWriter;
              plyWriter->SetInputData(finalOutput);
              plyWriter->SetFileName(filename.c_str());
              plyWriter->Write();
              errorCode = plyWriter->GetErrorCode();
              fileWritingCalled = true;
            }
            else
            {
              smtkErrorMacro(this->log(), "Unsupported output file extension: " << fileExt);
            }
          }
        }
      }
    }
  }

  if (!fileWritingCalled)
  {
    smtkErrorMacro(this->log(), "No valid geometry found.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  if (errorCode != vtkErrorCode::NoError)
  {
    std::string errorString(vtkErrorCode::GetStringFromErrorCode(errorCode));
    smtkErrorMacro(this->log(), "VTK failed to write output file. Error: " << errorString);
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  return this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
}

const char* ExportFaceset::xmlDescription() const
{
  return ExportFaceset_xml;
}
} // namespace geometry
} // namespace smtk
