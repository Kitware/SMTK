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
#include "vtkSMOperatorProxy.h"

#include "vtkClientServerStream.h"
#include "vtkDiscreteModel.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkObjectFactory.h"
#include "vtkProcessModule.h"

vtkStandardNewMacro(vtkSMOperatorProxy);
//----------------------------------------------------------------------------
vtkSMOperatorProxy::vtkSMOperatorProxy()
{
}

//----------------------------------------------------------------------------
vtkSMOperatorProxy::~vtkSMOperatorProxy()
{
}

//----------------------------------------------------------------------------
void vtkSMOperatorProxy::Operate(vtkDiscreteModel* /*ClientModel*/,
                                 vtkSMProxy* ModelProxy)
{
  this->UpdateVTKObjects();

  // Make sure the proxy have been created on the server side
  ModelProxy->UpdateVTKObjects();

  vtkClientServerStream stream;
  stream  << vtkClientServerStream::Invoke
          << VTKOBJECT(this)
          << "Operate"
          << VTKOBJECT(ModelProxy)
          << vtkClientServerStream::End;
  // calls "Operate" function on object this->GetId() (which gets turned
  // into a pointer on the server) with argument model->GetID() (also
  // becomes a pointer on the server)
  this->ExecuteStream(stream);
}

//----------------------------------------------------------------------------
void vtkSMOperatorProxy::Operate(
  vtkDiscreteModel* /*ClientModel*/, vtkSMProxy* ModelProxy,
  vtkSMProxy* InputProxy)
{
  this->UpdateVTKObjects();

  // Make sure the proxy have been created on the server side
  ModelProxy->UpdateVTKObjects();
  InputProxy->UpdateVTKObjects();

  vtkClientServerStream stream;
  stream  << vtkClientServerStream::Invoke
          << VTKOBJECT(this)
          << "Operate"
          << VTKOBJECT(ModelProxy)
          << VTKOBJECT(InputProxy)
          << vtkClientServerStream::End;
  // calls "Operate" function on object this->GetId() (which gets turned
  // into a pointer on the server) with argument model->GetID() (also
  // becomes a pointer on the server)
  this->ExecuteStream(stream);
}

//----------------------------------------------------------------------------
vtkIdType vtkSMOperatorProxy::Build(vtkDiscreteModel* /*ClientModel*/,
                                    vtkSMProxy* ModelProxy)
{
  this->UpdateVTKObjects();

  // first make sure that we can get the unique persistent Id of the object
  // that is going to be built, otherwise we cannot call Build for this
  // operator
  vtkSMIdTypeVectorProperty* EntityIdProperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      this->GetProperty("BuiltEntityId"));
  if(!EntityIdProperty)
    {
    vtkErrorMacro("Can't find proxy to build entity on server.");
    return -1;
    }

  // Make sure the proxy have been created on the server side
  ModelProxy->UpdateVTKObjects();

  vtkClientServerStream stream;
  stream  << vtkClientServerStream::Invoke
          << VTKOBJECT(this)
          << "Build"
          << VTKOBJECT(ModelProxy)
          << vtkClientServerStream::End;
  // calls "Build" function on object this->GetId() (which gets turned
  // into a pointer on the server) with argument model->GetID() (also
  // becomes a pointer on the server)
  this->ExecuteStream(stream);

  this->UpdatePropertyInformation();
  // get the built entity's unique persistent id
  vtkIdType BuiltEntityId = EntityIdProperty->GetElement(0);

  return BuiltEntityId;
}

//----------------------------------------------------------------------------
bool vtkSMOperatorProxy::Destroy(vtkDiscreteModel* /*ClientModel*/,
                                 vtkSMProxy* ModelProxy)
{
  this->UpdateVTKObjects();

  // first make sure that we can get the unique persistent Id of the object
  // that is going to be built, otherwise we cannot call Build for this
  // operator
  vtkSMIntVectorProperty* DestroySucceededProperty =
    vtkSMIntVectorProperty::SafeDownCast(
      this->GetProperty("DestroySucceeded"));
  if(!DestroySucceededProperty)
    {
    vtkErrorMacro("Can't find proxy to destroy entity on server.");
    return 0;
    }

  // Make sure the proxy have been created on the server side
  ModelProxy->UpdateVTKObjects();

  vtkClientServerStream stream;
  stream  << vtkClientServerStream::Invoke
          << VTKOBJECT(this)
          << "Destroy"
          << VTKOBJECT(ModelProxy)
          << vtkClientServerStream::End;
  // calls "Build" function on object this->GetId() (which gets turned
  // into a pointer on the server) with argument model->GetID() (also
  // becomes a pointer on the server)
  this->ExecuteStream(stream);

  this->UpdatePropertyInformation();
  // get the built entity's unique persistent id
  if(DestroySucceededProperty->GetElement(0))
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void vtkSMOperatorProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


