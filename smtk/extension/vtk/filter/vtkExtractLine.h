//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkExtractLine - filter extracts a single line from the input polydata
// .SECTION Description
// The filter extracts the line indicated by LineId into a separate polydata,
// using the points from the input.  This is extremely specialiazed, and
// perhaps shoould be removed for a more general algorithm (which may
// already exist).  Note, however, that it is no accident that the output
// points are the same as the input points, as we expect to call this
// filter multiple times for each line in a polydata, and we want each
// as a separate polydate but all using the same points.

#ifndef __smtk_vtk_ExtractLine_h
#define __smtk_vtk_ExtractLine_h

#include "smtk/extension/vtk/filter/Exports.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class VTKSMTKFILTEREXT_EXPORT vtkExtractLine : public vtkPolyDataAlgorithm
{
public:
  static vtkExtractLine *New();
  vtkTypeMacro(vtkExtractLine,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The Id of the line to extract
  vtkSetMacro(LineId, int);
  vtkGetMacro(LineId, int);

//BTX
protected:
  vtkExtractLine();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  void TryAppend(vtkPolyData *linePD, vtkCellArray *polyLines,
    vtkIdType currentCell, vtkIdType npts, vtkIdType *ptIds, char *appended);
  void AppendToLine(vtkPolyData *linePD,
    vtkIdList *appendedLine, vtkIdType cellToAdd, char *appended);

private:
  vtkExtractLine(const vtkExtractLine&);  // Not implemented.
  void operator=(const vtkExtractLine&);  // Not implemented.

  int LineId;
//ETX
};

#endif
