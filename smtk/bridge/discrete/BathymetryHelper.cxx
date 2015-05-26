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

#include "smtk/bridge/discrete/extension/reader/vtkCMBGeometryReader.h"
#include "smtk/bridge/discrete/extension/reader/vtkLASReader.h"
#include "smtk/bridge/discrete/Session.h"

#include "vtkAppendPoints.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkMultiBlockDataset.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include <vtksys/SystemTools.hxx>
#include "DiscreteMesh.h"

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

  if(ext == ".pts" || ext == ".bin"
     || ext == ".vtk"
     || ext == ".vtp"
     || ext == ".2dm" || ext == ".3dm"
     || ext == ".tin"
     || ext == ".poly" || ext == ".smesh"
     || ext == ".obj"
     || ext == ".fac"
     || ext == ".sol"
     || ext == ".stl"
     )
    {
    vtkNew<vtkCMBGeometryReader> reader;
    reader->SetFileName( filename.c_str() );
    reader->SetPrepNonClosedSurfaceForModelCreation(false);
    reader->SetEnablePostProcessMesh(false);
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
  else if(ext == ".las")
    {
    vtkNew<vtkLASReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();
    vtkMultiBlockDataSet* readout = reader->GetOutput();
    vtkNew<vtkAppendPoints> appendPoints;

    vtkCompositeDataIterator* iter = readout->NewIterator();
    for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
      {
      vtkPolyData* blockPoly = vtkPolyData::SafeDownCast(iter->GetCurrentDataObject());
      if(!blockPoly)
        {
        std::cerr << "This block from LAS reader is not a polydata!\n";
        continue;
        }
      appendPoints->AddInputData(blockPoly);
      }
    iter->Delete();
    appendPoints->Update();

    output->ShallowCopy( appendPoints->GetOutput() );
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

void BathymetryHelper::addModelBathymetry(const smtk::common::UUID& modelId,
                          const std::string& bathyfile)
{
  this->m_modelIdsToBathymetrys[modelId] = bathyfile;
}

void BathymetryHelper::removeModelBathymetry(const smtk::common::UUID& modelId)
{
  this->m_modelIdsToBathymetrys.erase(modelId);
}

bool BathymetryHelper::hasModelBathymetry(const smtk::common::UUID& modelId)
{
  return this->m_modelIdsToBathymetrys.find(modelId) != this->m_modelIdsToBathymetrys.end();
}

vtkPolyData* BathymetryHelper::findOrShallowCopyModelPoly(
  const smtk::common::UUID& modelId, smtk::bridge::discrete::Session* session)
{
  if(this->m_modelIdsToMasterPolys.find(modelId) != this->m_modelIdsToMasterPolys.end())
    return this->m_modelIdsToMasterPolys[modelId].GetPointer();

  vtkDiscreteModelWrapper* modelWrap = session->findModelEntity(modelId);
  if(!modelWrap)
    return NULL;

  vtkNew<vtkPolyData> tmpModelPoly;
  tmpModelPoly->Initialize();
  tmpModelPoly->SetPoints(modelWrap->GetModel()->GetMesh().SharePointsPtr());

  this->m_modelIdsToMasterPolys[modelId] = tmpModelPoly.GetPointer();
  return tmpModelPoly.GetPointer();
}

void BathymetryHelper::clear()
{
  this->m_filesToSources.clear();
  this->m_modelIdsToMasterPolys.clear();
  this->m_modelIdsToBathymetrys.clear();
}


    } // namespace discrete
  } // namespace bridge
} // namespace smtk
