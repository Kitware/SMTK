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

#include "vtkModelEdgeOperatorBase.h"

#include "vtkDiscreteModel.h"
#
#include "vtkObjectFactory.h"
#include "vtkDiscreteModelEdge.h"

vtkStandardNewMacro(vtkModelEdgeOperatorBase);

vtkModelEdgeOperatorBase::vtkModelEdgeOperatorBase()
{
  this->IsLineResolutionSet = 0;
  this->LineResolution = 1;
}

vtkModelEdgeOperatorBase::~vtkModelEdgeOperatorBase()
{
}

vtkDiscreteModelEdge* vtkModelEdgeOperatorBase::GetModelEdgeEntity(
  vtkDiscreteModel* Model)
{
  if(!Model)
    {
    return 0;
    }
  return vtkDiscreteModelEdge::SafeDownCast(
    this->Superclass::GetModelEntity(Model));
}

bool vtkModelEdgeOperatorBase::AbleToOperate(vtkDiscreteModel* Model)
{
  if(!Model)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }

  if(this->GetIsIdSet() == 0)
    {
    vtkErrorMacro("No entity id specified.");
    return 0;
    }
  if(this->GetIsLineResolutionSet() == 0)
    {
    vtkErrorMacro("No line resolution is set.");
    return 0;
    }

  return 1;
}

void vtkModelEdgeOperatorBase::SetLineResolution(int resolution)
{
  this->IsLineResolutionSet = 1;
  if(resolution != this->LineResolution)
    {
    this->LineResolution = resolution;
    this->Modified();
    }
}

bool vtkModelEdgeOperatorBase::Operate(vtkDiscreteModel* Model)
{
  if(!this->AbleToOperate(Model))
    {
    return 0;
    }

  return this->Superclass::Operate(Model);
}

void vtkModelEdgeOperatorBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "IsLineResolutionSet: " << this->IsLineResolutionSet << endl;
  os << indent << "LineResolution: " << this->LineResolution << endl;
}
