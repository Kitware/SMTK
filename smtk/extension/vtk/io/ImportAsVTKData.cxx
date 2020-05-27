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

#include "smtk/extension/vtk/io/ImportAsVTKData.h"

#include "smtk/common/Paths.h"

#include "smtk/extension/vtk/filter/vtkImageDual.h"
#include "smtk/extension/vtk/filter/vtkImageSpacingFlip.h"

#include "vtkAppendFilter.h"
#include "vtkAppendPoints.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataObject.h"
#include "vtkDataObjectTreeIterator.h"
#include "vtkDataSet.h"
#include "vtkDataSetReader.h"
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
#include "vtkSLACReader.h"
#include "vtkSTLReader.h"
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

ImportAsVTKData::ImportAsVTKData() = default;

ImportAsVTKData::~ImportAsVTKData() = default;

std::vector<ImportFormat> ImportAsVTKData::fileFormats() const
{
  std::vector<ImportFormat> formats;
  auto gens = ImportAsVTKData::generators().lock();
  if (gens != nullptr)
  {
    for (auto gen : *gens)
    {
      auto formats_ = gen->fileFormats();
      formats.insert(formats.end(), formats_.begin(), formats_.end());
    }
  }
  return formats;
}

bool ImportAsVTKData::valid(const std::string& file) const
{
  std::string fileType = smtk::common::Paths::extension(file);
  // If the file type isn't the empty string, remove the leading ".".
  if (fileType.begin() != fileType.end() && fileType[0] == '.')
  {
    fileType.erase(fileType.begin());
  }

  return this->valid(std::make_pair(fileType, file));
}

vtkSmartPointer<vtkDataObject> ImportAsVTKData::operator()(const std::string& file)
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
  class ImportAsVTKData_##FTYPE                                                                    \
    : public smtk::extension::vtk::io::ImportAsVTKDataType<ImportAsVTKData_##FTYPE>                \
  {                                                                                                \
  public:                                                                                          \
    ImportAsVTKData_##FTYPE();                                                                     \
    vtkSmartPointer<vtkDataObject> operator()(                                                     \
      const std::pair<std::string, std::string>& fileInfo) override;                               \
  };                                                                                               \
  static bool registered_##FTYPE = ImportAsVTKData_##FTYPE::registerClass()

DeclareReader_type(vtp);
DeclareReader_type(vtu);
DeclareReader_type(vti);
DeclareReader_type(vtm);
DeclareReader_type(vtk);
DeclareReader_type(obj);
DeclareReader_type(ply);
DeclareReader_type(pts);
DeclareReader_type(tif);
DeclareReader_type(png);
DeclareReader_type(slac);
DeclareReader_type(stl);

#undef DeclareReader_type

#define BasicReader_type(FTYPE, DESC, CLASS, READER)                                               \
  ImportAsVTKData_##FTYPE::ImportAsVTKData_##FTYPE()                                               \
    : ImportAsVTKDataType<ImportAsVTKData_##FTYPE>(                                                \
        { smtk::extension::vtk::io::ImportFormat(DESC, { #FTYPE }) })                              \
  {                                                                                                \
  }                                                                                                \
  vtkSmartPointer<vtkDataObject> ImportAsVTKData_##FTYPE::operator()(                              \
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
BasicReader_type(vtp, "VTK PolyData", vtkPolyData, vtkXMLPolyDataReader)
BasicReader_type(vtu, "VTK Unstructured Grid", vtkUnstructuredGrid, vtkXMLUnstructuredGridReader)
BasicReader_type(vti, "VTK Image Data", vtkImageData, vtkXMLImageDataReader)
BasicReader_type(vtm, "VTK MultiBlock Data Set", vtkMultiBlockDataSet, vtkXMLMultiBlockDataReader)
BasicReader_type(png, "Portable Network Graphics File", vtkImageData, vtkPNGReader)
BasicReader_type(stl, "Stereolithography File", vtkPolyData, vtkSTLReader)
#ifdef HELP_CLANG_FORMAT
;
#endif
/* clang-format on */

#undef BasicReader_type

ImportAsVTKData_vtk::ImportAsVTKData_vtk()
  : ImportAsVTKDataType<ImportAsVTKData_vtk>(
      smtk::extension::vtk::io::ImportFormat("Legacy VTK File", { "vtk" }))
{
}

vtkSmartPointer<vtkDataObject> ImportAsVTKData_vtk::operator()(
  const std::pair<std::string, std::string>& fileInfo)
{
  vtkNew<vtkDataSetReader> rdr;
  rdr->SetFileName(fileInfo.second.c_str());
  rdr->Update();

  vtkDataSet* data = rdr->GetOutput();
  if (vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast(data))
  {
    return ugrid;
  }
  else if (vtkPolyData* polydata = vtkPolyData::SafeDownCast(data))
  {
    return polydata;
  }

  return vtkSmartPointer<vtkDataObject>();
}

ImportAsVTKData_obj::ImportAsVTKData_obj()
  : ImportAsVTKDataType<ImportAsVTKData_obj>(
      { smtk::extension::vtk::io::ImportFormat("Wavefront OBJ File", { "obj" }) })
{
}

vtkSmartPointer<vtkDataObject> ImportAsVTKData_obj::operator()(
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

ImportAsVTKData_ply::ImportAsVTKData_ply()
  : ImportAsVTKDataType<ImportAsVTKData_ply>(
      { smtk::extension::vtk::io::ImportFormat("Polygon File Format", { "ply" }) })
{
}

vtkSmartPointer<vtkDataObject> ImportAsVTKData_ply::operator()(
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

ImportAsVTKData_pts::ImportAsVTKData_pts()
  : ImportAsVTKDataType<ImportAsVTKData_pts>(
      { smtk::extension::vtk::io::ImportFormat("Points File", { "pts", "xyz" }) })
{
}

vtkSmartPointer<vtkDataObject> ImportAsVTKData_pts::operator()(
  const std::pair<std::string, std::string>& fileInfo)
{
  vtkNew<vtkPTSReader> rdr;
  rdr->SetFileName(fileInfo.second.c_str());
  rdr->Update();

  vtkSmartPointer<vtkPolyData> data = vtkSmartPointer<vtkPolyData>::New();
  data->ShallowCopy(rdr->GetOutput());
  return data;
}

ImportAsVTKData_tif::ImportAsVTKData_tif()
  : ImportAsVTKDataType<ImportAsVTKData_tif>(
      { smtk::extension::vtk::io::ImportFormat("Tagged Image File", { "tif", "tiff", "dem" }) })
{
}

vtkSmartPointer<vtkDataObject> ImportAsVTKData_tif::operator()(
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
    // vtkGDALRasterReader has switched from storing its data as point data to
    // cell data. Traditional vtkImageData routines expect data as point data.
    // We therefore construct a dual graph to maintain VTK's paradigm.
    vtkNew<vtkImageDual> imageDual;
    imageDual->SetInputData(outImage);
    imageDual->Update();
    outImage->ShallowCopy(imageDual->GetOutput());

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

ImportAsVTKData_slac::ImportAsVTKData_slac()
  : ImportAsVTKDataType<ImportAsVTKData_slac>(
      { smtk::extension::vtk::io::ImportFormat("SLAC File", { "nc", "ncdf", "slac" }) })
{
}

vtkSmartPointer<vtkDataObject> ImportAsVTKData_slac::operator()(
  const std::pair<std::string, std::string>& fileInfo)
{
  vtkNew<vtkSLACReader> reader;
  reader->SetMeshFileName(fileInfo.second.c_str());
  reader->SetReadInternalVolume(1);
  reader->ReadExternalSurfaceOn();
  reader->ReadMidpointsOff();
  reader->Update();

  vtkMultiBlockDataSet* readout = reader->GetOutput();
  vtkNew<vtkAppendFilter> appendFilter;

  vtkCompositeDataIterator* iter = readout->NewIterator();
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
  {
    vtkUnstructuredGrid* block = vtkUnstructuredGrid::SafeDownCast(iter->GetCurrentDataObject());
    if (!block)
    {
      vtkGenericWarningMacro(<< "This block from SLAC reader is not an unstructured grid!\n");
      continue;
    }
    appendFilter->AddInputData(block);
  }
  iter->Delete();
  appendFilter->Update();

  auto uGridOutput = vtkSmartPointer<vtkUnstructuredGrid>::New();
  uGridOutput->ShallowCopy(appendFilter->GetOutput());
  return uGridOutput;
}
}
