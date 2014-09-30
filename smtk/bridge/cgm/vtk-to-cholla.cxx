//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkIdTypeArray.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkXMLPolyDataReader.h"

#include "smtk/options.h" // for CGM_HAVE_VERSION_H

#ifndef _MSC_VER
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored"-Wunused-parameter"
#endif
#ifdef CGM_HAVE_VERSION_H
#  include "cgm_version.h"
#endif
#include "AppUtil.hpp"
#include "CGMApp.hpp"
#include "GeometryQueryTool.hpp"
#include "FacetModifyEngine.hpp"
#include "FacetQueryEngine.hpp"
#include "CubitFacetData.hpp"
#include "CubitPointData.hpp"
#include "CubitQuadFacetData.hpp"
#include "DLIList.hpp"
#include "Surface.hpp"
#include "ShellSM.hpp"
#include "Lump.hpp"
#include "Body.hpp"
#include "BodySM.hpp"
#include "RefFace.hpp"
#include "RefVertex.hpp"
#ifndef _MSC_VER
#  pragma GCC diagnostic pop
#endif

#include <iostream>

bool generateFacetModel(
  vtkPolyData* pd,
  const std::string& nameOfModelFaceArray,
  const std::string& nameOfFacetToModelFaceMapArray,
  DLIList<TopologyBridge*>& model)
{
  FacetModifyEngine* fme = FacetModifyEngine::instance();

  CubitStatus stat;
  vtkIdType i;
  vtkPoints* pts = pd->GetPoints();
  vtkIdType numPolys = pd->GetNumberOfPolys();
  vtkIdType polyOffset = pd->GetNumberOfVerts() + pd->GetNumberOfLines();

  // I. Populate the map from model face IDs to 0-based counting integers
  std::map<vtkIdType,vtkIdType> modelFaceMap; // map model face IDs to sequential integers
  vtkIdType numModelFaces = 0;
  // First, we may have an explicit list of model face IDs.
  // For sanity, we want to preserve this ordering in the lists below.
  if (!nameOfModelFaceArray.empty())
    {
    vtkDataArray* mfid = pd->GetFieldData()->GetArray(nameOfModelFaceArray.c_str());
    if (mfid && mfid->GetNumberOfComponents() == 1)
      {
      double tupval;
      for (i = 0; i < mfid->GetNumberOfTuples(); ++i)
        {
        mfid->GetTuple(i, &tupval);
        if (
          modelFaceMap.insert(
            std::pair<vtkIdType,vtkIdType>(
              static_cast<vtkIdType>(tupval),
              numModelFaces)).second)
          ++numModelFaces;
        }
      }
    }
  // Now, we may have per-poly model-face IDs. If we had the explicit list
  // of model faces above, this *must* also be present.
  // Even if we did have the explicit list above, we'll run through the
  // relevant portion of this array to make sure there were no missing
  // entries in that list.
  bool forgetPrevious = true;
  vtkDataArray* mapArray = NULL;
  if (!nameOfFacetToModelFaceMapArray.empty())
    {
    vtkDataArray* mfid = pd->GetCellData()->GetArray(nameOfFacetToModelFaceMapArray.c_str());
    if (
      mfid &&
      (mfid->GetNumberOfComponents() == 1) &&
      (mfid->GetNumberOfTuples() >= polyOffset + numPolys))
      {
      mapArray = mfid;
      forgetPrevious = false;
      double tupval;
      for (i = 0; i < numPolys; ++i)
        {
        mfid->GetTuple(i + polyOffset, &tupval);
        vtkIdType fid = static_cast<vtkIdType>(tupval);
        if (modelFaceMap.find(fid) == modelFaceMap.end())
          {
          modelFaceMap[fid] = numModelFaces++;
          }
        }
      }
    }
  if (forgetPrevious)
    {
    modelFaceMap.clear();
    numModelFaces = 0;
    }
  // Finally, if all of the above fails, assume that each
  // tri/quad is its own model-face. In this case, the
  // modelFaceMap is just tuples of (i,i), for i=0...(numPolys-1).
  if (modelFaceMap.empty())
    {
    numModelFaces = numPolys;
    for (i = 0; i < numModelFaces; ++i)
      modelFaceMap[i] = i;
    }

  // II. Generate arrays of arrays of triangle and quadrilateral facets
  //     Each array of (tri/quad) facets contains those facets assigned
  //     a single model face. We will then call build_facet_surface once
  //     for each model face below.
  std::vector<DLIList<CubitFacet*> > tflists;
  std::vector<DLIList<CubitQuadFacet*> > qflists;
  std::vector<DLIList<CubitPoint*> > ptlists;
  std::vector<std::map<vtkIdType,vtkIdType> > ptmaps;
  tflists.resize(numModelFaces);
  qflists.resize(numModelFaces);
  ptlists.resize(numModelFaces);
  ptmaps.resize(numModelFaces);
  vtkIdType npts;
  vtkIdType* conn;
  vtkIdType offset = pd->GetNumberOfVerts() + pd->GetNumberOfLines();
  vtkCellArray* polys = pd->GetPolys();
  double x[3];
  int j;
  for (i = 0, polys->InitTraversal(); polys->GetNextCell(npts, conn); ++i)
    {
    vtkIdType curModelFace = mapArray ?
      modelFaceMap[mapArray->GetTuple1(i + polyOffset)] :
      i;
    int n4 = npts > 4 ? 4 : npts; // we only support triangles and quads.
    for (j = 0; j < n4; ++j)
      {
      if (ptmaps[curModelFace].find(conn[j]) == ptmaps[curModelFace].end())
        {
        pts->GetPoint(conn[j], x);
        ptmaps[curModelFace][conn[j]] = ptlists[curModelFace].size();
        ptlists[curModelFace].append(new CubitPointData(x[0], x[1], x[2]));
        }
      }
    switch (npts)
      {
    case 3:
      tflists[curModelFace].append(
        new CubitFacetData(
          ptlists[curModelFace][ptmaps[curModelFace][conn[0]]],
          ptlists[curModelFace][ptmaps[curModelFace][conn[1]]],
          ptlists[curModelFace][ptmaps[curModelFace][conn[2]]]));
      break;
    case 4:
      qflists[curModelFace].append(
        new CubitQuadFacetData(
          ptlists[curModelFace][ptmaps[curModelFace][conn[0]]], ptlists[curModelFace][ptmaps[curModelFace][conn[1]]],
          ptlists[curModelFace][ptmaps[curModelFace][conn[1]]], ptlists[curModelFace][ptmaps[curModelFace][conn[3]]]));
      break;
    default:
      std::cerr << "Cell " << (i + offset) << " not a triangle or quad (" << npts << ").\n";
      break;
      }
    }
  DLIList<Surface*> modelslist;
  for (i = 0; i < numModelFaces; ++i)
    {
    if (tflists[i].size() <= 0 && qflists[i].size() <= 0)
      { // Skip empty model-faces. Should we print a warning here?
      continue;
      }
    DLIList<Surface*> slist;
    stat = fme->build_facet_surface(
      qflists[i], tflists[i], ptlists[i], -1., 0, false, false, slist);
    if (slist.size() <= 0 || stat != CUBIT_SUCCESS) return false; // FIXME: Clean memory
    modelslist += slist;
    }

  ShellSM* shell;
  stat = fme->make_facet_shell(modelslist, shell);
  if (!shell || stat != CUBIT_SUCCESS) return false; // FIXME: Clean memory

  DLIList<ShellSM*> shlist;
  shlist.append(shell);
  Lump* lump;
  stat = fme->make_facet_lump(shlist, lump);
  if (!lump || stat != CUBIT_SUCCESS) return false; // FIXME: Clean memory

  DLIList<Lump*> llist;
  BodySM* bodySM;
  Body* body;
  llist.append(lump);
  stat = fme->make_facet_body(llist, bodySM);
  body = GeometryQueryTool::instance()->make_Body(bodySM);
  if (!body || stat != CUBIT_SUCCESS) return false; // FIXME: Clean memory

  model.append(bodySM);
  return true;
}

int main(int argc, char* argv[])
{
  if (argc < 3)
    {
    std::cout
      << "Usage:\n"
      << "  " << argv[0] << " input_filename output_filename [modelfacemap [modelfaceids]]\n"
      << "where\n"
      << "  input_filename is the path to a VTK XML polydata file\n"
      << "  output_filename is the path to a CGM facet model\n"
      << "  modelfacemap    is the name of a cell-array specifying model face IDs per poly\n"
      << "  modelfaceids    is the name of a field-data array listing model face IDs\n"
      ;
      return 1;
    }

  // Initialize CGM
#if CGM_MAJOR_VERSION >= 14
  std::vector<CubitString> args(argv + 1, argv + argc);
  AppUtil::instance()->startup(args);
  CGMApp::instance()->startup(args);
#else
  AppUtil::instance()->startup(argc, argv);
  CGMApp::instance()->startup(argc, argv);
#endif
  FacetQueryEngine* fqe = FacetQueryEngine::instance();

  // Read the VTK dataset
  vtkNew<vtkXMLPolyDataReader> rdr;
  rdr->SetFileName(argv[1]);
  rdr->Update();
  vtkPolyData* pd = rdr->GetOutput();

  // Convert every polygon into a CGM facet.
  vtkCellArray* cd = pd->GetPolys();
  if (cd)
    {
    std::cout << "Read \"" << argv[1] << "\" with " << cd->GetNumberOfCells() << " polys\n";
    DLIList<TopologyBridge*> model;
    if (generateFacetModel(
        pd,
        argc > 4 ? argv[4] : "ModelFaceIds",
        argc > 2 ? argv[3] : "modelfaceids",
        model))
      {
      ModelExportOptions opts;
      fqe->export_solid_model(
        model, argv[2], FACET_TYPE,
        CubitString(), opts);
      std::cout << "Wrote \"" << argv[2] << "\"\n";
      }
    }

  return 0;
}
