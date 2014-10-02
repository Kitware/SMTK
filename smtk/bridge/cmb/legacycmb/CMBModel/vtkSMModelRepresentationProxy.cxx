/*=========================================================================

  Program:   ParaView
  Module:    vtkSMModelRepresentationProxy.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMModelRepresentationProxy.h"

#include "vtkClientServerStream.h"
#include "vtkCollection.h"
#include "vtkObjectFactory.h"
#include "vtkProcessModule.h"
#include "vtkPVXMLElement.h"
#include "vtkSmartPointer.h"
#include "vtkSMEnumerationDomain.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMPropertyHelper.h"

#include <string>

vtkStandardNewMacro(vtkSMModelRepresentationProxy);
//----------------------------------------------------------------------------
vtkSMModelRepresentationProxy::vtkSMModelRepresentationProxy()
{
  this->Representation = SURFACE;
}

//----------------------------------------------------------------------------
vtkSMModelRepresentationProxy::~vtkSMModelRepresentationProxy()
{
}

//----------------------------------------------------------------------------
void vtkSMModelRepresentationProxy::SetRepresentation(int repr)
{
  if (this->Representation != repr)
    {
    this->Representation = repr;

    vtkSMProxy* subProxy = this->GetSubProxy("ModelRepresentation");
    if (subProxy)
      {
      //vtkSMProperty* repProperty =
      //  subProxy->GetProperty("Representation");
      //vtkSMEnumerationDomain* ed = vtkSMEnumerationDomain::SafeDownCast(
      //  repProperty->GetDomain("enum"));
      //if(ed)
      //  {
      //  vtkClientServerStream stream;
      //  stream << vtkClientServerStream::Invoke
      //    << this->GetID()
      //    << "SetActiveRepresentation"
      //    << ed->GetEntryText(repr)
      //    << vtkClientServerStream::End;
      //  vtkProcessModule::GetProcessModule()->SendStream(
      //    this->ConnectionID, this->Servers, stream);
      //  }
      vtkSMPropertyHelper(subProxy, "Representation").Set(this->Representation);
      subProxy->UpdateVTKObjects();
      }
    this->Modified();
    }
  this->InvalidateDataInformation();
}

//----------------------------------------------------------------------------
void vtkSMModelRepresentationProxy::CreateVTKObjects()
{
  if (this->ObjectsCreated)
    {
    return;
    }
  this->Superclass::CreateVTKObjects();
  if (!this->ObjectsCreated)
    {
    return;
    }
/*
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
    << VTKOBJECT(this) << "SetCubeAxesRepresentation"
    << VTKOBJECT(this->GetSubProxy("CubeAxesRepresentation"))
  << vtkClientServerStream::End;
  this->ExecuteStream(stream);

  stream << vtkClientServerStream::Invoke
    << VTKOBJECT(this) << "SetSelectionRepresentation"
    << VTKOBJECT(this->GetSubProxy("SelectionRepresentation"))
    << vtkClientServerStream::End;
  this->ExecuteStream(stream);

  vtkSMProxy* subProxy = this->GetSubProxy("ModelRepresentation");
  if (subProxy)
    {
    vtkSMPropertyHelper(subProxy, "Visibility").Set(0);
    subProxy->UpdateVTKObjects();
    vtkSMProperty* repProperty =
      subProxy->GetProperty("Representation");
    vtkSMEnumerationDomain* ed = vtkSMEnumerationDomain::SafeDownCast(
      repProperty->GetDomain("enum"));
    if(ed)
      {
      for (unsigned int i=0; i<ed->GetNumberOfEntries(); i++)
        {
        stream << vtkClientServerStream::Invoke
          << VTKOBJECT(this) << "AddRepresentation"
          << ed->GetEntryText(i)
          << VTKOBJECT(subProxy)
          << vtkClientServerStream::End;
        this->ExecuteStream(stream);
        }
      stream << vtkClientServerStream::Invoke
        << VTKOBJECT(this)
        << "SetActiveRepresentation"
        << "Surface"
        << vtkClientServerStream::End;
      this->ExecuteStream(stream);
      }
    }
*/
}

//----------------------------------------------------------------------------
void vtkSMModelRepresentationProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
