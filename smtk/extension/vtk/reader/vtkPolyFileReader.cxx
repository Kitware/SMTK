//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkPolyFileReader.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkNew.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkSmartPointer.h"

#include "vtksys/SystemTools.hxx"

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <limits>

#include "smtk/extension/vtk/reader/vtkPolyFileTokenConverters.h"
#include "smtk/extension/vtk/reader/vtkPolyFileErrorReporter.h"

vtkStandardNewMacro(vtkPolyFileReader);

template<typename Converter, typename ErrorReporter>
typename Converter::type readValue(std::istream& in, bool& ok, ErrorReporter& err);

class vtkPolyFileReader::Private
{
public:
  std::map<int,int> PointIds;
};

class PolyFileModelBuilder
{
public:
  int Dimension;
  int NumberOfPointAttributes;
  int HavePointBoundaryMarks;
  vtkIdType NextPointId;
  vtkSmartPointer<vtkPolyData> Output;

  PolyFileModelBuilder()
    {
    this->NextPointId = 0;
    }

};

class vtkPolyFileReader::Builder : public PolyFileModelBuilder
{
public:

  Builder()
    {
    this->CurrentFacetOnBdy = 0;
    this->FacetMarksAsCellData = 0;
    }

  void NumberOfNodes(vtkIdType nn)
    {
    this->Points->SetNumberOfPoints(nn);
    }

  /// Must be called after NumberOfNodes()
  void NodeMetadata(int dimension, int numAttribs, int bdyMarkers)
    {
    this->Dimension = dimension;
    this->NumberOfPointAttributes = numAttribs;
    this->HavePointBoundaryMarks = bdyMarkers;

    // Create attribute arrays
    vtkIdType numPts = this->Points->GetNumberOfPoints();
    int numPtData = numAttribs + (bdyMarkers ? 1 : 0) + 1;
    this->PointArrays.resize(numPtData);
    for (int i = 0; i < numAttribs; ++i)
      {
      std::ostringstream arrayName;
      arrayName << "Attribute" << i;
      this->PointArrays[i] = vtkSmartPointer<vtkDoubleArray>::New();
      this->PointArrays[i]->SetName(arrayName.str().c_str());
      this->PointArrays[i]->SetNumberOfTuples(numPts);
      this->PointData->AddArray(this->PointArrays[i].GetPointer());
      }
    if (bdyMarkers)
      {
      this->PointArrays[numAttribs] = vtkSmartPointer<vtkDoubleArray>::New();
      this->PointArrays[numAttribs]->SetName(VTK_POLYFILE_NODE_GROUP);
      this->PointArrays[numAttribs]->SetNumberOfTuples(numPts);
      this->PointData->AddArray(this->PointArrays[numAttribs].GetPointer());
      }
    this->GlobalIds->SetName(VTK_POLYFILE_GLOBAL_NODE_ID);
    this->GlobalIds->SetNumberOfTuples(numPts);
    this->PointData->AddArray(this->GlobalIds.GetPointer());
    this->PointData->SetGlobalIds(this->GlobalIds.GetPointer());

    this->Poly2Facet->SetName(VTK_POLYFILE_MODEL_FACE_ID);
    this->CellData->SetPedigreeIds(this->Poly2Facet.GetPointer());
    }

  vtkIdType AddVertex(vtkIdType vid, double x[3], const std::vector<double>& attribs)
    {
    this->Points->SetPoint(this->NextPointId, x);
    this->GlobalIds->SetValue(this->NextPointId, vid);
    for (int a = 0; a < this->NumberOfPointAttributes; ++a)
      {
      this->PointArrays[a]->SetValue( this->NextPointId, attribs[a]);
      }
    if (this->HavePointBoundaryMarks)
      {
      this->PointArrays[this->NumberOfPointAttributes]->SetValue(
        this->NextPointId, attribs[this->NumberOfPointAttributes]);
      }
    return this->NextPointId++;
    }

  void FacetMetadata(vtkIdType /*numFacets*/, int /*bdyMarkers*/, int bdysAsCellData)
    {
    this->FacetBoundaryMarkers->SetName(VTK_POLYFILE_FACE_GROUP);
    this->FacetMarksAsCellData = bdysAsCellData;
    if (FacetMarksAsCellData)
      {
      this->CellData->AddArray(this->FacetBoundaryMarkers.GetPointer());
      }

    this->FacetHoleFacetIds->SetName(VTK_POLYFILE_MODEL_FACE_ID);
    }

  void StartFacet(vtkIdType facetIdx, vtkIdType /*numPolys*/, vtkIdType /*numHoles*/, int onBdy)
    {
    this->CurrentFacet = facetIdx;
    this->CurrentFacetOnBdy = onBdy;
    if (!FacetMarksAsCellData)
      {
      this->FacetBoundaryMarkers->InsertNextValue(this->CurrentFacetOnBdy);
      }
    }

  void AddPoly(vtkIdType /*polyIdx*/, const std::vector<vtkIdType>& loop)
    {
    this->Polylines->InsertNextCell(loop.size(), &loop[0]);
    this->Poly2Facet->InsertNextValue(this->CurrentFacet);
    if (FacetMarksAsCellData)
      {
      this->FacetBoundaryMarkers->InsertNextValue(this->CurrentFacetOnBdy);
      }
    }

  vtkIdType FacetHolePoint(vtkIdType /*holeIdx*/, double x[3])
    {
    vtkIdType hidx = this->FacetHolePoints->InsertNextPoint(x);
    this->FacetHoleFacetIds->InsertNextValue(this->CurrentFacet);
    return hidx;
    }

  void FinalizeFacet(vtkIdType /*facetIdx*/)
    {
    }

  void VolumeHoleMetadata(vtkIdType numVolumeHoles)
    {
    this->VolumeHoleIds->SetName(VTK_POLYFILE_GLOBAL_HOLE_ID);
    this->VolumeHoleIds->SetNumberOfTuples(numVolumeHoles);
    this->VolumeHolePoints->Allocate(numVolumeHoles);
    }

  vtkIdType VolumeHolePoint(vtkIdType holeIdx, double x[3])
    {
    vtkIdType hidx = this->VolumeHolePoints->InsertNextPoint(x);
    this->VolumeHoleIds->SetValue(hidx, holeIdx);
    return hidx;
    }

  void RegionMetadata(vtkIdType numRegions)
    {
    this->RegionPoints->Allocate(numRegions);
    this->RegionIds->SetName(VTK_POLYFILE_REGION_GROUP_NUMBER);
    this->RegionIds->SetNumberOfTuples(numRegions);
    this->RegionGroups->SetName(VTK_POLYFILE_REGION_GROUP);
    this->RegionGroups->SetNumberOfTuples(numRegions);
    this->RegionAttrib->SetName(VTK_POLYFILE_REGION_GROUP_ATTRIBUTES);
    this->RegionAttrib->SetNumberOfTuples(numRegions);
    }

  vtkIdType AddRegion(vtkIdType rid, double x[3], double rnum, double attrib)
    {
    vtkIdType pidx = this->RegionPoints->InsertNextPoint(x);
    this->RegionGroups->SetValue(pidx, rnum);
    this->RegionAttrib->SetValue(pidx, attrib);
    this->RegionIds->SetValue(pidx, rid);
    return pidx;
    }

  void Finalize(
    vtkPolyData* polyOutput,
    vtkPolyData* facetHoleOutput,
    vtkPolyData* volumeHoleOutput,
    vtkPolyData* regionOutput)
    {
    polyOutput->SetPoints(this->Points.GetPointer());
    polyOutput->GetPointData()->ShallowCopy(this->PointData.GetPointer());
    polyOutput->GetCellData()->ShallowCopy(this->CellData.GetPointer());
    polyOutput->SetLines(this->Polylines.GetPointer());
    if (!this->FacetMarksAsCellData)
      {
      polyOutput->GetFieldData()->AddArray(
        this->FacetBoundaryMarkers.GetPointer());
      }
    vtkNew<vtkIntArray> polyDimension;
    polyDimension->SetName("Dimension");
    polyDimension->InsertNextValue(this->Dimension);
    polyOutput->GetFieldData()->AddArray(polyDimension.GetPointer());

    facetHoleOutput->SetPoints(this->FacetHolePoints.GetPointer());
    facetHoleOutput->GetPointData()->SetPedigreeIds(this->FacetHoleFacetIds.GetPointer());

    volumeHoleOutput->SetPoints(this->VolumeHolePoints.GetPointer());
    volumeHoleOutput->GetPointData()->SetGlobalIds(this->VolumeHoleIds.GetPointer());

    regionOutput->SetPoints(this->RegionPoints.GetPointer());
    regionOutput->GetPointData()->SetScalars(this->RegionAttrib.GetPointer());
    regionOutput->GetPointData()->AddArray(this->RegionGroups.GetPointer());
    regionOutput->GetPointData()->AddArray(this->RegionAttrib.GetPointer());
    regionOutput->GetPointData()->AddArray(this->RegionIds.GetPointer());
    }

  vtkIdType CurrentFacet;
  int CurrentFacetOnBdy;
  int FacetMarksAsCellData;
  vtkNew<vtkPoints> Points;
  vtkNew<vtkPointData> PointData;
  vtkNew<vtkCellData> CellData;
  vtkNew<vtkIdTypeArray> GlobalIds;
  vtkNew<vtkIdTypeArray> Poly2Facet;
  vtkNew<vtkCellArray> Polylines;
  vtkNew<vtkPoints> FacetHolePoints;
  vtkNew<vtkIdTypeArray> FacetHoleFacetIds;
  vtkNew<vtkIdTypeArray> PointBoundaryMarkers;
  vtkNew<vtkIdTypeArray> FacetBoundaryMarkers;
  std::vector<vtkSmartPointer<vtkDoubleArray> > PointArrays;
  vtkNew<vtkPoints> VolumeHolePoints;
  vtkNew<vtkIdTypeArray> VolumeHoleIds;
  vtkNew<vtkPoints> RegionPoints;
  vtkNew<vtkIdTypeArray> RegionIds;
  vtkNew<vtkDoubleArray> RegionGroups;
  vtkNew<vtkDoubleArray> RegionAttrib;
};

template<typename Converter, typename ErrorReporter>
typename Converter::type readValue(std::istream& in, bool& ok, ErrorReporter& err)
{
  std::string token;
  while (in.good() && !in.eof())
    {
    in >> token;
    if (!token.empty() && token[0] == '#')
      {
      in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      }
    else if (token.empty())
      {
      ok = false;
      err.Report(in, -1, -1, VTK_POLYFILE_EOF);
      return Converter::bad_value();
      }
    else
      {
      // Sometimes a token ends with the beginning of a comment:
      if (token.find('#', 1) != std::string::npos)
        {
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
      typename Converter::type result = Converter::convert(token, ok);
      if (!ok)
        {
        int errEnd = in.tellg();
        int errBeg = errEnd - token.size();
        if (err.Report(in, errBeg, errEnd, VTK_POLYFILE_BAD_TOKEN))
          {
          return result;
          }
        }
      return result;
      }
    }
  ok = false;
  err.Report(in, in.eof() ? VTK_POLYFILE_EOF : VTK_POLYFILE_OUT_OF_DATA);
  return Converter::bad_value();
}

template<typename ErrorReporter>
bool vtkPolyFileReader::ReadNodes(
  std::istream& in, vtkIdType numPts, const std::string& nodeSpec,
  int& dimension, int& numAttribs,
  ErrorReporter& err)
{
  bool ok;
  dimension = 0;
  numAttribs = 0;
  int bdyMarkers = 0;

  int nv = sscanf(nodeSpec.c_str(), "%d %d %d", &dimension, &numAttribs, &bdyMarkers);
  if (nv < 1)
    {
    err.Report(in, VTK_POLYFILE_MISSING_DIM);
    dimension = 3;
    }
  if (dimension < 2 || dimension > 3)
    {
    err.Report(in, VTK_POLYFILE_BAD_DIM);
    return false;
    }

  this->B->NodeMetadata(dimension, numAttribs, bdyMarkers);
  /*
  cout
    << numPts << " " << dimension << "-D points, "
    << numAttribs << " attributes, "
    << (bdyMarkers ? "boundary markers" : "no boundary markers") << "\n";
    */

  double x[3] = { 0., 0., 0. };
  std::vector<double> attribs;
  attribs.resize(numAttribs + bdyMarkers ? 1 : 0);
  for (vtkIdType i = 0; i < numPts; ++i)
    {
    int ptid = readValue<Int32Converter>(in, ok, err);
    for (int c = 0; c < dimension; ++ c)
      {
      x[c] = readValue<DoubleConverter>(in, ok, err);
      }
    //cout << "pt " << (int)this->P->PointIds[ptid] << " id " << (int)ptid << "\n";
    for (int a = 0; a < numAttribs; ++a)
      {
      attribs[a] = readValue<DoubleConverter>(in, ok, err);
      }
    if (bdyMarkers)
      {
      attribs[numAttribs] = readValue<DoubleConverter>(in, ok, err);
      }
    this->P->PointIds[ptid] = this->B->AddVertex(ptid, x, attribs);
    //cout << ptid << ": " << x[0] << ", " << x[1] << ", " << x[2] << "\n";

    // Some evil .node/.poly files say they don't have attributes but do.
    // Ignore text from the end of what we expect after point coordinates
    // until the end of the line.
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Update progress
    if (i % 1000 == 0)
      {
      this->UpdateProgress(i * 0.45 / numPts);
      }
    }

  return true;
}

template<typename ErrorReporter>
bool vtkPolyFileReader::ReadSegments(
  std::istream& in,
  int& bdyMarkers, ErrorReporter& err)
{
  // Lines are special in this section as some numbers are mandatory de jure but optional de facto.
  bool ok;
  int numSegments = readValue<Int32Converter>(in, ok, err);
  std::string restOfSegmentSpec;
  std::getline(in, restOfSegmentSpec);
  int nv = sscanf(restOfSegmentSpec.c_str(), "%d", &bdyMarkers);
  (void)nv; // keep nv around for debugging

  this->B->FacetMetadata(1, bdyMarkers, this->FacetMarksAsCellData);
  //std::cout << numSegments << " segments, " << (bdyMarkers ? "boundary markers" : "no boundary markers") << "\n";
  int numBdyWarnings = 0;

  this->B->StartFacet(0, /* don't know how many polys yet*/ -1, 0, /* no bdy info yet*/ 0);
  std::vector<vtkIdType> loop;
  vtkIdType polyIndex = 0;
  for (int i = 0; i < numSegments; ++ i)
    {
    int segmentId = readValue<Int32Converter>(in, ok, err);
    (void)segmentId;
    vtkIdType endpt0 = readValue<Int32Converter>(in, ok, err);
    if (!ok)
      {
      return false;
      }
    vtkIdType endpt1 = readValue<Int32Converter>(in, ok, err);
    if (!ok)
      {
      return false;
      }
    endpt0 = this->P->PointIds[endpt0];
    endpt1 = this->P->PointIds[endpt1];
    int onBdy = 0;
    std::getline(in, restOfSegmentSpec);
    nv = 0;
    if (!restOfSegmentSpec.empty())
      {
      nv = sscanf(restOfSegmentSpec.c_str(), "%d", &onBdy);
      }
    if (bdyMarkers && nv < 1)
      {
      if (++numBdyWarnings < 10)
        {
        err.Report(in, static_cast<int>(in.tellg())-1, in.tellg(),
          VTK_POLYFILE_MISSING_SEGMENT_BDY);
        }
      else if (numBdyWarnings < 11)
        {
        err.Report(in, VTK_POLYFILE_TOO_MANY_SEGMENT_ERRS);
        }
      }
    if (!loop.empty())
      {
      if (endpt1 == loop.front() && endpt0 == loop.back())
        {
        loop.push_back(endpt1);
        this->B->AddPoly(polyIndex++, loop);
        loop.clear();
        }
      else if (loop.back() != endpt0)
        {
        this->B->AddPoly(polyIndex++, loop);
        loop.clear();
        loop.push_back(endpt0);
        loop.push_back(endpt1);
        }
      else
        {
        loop.push_back(endpt1);
        }
      }
    else
      {
      loop.push_back(endpt0);
      loop.push_back(endpt1);
      }

    // Update progress
    if (i % 1000 == 0)
      {
      this->UpdateProgress(0.45 + i * 0.45 / numSegments);
      }
    }
  // Construct the face in the model
  this->B->FinalizeFacet(0);
  return true;
}

template<typename ErrorReporter>
bool vtkPolyFileReader::ReadSimpleFacets(
  std::istream& in, int /*dimension*/,
  int& bdyMarkers, ErrorReporter& err)
{
  // Lines are special in this section as some numbers are mandatory de jure but optional de facto.
  bool ok;
  int numFacets = readValue<Int32Converter>(in, ok, err);
  std::string restOfFacetSpec;
  std::getline(in, restOfFacetSpec);
  int nv = sscanf(restOfFacetSpec.c_str(), "%d", &bdyMarkers);
  (void)nv; // keep nv around for debugging

  this->B->FacetMetadata(numFacets, bdyMarkers, this->FacetMarksAsCellData);
  //std::cout << numFacets << " facets, " << (bdyMarkers ? "boundary markers" : "no boundary markers") << "\n";
  int numBdyWarnings = 0;

  for (int i = 0; i < numFacets; ++ i)
    {
    //std::cout << numPolys << " polys, " << numHoles << " holes, " << (onBdy ? "boundary" : "interior") << "\n";
    int numCorners = readValue<Int32Converter>(in, ok, err);
    std::vector<vtkIdType> loop;
    if (ok && numCorners)
      {
      for (int c = 0; c < numCorners; ++c)
        {
        int ptId = readValue<Int32Converter>(in, ok, err);
        //std::cout << " " << this->P->PointIds[ptId];
        loop.push_back(this->P->PointIds[ptId]);
        }
      loop.push_back(loop[0]); // close the loop
      int onBdy = 0;
      std::getline(in, restOfFacetSpec);
      nv = 0;
      if (!restOfFacetSpec.empty())
        {
        nv = sscanf(restOfFacetSpec.c_str(), "%d", &onBdy);
        }
      if (bdyMarkers && nv < 1)
        {
        if (++numBdyWarnings < 10)
          {
          err.Report(in, static_cast<int>(in.tellg()), in.tellg(),
            VTK_POLYFILE_MISSING_FACET_BDY);
          }
        else if (numBdyWarnings < 11)
          {
          err.Report(in, VTK_POLYFILE_TOO_MANY_FACET_ERRS);
          }
        }
      this->B->StartFacet(i, 1, 0, onBdy);
      this->B->AddPoly(0, loop);
      this->B->FinalizeFacet(i);
      }
    // Update progress
    if (i % 1000 == 0)
      {
      this->UpdateProgress(0.45 + i * 0.45 / numFacets);
      }
    }

  return true;
}

template<typename ErrorReporter>
bool vtkPolyFileReader::ReadFacets(
  std::istream& in, int dimension,
  int& bdyMarkers, ErrorReporter& err)
{
  // Lines are special in this section as some numbers are mandatory de jure but optional de facto.
  bool ok;
  int numFacets = readValue<Int32Converter>(in, ok, err);
  std::string restOfFacetSpec;
  std::getline(in, restOfFacetSpec);
  int nv = sscanf(restOfFacetSpec.c_str(), "%d", &bdyMarkers);
  (void)nv; // keep nv around for debugging

  this->B->FacetMetadata(numFacets, bdyMarkers, this->FacetMarksAsCellData);
  //std::cout << numFacets << " facets, " << (bdyMarkers ? "boundary markers" : "no boundary markers") << "\n";
  int numBdyWarnings = 0;

  for (int i = 0; i < numFacets; ++ i)
    {
    int numPolys = readValue<Int32Converter>(in, ok, err);
    std::getline(in, restOfFacetSpec);
    int numHoles = 0;
    int onBdy = 0;
    if (!restOfFacetSpec.empty())
      {
      nv = sscanf(restOfFacetSpec.c_str(), "%d %d", &numHoles, &onBdy);
      if (bdyMarkers && nv < 2)
        {
        if (++numBdyWarnings < 10)
          {
          err.Report(in, static_cast<int>(in.tellg()), in.tellg(),
            VTK_POLYFILE_MISSING_FACET_BDY);
          }
        else if (numBdyWarnings < 11)
          {
          err.Report(in, VTK_POLYFILE_TOO_MANY_FACET_ERRS);
          }
        }
      }
    this->B->StartFacet(i, numPolys, numHoles, onBdy);
    //std::cout << numPolys << " polys, " << numHoles << " holes, " << (onBdy ? "boundary" : "interior") << "\n";
    for (int p = 0; p < numPolys; ++p)
      {
      int numCorners = readValue<Int32Converter>(in, ok, err);
      std::vector<vtkIdType> loop;
      if (ok && numCorners)
        {
        for (int c = 0; c < numCorners; ++c)
          {
          int ptId = readValue<Int32Converter>(in, ok, err);
          //std::cout << " " << this->P->PointIds[ptId];
          loop.push_back(this->P->PointIds[ptId]);
          }
        loop.push_back(loop[0]); // close the loop
        this->B->AddPoly(p, loop);
        }
      //std::cout << "\n";
      }
    double x[3] = {0., 0., 0.};
    for (int h = 0; h < numHoles; ++h)
      {
      int holeId = readValue<Int32Converter>(in, ok, err);
      (void)holeId;
      for (int c = 0; c < dimension; ++c)
        {
        x[c] = readValue<DoubleConverter>(in, ok, err);
        }
      this->B->FacetHolePoint(h, x);
      }

    // Construct the face in the model
    this->B->FinalizeFacet(i);

    // Update progress
    if (i % 1000 == 0)
      {
      this->UpdateProgress(0.45 + i * 0.45 / numFacets);
      }
    }
  return true;
}

template<typename ErrorReporter>
bool vtkPolyFileReader::ReadHoles(std::istream& in, int dimension, ErrorReporter& err)
{
  bool ok;
  int numHoles = readValue<Int32Converter>(in, ok, err);
  double x[3] = { 0., 0., 0. };
  this->B->VolumeHoleMetadata(numHoles);

  for (int i = 0; i < numHoles; ++i)
    {
    int holeId = readValue<Int32Converter>(in, ok, err);
    for (int c = 0; c < dimension; ++c)
      {
      x[c] = readValue<DoubleConverter>(in, ok, err);
      }
    this->B->VolumeHolePoint(holeId, x);

    // Update progress
    if (i % 1000 == 0)
      {
      this->UpdateProgress(0.9 + i * 0.05 / numHoles);
      }
    }
  return true;
}

template<typename ErrorReporter>
bool vtkPolyFileReader::ReadRegionAttributes(std::istream& in, int dimension, ErrorReporter& err)
{
  bool ok;
  int numRegions = readValue<Int32Converter>(in, ok, err);
  if (! ok)
    {
    err.Report(in, VTK_POLYFILE_BAD_REGION_COUNT);
    return false;
    }
  this->B->RegionMetadata(numRegions);

  std::string restOfRegionSpec;
  double x[3] = { 0., 0., 0. };
  for (int i = 0; i < numRegions; ++i)
    {
    int holeId = readValue<Int32Converter>(in, ok, err);
    for (int c = 0; c < dimension; ++c)
      {
      x[c] = readValue<DoubleConverter>(in, ok, err);
      }
    std::getline(in, restOfRegionSpec);
    double regionAttrib = 0;
    double regionVolumeConstraint = 0;
    if (!restOfRegionSpec.empty())
      {
      int nv = sscanf(restOfRegionSpec.c_str(), "%lg %lg", &regionAttrib, &regionVolumeConstraint);
      if (nv == 1)
        { // File format spec says that if 1 number is specified, use for both:
        regionVolumeConstraint = regionAttrib;
        }
      }
    this->B->AddRegion(holeId, x, regionAttrib, regionVolumeConstraint);
    //std::cout << numPolys << " polys, " << numHoles << " holes, " << (onBdy ? "boundary" : "interior") << "\n";

    // Update progress
    if (i % 1000 == 0)
      {
      this->UpdateProgress(0.95 + i * 0.05 / numRegions);
      }
    }
  return true;
}

template<typename ErrorReporter>
void vtkPolyFileReader::ReadFile(
  std::istream& in, int isSimpleMesh,
  ErrorReporter& err, const std::string& nodeFileName)
{
  std::vector<double> values;
  bool ok = true;
  double v;

  int numPts = readValue<Int32Converter>(in, ok, err);
  std::string restOfNodeLine;
  // The line containing the number of nodes is special because
  // many mal-formed files exist which do not specify the number of
  // attributes or boundary markers. Some even assume the dimension is 3.
  std::getline(in, restOfNodeLine);
  int dimension;
  int numAttribs;
  int bdyMarkers;
  if (numPts == 0)
    {
    std::ifstream nodeFile(nodeFileName.c_str(), std::ios::in);
    if (! nodeFile.good() || nodeFile.eof())
      {
      err.Report(nodeFile, 0, 0, VTK_POLYFILE_BAD_EXTERNAL_NODEFILE);
      }
    vtkPolyFileErrorReporter nodeErr(nodeFileName);
    numPts = readValue<Int32Converter>(nodeFile, ok, nodeErr);
    if (!ok || numPts <= 0)
      {
      int npos = nodeFile.tellg();
      nodeErr.Report(nodeFile, 0, npos, VTK_POLYFILE_BAD_NODEFILE_POINTS);
      return;
      }
    this->B->NumberOfNodes(numPts);
    std::getline(nodeFile, restOfNodeLine);
    this->ReadNodes(nodeFile, numPts, restOfNodeLine, dimension, numAttribs, nodeErr);
    }
  else
    {
    this->B->NumberOfNodes(numPts);
    this->ReadNodes(in, numPts, restOfNodeLine, dimension, numAttribs, err);
    }

  // <point #> <x> <y> <z>[attributes] [boundary marker]

  if (dimension == 2)
    {
    // Read in the segments and make them part of a single facet
    // embedded in the z=0 plane.
    if (!this->ReadSegments(in, bdyMarkers, err))
      {
      return;
      }
    }
  else
    {
    if (isSimpleMesh)
      {
      if (!this->ReadSimpleFacets(in, dimension, bdyMarkers, err))
        {
        return;
        }
      }
    else
      {
      if (!this->ReadFacets(in, dimension, bdyMarkers, err))
        {
        return;
        }
      }
    }

  if (!this->ReadHoles(in, dimension, err))
    {
    return;
    }

  if (!this->ReadRegionAttributes(in, dimension, err))
    {
    return;
    }

  while (err.EndOfFile <= 0)
    {
    v = readValue<DoubleConverter>(in, ok, err);
    if (ok)
      {
      values.push_back(v);
      }
    }

  this->UpdateProgress(1.0);
}

vtkPolyFileReader::vtkPolyFileReader()
{
  this->P = new Private;
  this->B = new Builder;
  this->FileName = NULL;
  this->NodeFileName = NULL;
  this->SimpleMeshFormat = 0;
  this->FacetMarksAsCellData = 0;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(4); // polys + facet holes + volume holes + regions
}

vtkPolyFileReader::~vtkPolyFileReader()
{
  this->SetFileName(NULL);
  this->SetNodeFileName(NULL);
  delete this->P;
  delete this->B;
}

void vtkPolyFileReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os
    << indent << "FileName: "
    << (this->FileName == NULL ? "(NULL)" : this->FileName) << "\n";
  os
    << indent << "NodeFileName: "
    << (this->NodeFileName == NULL ? "(NULL)" : this->NodeFileName) << "\n";
  os << indent << "SimpleMeshFormat: " << this->SimpleMeshFormat << "\n";
  os
    << indent << "FacetMarksAsCellData: "
    << (this->FacetMarksAsCellData ? "ON" : "OFF") << "\n";
}

int vtkPolyFileReader::RequestData(
  vtkInformation* /*request*/,
  vtkInformationVector** /*inVec*/,
  vtkInformationVector* outVec)
{
  if (!this->FileName || this->FileName[0] == '\0')
    {
    vtkErrorMacro("Empty filename");
    return 0;
    }

  vtkInformation* polyOutInfo = outVec->GetInformationObject(0);
  vtkPolyData* polyOutput = vtkPolyData::SafeDownCast(
    polyOutInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkInformation* facetHoleOutInfo = outVec->GetInformationObject(1);
  vtkPolyData* facetHoleOutput = vtkPolyData::SafeDownCast(
    facetHoleOutInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkInformation* volumeHoleOutInfo = outVec->GetInformationObject(2);
  vtkPolyData* volumeHoleOutput = vtkPolyData::SafeDownCast(
    volumeHoleOutInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkInformation* regionOutInfo = outVec->GetInformationObject(3);
  vtkPolyData* regionOutput = vtkPolyData::SafeDownCast(
    regionOutInfo->Get(vtkDataObject::DATA_OBJECT()));

  std::string polyFileName(this->FileName);
  std::string nodeFileName = vtksys::SystemTools::GetFilenameWithoutExtension(this->FileName);
  nodeFileName = vtksys::SystemTools::GetParentDirectory(this->FileName) + "/" + nodeFileName + ".node";
  //cout << "Nodes in " << nodeFileName << "???\n";
  vtkPolyFileErrorReporter err(polyFileName);
  std::ifstream polyfile(polyFileName.c_str(), std::ios::in);

  int isSimpleMesh = this->SimpleMeshFormat;
  if (isSimpleMesh == -1)
    {
    std::string extension = vtksys::SystemTools::GetFilenameLastExtension(this->FileName);
    if (extension == ".smesh")
      {
      isSimpleMesh = 1;
      }
    else
      {
      isSimpleMesh = 0;
      }
    }

  if (polyfile.good())
    {
    this->ReadFile(polyfile, isSimpleMesh, err, nodeFileName);
    this->B->Finalize(polyOutput, facetHoleOutput, volumeHoleOutput, regionOutput);
    }
  else
    {
    err.Report(polyfile, -1, -1, VTK_POLYFILE_CANNOT_OPEN);
    return 0;
    }

  return 1;
}
