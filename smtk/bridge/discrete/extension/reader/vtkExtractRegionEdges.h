//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkExtractRegionEdges - filter removes polylines below threshold length
// .SECTION Description
// The filter removes polyline below threshold length.  Before analysis,
// duplicate points are merged, and then any input lines are stripped.  Then,
// before discarding any polylines, the polylines are appended to adjacent
// polylines where possible.

#ifndef __smtkdiscrete_vtkExtractRegionEdges_h
#define __smtkdiscrete_vtkExtractRegionEdges_h

#include "smtk/bridge/discrete/extension/vtkSMTKDiscreteExtModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

namespace smtk {
  namespace bridge {
    namespace discrete {

class VTKSMTKDISCRETEEXT_EXPORT vtkExtractRegionEdges : public vtkPolyDataAlgorithm
{
public:
  static vtkExtractRegionEdges *New();
  vtkTypeMacro(vtkExtractRegionEdges,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetStringMacro(RegionArrayName);
  vtkGetStringMacro(RegionArrayName);

  vtkGetMacro(RegionIdentifiersModified, bool);

//BTX
protected:
  vtkExtractRegionEdges();
  ~vtkExtractRegionEdges();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

  void ConvertInputToPolyData(vtkPointSet *input, vtkPolyData *polyData);
  void ExtractRegionEdgeSegments(vtkPolyData *polyData, vtkCellArray *lines,
    vtkPolyData *linePolyData);
  void BuildRegionLoops(vtkPolyData *polyData);

  void UpdateRegionIdentifiersIfNecessary(vtkPolyData *outputPD);
  void SetupOutputFieldData(vtkPolyData *output);


private:
  vtkExtractRegionEdges(const vtkExtractRegionEdges&);  // Not implemented.
  void operator=(const vtkExtractRegionEdges&);  // Not implemented.

  char *RegionArrayName;
  bool RegionIdentifiersModified;

  class vtkInternal;
  vtkInternal* Internal;
//ETX
};

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif
