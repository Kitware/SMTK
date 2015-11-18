//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBGeometryReader - "reader" for various SceneGen geometry formats
// .SECTION Description
// Not actually a reader in the sense that it internally creates the appropriate
// reader based on the filename's extension.

#ifndef __smtkdiscrete_CMBGeometryReader_h
#define __smtkdiscrete_CMBGeometryReader_h

#include "smtk/bridge/discrete/extension/reader/vtkSMTKDiscreteReaderExtModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"


namespace smtk {
  namespace bridge {
    namespace discrete {

class VTKSMTKDISCRETEREADEREXT_EXPORT vtkCMBGeometryReader : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBGeometryReader *New();
  vtkTypeMacro(vtkCMBGeometryReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // If the file read contains a 2D mesh, that has boundary edges,
  // PrepNonClosedSurfaceForModelCreation determines
  // whether to prep the data for model creation; whether or not vertices /
  // edges / loops are detected.  Right now only used for 3dm and vtk files
  vtkBooleanMacro(PrepNonClosedSurfaceForModelCreation, bool);
  vtkSetMacro(PrepNonClosedSurfaceForModelCreation, bool);
  vtkGetMacro(PrepNonClosedSurfaceForModelCreation, bool);

  // Description:
  // If enabled, the data read by the reader if further post-processed after
  // reading.  For now, this only has an affect for vtk, and 2dm/3dm files...
  // other file types are post processed (or not) as previously done before
  // adding this variable.
  vtkBooleanMacro(EnablePostProcessMesh, bool);
  vtkSetMacro(EnablePostProcessMesh, bool);
  vtkGetMacro(EnablePostProcessMesh, bool);

  // Description:
  // Get whether the mesh has boundary edges (only for 3dm and vtk files that do
  // NOT contain volume elments).  Note the value will always return false for
  // all other reader types.
  vtkGetMacro(HasBoundaryEdges, bool);

  // Description:
  // Get whether the Cmb3dm reader modified the regions identifiers
  vtkGetMacro(RegionIdentifiersModified, bool);

protected:
  vtkCMBGeometryReader();
  ~vtkCMBGeometryReader();

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  void PostProcessMesh(vtkDataSet *dataset, bool is3DVolumeMesh,
    bool passThroughPointIds, const char *regionArrayName, vtkPolyData *output);

  char *FileName;

  vtkSetMacro(HasBoundaryEdges, bool);
  vtkSetMacro(RegionIdentifiersModified, bool);

private:
  vtkCMBGeometryReader(const vtkCMBGeometryReader&);  // Not implemented.
  void operator=(const vtkCMBGeometryReader&);  // Not implemented.

  bool PrepNonClosedSurfaceForModelCreation;
  bool HasBoundaryEdges;
  bool RegionIdentifiersModified;
  bool EnablePostProcessMesh;
};

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif
