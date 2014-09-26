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
// .NAME vtkEdgeSplitOperatorClient - Split a model edge
// .SECTION Description
// Operator to split a model edge given a point id for the grid.  This
// will trigger the split operation on the server first and if it's
// successful it will do the same on the client. Note
// that the master polydata and the model edge polydata share the
// same point set so there is no concern about getting the point Ids
// confused.

#ifndef __vtkEdgeSplitOperatorClient_h
#define __vtkEdgeSplitOperatorClient_h

#include "vtkEdgeSplitOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkSMProxy;

class VTK_EXPORT vtkEdgeSplitOperatorClient : public vtkEdgeSplitOperatorBase
{
public:
  static vtkEdgeSplitOperatorClient * New();
  vtkTypeMacro(vtkEdgeSplitOperatorClient,vtkEdgeSplitOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Initiate the split on the server and then follow up with the
  // split on the client.
  virtual bool Operate(vtkDiscreteModel* model, vtkSMProxy* serverModelProxy);

protected:
  vtkEdgeSplitOperatorClient();
  virtual ~vtkEdgeSplitOperatorClient();

private:
  vtkEdgeSplitOperatorClient(const vtkEdgeSplitOperatorClient&);  // Not implemented.
  void operator=(const vtkEdgeSplitOperatorClient&);  // Not implemented.
};

#endif
