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
// .NAME vtkCreateModelEdgesOperatorClient - Split a model face on the client
// .SECTION Description
// Operator to split a model face given an angle on the client.
// This will also perform the split operation on the server.

#ifndef __vtkCreateModelEdgesOperatorClient_h
#define __vtkCreateModelEdgesOperatorClient_h

#include "vtkCreateModelEdgesOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;
class vtkSMProxy;
class vtkDiscreteModel;
class vtkStringArray;

class VTK_EXPORT vtkCreateModelEdgesOperatorClient : public vtkCreateModelEdgesOperatorBase
{
public:
  static vtkCreateModelEdgesOperatorClient * New();
  vtkTypeMacro(vtkCreateModelEdgesOperatorClient,vtkCreateModelEdgesOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Modify the color and/or the visibility of an object. The
  // operator returns true for successful completion.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy,
                       const char* strSerializedModel);

protected:
  vtkCreateModelEdgesOperatorClient();
  virtual ~vtkCreateModelEdgesOperatorClient();

private:
  vtkCreateModelEdgesOperatorClient(const vtkCreateModelEdgesOperatorClient&);  // Not implemented.
  void operator=(const vtkCreateModelEdgesOperatorClient&);  // Not implemented.
};

#endif
