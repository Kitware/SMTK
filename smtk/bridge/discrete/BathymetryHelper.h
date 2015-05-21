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

class vtkPointSet;

namespace smtk {
  namespace bridge {
    namespace discrete {

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

  vtkPointSet* bathymetryData(const std::string& filename);
  void loadedBathymetryFiles(
    std::vector<std::string> &result) const;
  void clear();

protected:
  friend class Session;
  BathymetryHelper();

  std::map<std::string, vtkSmartPointer<vtkPointSet> > m_filesToSources;

private:
  BathymetryHelper(const BathymetryHelper& other); // Not implemented.
  void operator = (const BathymetryHelper& other); // Not implemented.
};

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif // __smtk_model_BathymetryHelper_h
