/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

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
// .NAME vtkSMTKModelReader -
// .SECTION Description

#ifndef __vtkSMTKModelReader_h
#define __vtkSMTKModelReader_h

#include "smtk/pvSMTKExports.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include <string>

class PVSMTK_EXPORT vtkSMTKModelReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkSMTKModelReader * New();
  vtkTypeMacro(vtkSMTKModelReader,vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Get json string of the smtk model
  virtual const char* GetJSONModel()
    {return this->JSONModel.c_str();}

protected:
  vtkSMTKModelReader();
  virtual ~vtkSMTKModelReader();

  int RequestInformation(vtkInformation *,
        vtkInformationVector **,
        vtkInformationVector *);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  // Description:
  // The name of the file to be read in.
  char* FileName;

  // Description:
  // smtk model in json
  std::string JSONModel;

  vtkSMTKModelReader(const vtkSMTKModelReader&);  // Not implemented.
  void operator=(const vtkSMTKModelReader&);  // Not implemented.
};

#endif
