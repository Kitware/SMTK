//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#include "smtk/extension/vtk/io/ReadVTKData.h"

#include "smtk/common/Paths.h"

#include "smtk/extension/vtk/filter/vtkImageSpacingFlip.h"
#include "smtk/extension/vtk/reader/vtkCMBGeometryReader.h"
#include "smtk/extension/vtk/reader/vtkLASReader.h"

#include "vtkAppendPoints.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataObject.h"
#include "vtkDataObjectTreeIterator.h"
#include "vtkDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkGDALRasterReader.h"
#include "vtkGraph.h"
#include "vtkIdTypeArray.h"
#include "vtkImageData.h"
#include "vtkImageMapToColors.h"
#include "vtkInformation.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationVector.h"
#include "vtkLookupTable.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkOBJReader.h"
#include "vtkObjectFactory.h"
#include "vtkPLYReader.h"
#include "vtkPNGReader.h"
#include "vtkPTSReader.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"
#include "vtkProperty.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkXMLImageDataReader.h"
#include "vtkXMLMultiBlockDataReader.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLUnstructuredGridReader.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace io
{

ReadVTKData::~ReadVTKData()
{
}

bool ReadVTKData::valid(const std::string& file) const
{
  std::string fileType = smtk::common::Paths::extension(file);
  // If the file type isn't the empty string, remove the leading ".".
  if (fileType.begin() != fileType.end() && fileType[0] == '.')
  {
    fileType.erase(fileType.begin());
  }

  return this->valid(std::make_pair(fileType, file));
}

vtkSmartPointer<vtkDataObject> ReadVTKData::operator()(const std::string& file)
{
  std::string fileType = smtk::common::Paths::extension(file);
  // If the file type isn't the empty string, remove the leading ".".
  if (fileType.begin() != fileType.end() && fileType[0] == '.')
  {
    fileType.erase(fileType.begin());
  }

  return this->operator()(std::make_pair(fileType, file));
}
}
}
}
}

namespace
{
/// Reader types for each VTK file type that SMTK supports.
#define DeclareReader_type(FTYPE)                                                                  \
  class ReadVTKData_##FTYPE                                                                        \
    : public smtk::common::GeneratorType<std::pair<std::string, std::string>,                      \
        vtkSmartPointer<vtkDataObject>, ReadVTKData_##FTYPE>                                       \
  {                                                                                                \
  public:                                                                                          \
    bool valid(const std::pair<std::string, std::string>& fileInfo) const override;                \
                                                                                                   \
    vtkSmartPointer<vtkDataObject> operator()(                                                     \
      const std::pair<std::string, std::string>& fileInfo) override;                               \
  };                                                                                               \
  static bool registered_##FTYPE = ReadVTKData_##FTYPE::registerClass()

DeclareReader_type(vtp);
DeclareReader_type(vtu);
DeclareReader_type(vti);
DeclareReader_type(vtm);
DeclareReader_type(obj);
DeclareReader_type(ply);
DeclareReader_type(pts);
DeclareReader_type(tif);
DeclareReader_type(png);
DeclareReader_type(cmb);
DeclareReader_type(las);

#undef DeclareReader_type

#define BasicReader_type(FTYPE, CLASS, READER)                                                     \
  bool ReadVTKData_##FTYPE::valid(const std::pair<std::string, std::string>& fileInfo) const       \
  {                                                                                                \
    return fileInfo.first == #FTYPE;                                                               \
  }                                                                                                \
                                                                                                   \
  vtkSmartPointer<vtkDataObject> ReadVTKData_##FTYPE::operator()(                                  \
    const std::pair<std::string, std::string>& fileInfo)                                           \
  {                                                                                                \
    vtkNew<READER> rdr;                                                                            \
    rdr->SetFileName(fileInfo.second.c_str());                                                     \
    rdr->Update();                                                                                 \
                                                                                                   \
    vtkSmartPointer<CLASS> data = vtkSmartPointer<CLASS>::New();                                   \
    data->ShallowCopy(rdr->GetOutput());                                                           \
    return data;                                                                                   \
  }

/* clang-format off */
BasicReader_type(vtp, vtkPolyData, vtkXMLPolyDataReader)
BasicReader_type(vtu, vtkUnstructuredGrid, vtkXMLUnstructuredGridReader)
BasicReader_type(vti, vtkImageData, vtkXMLImageDataReader)
BasicReader_type(vtm, vtkMultiBlockDataSet, vtkXMLMultiBlockDataReader)
BasicReader_type(png, vtkImageData, vtkPNGReader)
#ifdef HELP_CLANG_FORMAT
;
#endif
/* clang-format on */

#undef BasicReader_type

bool ReadVTKData_obj::valid(const std::pair<std::string, std::string>& fileInfo) const
{
  return fileInfo.first == "obj";
}

vtkSmartPointer<vtkDataObject> ReadVTKData_obj::operator()(
  const std::pair<std::string, std::string>& fileInfo)
{
  vtkNew<vtkOBJReader> rdr;
  rdr->SetFileName(fileInfo.second.c_str());
  rdr->Update();

  vtkSmartPointer<vtkPolyData> data = vtkSmartPointer<vtkPolyData>::New();
  data->ShallowCopy(rdr->GetOutput());

  // If a data read prior to this call failed, the data passed to this method
  // may be incomplete. So, we guard against bad data to prevent the subsequent
  // logic from causing the program to crash.
  if (data == nullptr || data->GetNumberOfPoints() == 0)
  {
    return data;
  }

  vtkNew<vtkDoubleArray> pointCoords;
  pointCoords->ShallowCopy(data->GetPoints()->GetData());
  pointCoords->SetName("PointCoordinates");
  data->GetPointData()->AddArray(pointCoords.GetPointer());

  return data;
}

bool ReadVTKData_ply::valid(const std::pair<std::string, std::string>& fileInfo) const
{
  return fileInfo.first == "ply";
}

vtkSmartPointer<vtkDataObject> ReadVTKData_ply::operator()(
  const std::pair<std::string, std::string>& fileInfo)
{
  vtkNew<vtkPLYReader> rdr;
  rdr->SetFileName(fileInfo.second.c_str());
  rdr->Update();

  vtkSmartPointer<vtkPolyData> data = vtkSmartPointer<vtkPolyData>::New();
  data->ShallowCopy(rdr->GetOutput());

  // If a data read prior to this call failed, the data passed to this method
  // may be incomplete. So, we guard against bad data to prevent the subsequent
  // logic from causing the program to crash.
  if (data == nullptr || data->GetNumberOfPoints() == 0)
  {
    return data;
  }

  vtkNew<vtkDoubleArray> pointCoords;
  pointCoords->ShallowCopy(data->GetPoints()->GetData());
  pointCoords->SetName("PointCoordinates");
  data->GetPointData()->AddArray(pointCoords.GetPointer());

  return data;
}

bool ReadVTKData_pts::valid(const std::pair<std::string, std::string>& fileInfo) const
{
  return fileInfo.first == "pts" || fileInfo.first == "xyz";
}

vtkSmartPointer<vtkDataObject> ReadVTKData_pts::operator()(
  const std::pair<std::string, std::string>& fileInfo)
{
  vtkNew<vtkPTSReader> rdr;
  rdr->SetFileName(fileInfo.second.c_str());
  rdr->Update();

  vtkSmartPointer<vtkPolyData> data = vtkSmartPointer<vtkPolyData>::New();
  data->ShallowCopy(rdr->GetOutput());
  return data;
}

bool ReadVTKData_tif::valid(const std::pair<std::string, std::string>& fileInfo) const
{
  return fileInfo.first == "tif" || fileInfo.first == "tiff" || fileInfo.first == "dem";
}

vtkSmartPointer<vtkDataObject> ReadVTKData_tif::operator()(
  const std::pair<std::string, std::string>& fileInfo)
{
  vtkNew<vtkGDALRasterReader> rdr;
  rdr->SetFileName(fileInfo.second.c_str());
  rdr->Update();

  vtkSmartPointer<vtkImageData> outImage = vtkSmartPointer<vtkImageData>::New();
  outImage->ShallowCopy(rdr->GetOutput());

  vtkSmartPointer<vtkImageData> data = vtkSmartPointer<vtkImageData>::New();
  if (outImage.GetPointer())
  {
    // When dealing with indexed data into a color map, vtkGDALRasterReader
    // creates a point data named "Categories" and associates to it the
    // appropriate lookup table to convert to RGB space. We key off of the
    // existence of this scalar data to convert our data from indices to RGB.
    if (outImage->GetPointData() && outImage->GetPointData()->GetScalars() &&
      strcmp(outImage->GetPointData()->GetScalars()->GetName(), "Categories") == 0)
    {
      vtkNew<vtkImageMapToColors> imageMapToColors;
      imageMapToColors->SetInputData(outImage);
      imageMapToColors->SetLookupTable(outImage->GetPointData()->GetScalars()->GetLookupTable());
      imageMapToColors->Update();
      outImage->ShallowCopy(imageMapToColors->GetOutput());
    }
    vtkNew<vtkImageSpacingFlip> flipImage;
    flipImage->SetInputData(outImage);
    flipImage->Update();
    data->ShallowCopy(flipImage->GetOutput());
  }

  return data;
}

bool ReadVTKData_cmb::valid(const std::pair<std::string, std::string>& fileInfo) const
{
  return (fileInfo.first == "bin" || fileInfo.first == "vtk" || fileInfo.first == "2dm" ||
    fileInfo.first == "3dm" || fileInfo.first == "tin" || fileInfo.first == "poly" ||
    fileInfo.first == "smesh" || fileInfo.first == "fac" || fileInfo.first == "sol" ||
    fileInfo.first == "stl");
}

vtkSmartPointer<vtkDataObject> ReadVTKData_cmb::operator()(
  const std::pair<std::string, std::string>& fileInfo)
{
  vtkNew<vtkCMBGeometryReader> reader;
  reader->SetFileName(fileInfo.second.c_str());
  reader->SetPrepNonClosedSurfaceForModelCreation(false);
  reader->SetEnablePostProcessMesh(false);
  reader->Update();

  auto polyOutput = vtkSmartPointer<vtkPolyData>::New();
  polyOutput->ShallowCopy(reader->GetOutput());
  return polyOutput;
}

bool ReadVTKData_las::valid(const std::pair<std::string, std::string>& fileInfo) const
{
  return fileInfo.first == "las";
}

vtkSmartPointer<vtkDataObject> ReadVTKData_las::operator()(
  const std::pair<std::string, std::string>& fileInfo)
{
  vtkNew<vtkLASReader> reader;
  reader->SetFileName(fileInfo.second.c_str());
  reader->Update();

  vtkMultiBlockDataSet* readout = reader->GetOutput();
  vtkNew<vtkAppendPoints> appendPoints;

  vtkCompositeDataIterator* iter = readout->NewIterator();
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
  {
    vtkPolyData* blockPoly = vtkPolyData::SafeDownCast(iter->GetCurrentDataObject());
    if (!blockPoly)
    {
      vtkGenericWarningMacro(<< "This block from LAS reader is not a polydata!\n");
      continue;
    }
    appendPoints->AddInputData(blockPoly);
  }
  iter->Delete();
  appendPoints->Update();

  auto polyOutput = vtkSmartPointer<vtkPolyData>::New();
  polyOutput->ShallowCopy(appendPoints->GetOutput());
  return polyOutput;
}
}
