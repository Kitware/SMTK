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
#include "vtkSmartPointer.h"
#include <map>

class vtkDataSet;
class vtkPolyData;

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

  void clear();

protected:
  friend class Session;
  friend class BathymetryOperator;
  BathymetryHelper();

  typedef std::map<smtk::common::UUID,vtkSmartPointer<vtkPolyData> > ModelIdToMasterPolyMap;
  typedef std::map<smtk::common::UUID,std::string> ModelIdToBathymetryMap;

  std::map<std::string, vtkSmartPointer<vtkDataSet> > m_filesToSources;
  ModelIdToMasterPolyMap m_modelIdsToMasterPolys;
  ModelIdToBathymetryMap m_modelIdsToBathymetrys;

private:
  BathymetryHelper(const BathymetryHelper& other); // Not implemented.
  void operator = (const BathymetryHelper& other); // Not implemented.
};

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif // __smtk_model_BathymetryHelper_h
