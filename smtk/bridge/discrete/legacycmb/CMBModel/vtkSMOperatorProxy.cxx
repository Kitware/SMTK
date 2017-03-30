//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkSMOperatorProxy.h"

#include "vtkClientServerStream.h"
#include "vtkDiscreteModel.h"
#include "vtkObjectFactory.h"
#include "vtkProcessModule.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMStringVectorProperty.h"

vtkStandardNewMacro(vtkSMOperatorProxy);

vtkSMOperatorProxy::vtkSMOperatorProxy()
{
}

vtkSMOperatorProxy::~vtkSMOperatorProxy()
{
}

void vtkSMOperatorProxy::Operate(vtkDiscreteModel* /*ClientModel*/, vtkSMProxy* ModelProxy)
{
  this->UpdateVTKObjects();

  // Make sure the proxy have been created on the server side
  ModelProxy->UpdateVTKObjects();

  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke << VTKOBJECT(this) << "Operate" << VTKOBJECT(ModelProxy)
         << vtkClientServerStream::End;
  // calls "Operate" function on object this->GetId() (which gets turned
  // into a pointer on the server) with argument model->GetID() (also
  // becomes a pointer on the server)
  this->ExecuteStream(stream);
}

void vtkSMOperatorProxy::Operate(
  vtkDiscreteModel* /*ClientModel*/, vtkSMProxy* ModelProxy, vtkSMProxy* InputProxy)
{
  this->UpdateVTKObjects();

  // Make sure the proxy have been created on the server side
  ModelProxy->UpdateVTKObjects();
  InputProxy->UpdateVTKObjects();

  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke << VTKOBJECT(this) << "Operate" << VTKOBJECT(ModelProxy)
         << VTKOBJECT(InputProxy) << vtkClientServerStream::End;
  // calls "Operate" function on object this->GetId() (which gets turned
  // into a pointer on the server) with argument model->GetID() (also
  // becomes a pointer on the server)
  this->ExecuteStream(stream);
}

vtkIdType vtkSMOperatorProxy::Build(vtkDiscreteModel* /*ClientModel*/, vtkSMProxy* ModelProxy)
{
  this->UpdateVTKObjects();

  // first make sure that we can get the unique persistent Id of the object
  // that is going to be built, otherwise we cannot call Build for this
  // operator
  vtkSMIdTypeVectorProperty* EntityIdProperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(this->GetProperty("BuiltEntityId"));
  if (!EntityIdProperty)
  {
    vtkErrorMacro("Can't find proxy to build entity on server.");
    return -1;
  }

  // Make sure the proxy have been created on the server side
  ModelProxy->UpdateVTKObjects();

  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke << VTKOBJECT(this) << "Build" << VTKOBJECT(ModelProxy)
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

bool vtkSMOperatorProxy::Destroy(vtkDiscreteModel* /*ClientModel*/, vtkSMProxy* ModelProxy)
{
  this->UpdateVTKObjects();

  // first make sure that we can get the unique persistent Id of the object
  // that is going to be built, otherwise we cannot call Build for this
  // operator
  vtkSMIntVectorProperty* DestroySucceededProperty =
    vtkSMIntVectorProperty::SafeDownCast(this->GetProperty("DestroySucceeded"));
  if (!DestroySucceededProperty)
  {
    vtkErrorMacro("Can't find proxy to destroy entity on server.");
    return 0;
  }

  // Make sure the proxy have been created on the server side
  ModelProxy->UpdateVTKObjects();

  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke << VTKOBJECT(this) << "Destroy" << VTKOBJECT(ModelProxy)
         << vtkClientServerStream::End;
  // calls "Build" function on object this->GetId() (which gets turned
  // into a pointer on the server) with argument model->GetID() (also
  // becomes a pointer on the server)
  this->ExecuteStream(stream);

  this->UpdatePropertyInformation();
  // get the built entity's unique persistent id
  if (DestroySucceededProperty->GetElement(0))
  {
    return true;
  }
  return false;
}

void vtkSMOperatorProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
