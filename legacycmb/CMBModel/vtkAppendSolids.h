/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed, or modified, in any form or by any means, without
permission in writing from Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME vtkAppendSolids - filter appends polydata inputs and adds region specifier
// .SECTION Description
// This filter is basically a glorified vtkAppendPolyData filter that also
// sets the "Region" value of each input to a different value (starting at 0).
// The output is also cleaned (if there is more than 1 input).
// .SECTION See Also

#ifndef __vtkAppendSolids_h
#define __vtkAppendSolids_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkPolyData;

class VTKCMBDISCRETEMODEL_EXPORT vtkAppendSolids : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkAppendSolids,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkAppendSolids *New();

  // Description:
  // Set 2nd input input to the filter (required)
  void AddInputData(vtkPolyData *input);

  vtkSetStringMacro(RegionArrayName);
  vtkGetStringMacro(RegionArrayName);

protected:
  vtkAppendSolids();
  ~vtkAppendSolids();

  // Usual data generation method
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int, vtkInformation *);

private:
  vtkAppendSolids(const vtkAppendSolids&);  // Not implemented.
  void operator=(const vtkAppendSolids&);  // Not implemented.

  char *RegionArrayName;

};

#endif
