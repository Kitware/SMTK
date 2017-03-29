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
#include "smtk/SharedFromThis.h" // for smtkTypeMacro
#include "smtk/extension/vtk/operators/Exports.h" // for VTKSMTKOPERATORSEXT_EXPORT
#include "smtk/mesh/ForEachTypes.h"

#include "vtkSmartPointer.h"
#include <iostream>
#include <map>
#include <vector>

class vtkDataSet;
class vtkPolyData;
class vtkPoints;

namespace smtk {
  namespace model {

static const std::string BO_elevation("bathymetry original elevations");

class Session;

/**\brief Helper class for loading and caching bathymetry files for session.
  *
  * This class will read the following files and cache their output:
  * "LIDAR (*.pts *.bin *.bin.pts);;LAS (*.las);;DEM (*.hdr *.FLT *.ftw);;VTK files (*.vtk)"
  */
class VTKSMTKOPERATORSEXT_EXPORT BathymetryHelper
{
public:
  smtkTypeMacro(BathymetryHelper);
  virtual ~BathymetryHelper();

  virtual bool loadBathymetryFile(const std::string& filename);

  vtkDataSet* bathymetryData(const std::string& filename);
  void loadedBathymetryFiles(
    std::vector<std::string> &result) const;

  bool storeMeshPointsZ(smtk::mesh::CollectionPtr collection);
  bool resetMeshPointsZ(smtk::mesh::CollectionPtr collection);
  void clear();

  bool computeBathymetryPoints(vtkDataSet* input, vtkPoints* output);

  vtkIdType GenerateRepresentationFromModel(
      vtkPoints* pts, const smtk::model::EntityRef& entityref);

  void CopyCoordinatesToTessellation(
      vtkPoints* pts, const smtk::model::EntityRef& entityref,
      const vtkIdType startingIndex);

  void GetZValuesFromMasterModelPts(vtkPoints *pts, std::vector<double> & zValues);
  bool SetZValuesIntoMasterModelPts(vtkPoints *pts, const std::vector<double>* zValues);

protected:
  friend class Session;
  friend class BathymetryOperator;
  BathymetryHelper();

  std::map<std::string, vtkSmartPointer<vtkDataSet> > m_filesToSources;

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

  } // namespace model
} // namespace smtk

#endif // __smtk_model_BathymetryHelper_h
