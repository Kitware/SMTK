//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "BathymetryHelper.h"

#ifdef HAS_GDAL_RASTER_READER
#include "smtk/bridge/discrete/extension/reader/vtkGDALRasterReader.h"
#include "smtk/bridge/discrete/extension/reader/vtkGDALRasterPolydataWrapper.h"
#endif

#include "smtk/bridge/discrete/extension/reader/vtkLASReader.h"
#include "smtk/bridge/discrete/extension/reader/vtkLIDARReader.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkNew.h"
#include "vtkPDataSetReader.h"
#include "vtkPolyData.h"
#include "vtkXMLPolyDataReader.h"
#include <vtksys/SystemTools.hxx>

namespace smtk {
  namespace bridge {
    namespace discrete {

/// Private constructor since this class is a base class which should not be instantiated.
BathymetryHelper::BathymetryHelper()
{
}

BathymetryHelper::~BathymetryHelper()
{
  this->m_filesToSources.clear();
}

bool BathymetryHelper::loadBathymetryFile(const std::string& filename)
{
  if (filename.empty())
    {
    std::cerr << "File name is empty!\n";
    return false;
    }
  // if the data is already loaded, return true;
  if(this->bathymetryData(filename) != NULL)
    return true;
  // get the ouptut
  vtkNew<vtkPolyData> output;

  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(filename);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  if(ext == ".pts" || ext == ".bin")
    {
    // binary or ASCII automatically determined
    vtkNew<vtkLIDARReader> reader;
    reader->SetFileName( filename.c_str() );
    reader->Update();
    output->ShallowCopy( reader->GetOutput() );
    }
#ifdef HAS_GDAL_RASTER_READER
  else if (ext == ".dem")
    {
    vtkNew<vtkGDALRasterPolydataWrapper> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();
    output->ShallowCopy( reader->GetOutput() );
    }
#endif
  else if(ext == ".vtk")
    {
    vtkNew<vtkPDataSetReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();

    vtkNew<vtkDataSetSurfaceFilter> surface;
    surface->SetInputData(0, reader->GetOutputDataObject(0));
    surface->Update();

    output->ShallowCopy( surface->GetOutput() );
    }
  else if(ext == ".vtp")
    {
    vtkNew<vtkXMLPolyDataReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();
    output->ShallowCopy( reader->GetOutput() );
    }
  else
    {
    std::cerr << "File type is not supported!\n";
    return false;
    }

  this->m_filesToSources[filename] = output.GetPointer();

  return true;
}

vtkPointSet* BathymetryHelper::bathymetryData(const std::string& filename)
{
  if(this->m_filesToSources.find(filename) !=
    this->m_filesToSources.end())
    return this->m_filesToSources[filename];
  return NULL;
}

void BathymetryHelper::loadedBathymetryFiles(
  std::vector<std::string> &result) const
{
  result.clear();
  std::map<std::string, vtkSmartPointer<vtkPointSet> >::const_iterator it;
  for(it=this->m_filesToSources.begin(); it!=this->m_filesToSources.end(); ++it)
    result.push_back(it->first);
}
void BathymetryHelper::clear()
{
  this->m_filesToSources.clear();
}

    } // namespace discrete
  } // namespace bridge
} // namespace smtk
