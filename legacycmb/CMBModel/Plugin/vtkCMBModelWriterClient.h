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
// .NAME vtkCMBModelWriterClient - Wrapper on the client to write out a CMB file on the server.
// .SECTION Description
// Wrapper operator on the client to write out a CMB file on the server.
// It currently gets written
// out as a vtkPolyData using vtkXMLPolyDataWriter with the necessary
// information included in the field data.

#ifndef __vtkCMBModelWriterClient_h
#define __vtkCMBModelWriterClient_h

#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkSMProxy;

class VTK_EXPORT vtkCMBModelWriterClient : public vtkObject
{
public:
  static vtkCMBModelWriterClient * New();
  vtkTypeMacro(vtkCMBModelWriterClient,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Write the CMB file out.
  bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Set/get the version of the file to be written.  Currently
  // this is ignored as there is only a single version.
  vtkSetMacro(Version, int);
  vtkGetMacro(Version, int);

protected:
  vtkCMBModelWriterClient();
  virtual ~vtkCMBModelWriterClient();

private:
  vtkCMBModelWriterClient(const vtkCMBModelWriterClient&);  // Not implemented.
  void operator=(const vtkCMBModelWriterClient&);  // Not implemented.

  // Description:
  // The name of the file to be written.
  char* FileName;

  // Description:
  // The version of the file.
  int Version;
};

#endif
