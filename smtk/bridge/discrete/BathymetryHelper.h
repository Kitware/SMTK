//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_BathymetryHelper_h
#define __smtk_model_BathymetryHelper_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/bridge/discrete/Exports.h" // for SMTKDISCRETESESSION_EXPORT
#include "smtk/SharedFromThis.h" // for smtkTypeMacro
#include "smtk/mesh/ForEachTypes.h"

#include "vtkSmartPointer.h"
#include <map>
#include <vector>

class vtkDataSet;
class vtkPolyData;
class vtkPoints;

namespace smtk {
  namespace bridge {
    namespace discrete {

class Session;

/**\brief Helper class for loading and caching bathymetry files for session.
  *
  * This class will read the following files and cache their output:
  * "LIDAR (*.pts *.bin *.bin.pts);;LAS (*.las);;DEM (*.hdr *.FLT *.ftw);;VTK files (*.vtk)"
  */
class SMTKDISCRETESESSION_EXPORT BathymetryHelper
{
public:
  smtkTypeMacro(BathymetryHelper);
  virtual ~BathymetryHelper();

  virtual bool loadBathymetryFile(const std::string& filename);

  vtkDataSet* bathymetryData(const std::string& filename);
  void loadedBathymetryFiles(
    std::vector<std::string> &result) const;

  void addModelBathymetry(const smtk::common::UUID& modelId,
                          const std::string& bathyfile);
  void removeModelBathymetry(const smtk::common::UUID& modelId);
  bool hasModelBathymetry(const smtk::common::UUID& modelId);
  vtkPolyData* findOrShallowCopyModelPoly(
    const smtk::common::UUID& modelId,
    Session* session);

  const std::vector<double>& cachedMeshPointsZ(const smtk::common::UUID& collectionId) const;
  bool storeMeshPointsZ(smtk::mesh::CollectionPtr collection);
  bool resetMeshPointsZ(smtk::mesh::CollectionPtr collection);
  void clear();

  bool computeBathymetryPoints(vtkDataSet* input, vtkPoints* output);

protected:
  friend class Session;
  friend class BathymetryOperator;
  BathymetryHelper();

  typedef std::map<smtk::common::UUID,vtkSmartPointer<vtkPolyData> > ModelIdToMasterPolyMap;
  typedef std::map<smtk::common::UUID,std::string> ModelIdToBathymetryMap;
  typedef std::map<smtk::common::UUID,std::vector<double> > MeshIdToPointsMap;

  std::map<std::string, vtkSmartPointer<vtkDataSet> > m_filesToSources;
  ModelIdToMasterPolyMap m_modelIdsToMasterPolys;
  ModelIdToBathymetryMap m_modelIdsToBathymetrys;

  MeshIdToPointsMap m_meshIdsToPoints;

private:
  BathymetryHelper(const BathymetryHelper& other); // Not implemented.
  void operator = (const BathymetryHelper& other); // Not implemented.

  const std::vector<double> m_dummy;
};

//----------------------------------------------------------------------------
class ZValueHelper : public smtk::mesh::PointForEach
{
  std::vector<double>& m_originalZs;
  bool m_modifyZ;
public:
  ZValueHelper(std::vector<double>& originalZs, bool modifyZ) :
    m_originalZs(originalZs),
    m_modifyZ(modifyZ)
  {
  }

  void forPoints(const smtk::mesh::HandleRange& pointIds,
                 std::vector<double>& xyz,
                 bool& coordinatesModified)
  {
  if(!m_modifyZ)
    {
    m_originalZs.resize(pointIds.size());
    }
  if(m_modifyZ && xyz.size() != (m_originalZs.size() * 3) )
    {
    std::cerr << "originalZs size does not match with PointSet points!\n";
    return;
    }
  coordinatesModified = m_modifyZ;
  for(std::size_t offset = 0; offset < xyz.size(); offset+=3)
    {
    // modify
    if(m_modifyZ)
      {
      //reset Z to original
      xyz[offset+2] = m_originalZs[offset/3];
      }
    // copy
    else
      {
      m_originalZs[offset/3] = xyz[offset+2];
      }
    }
  }
};

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif // __smtk_model_BathymetryHelper_h
