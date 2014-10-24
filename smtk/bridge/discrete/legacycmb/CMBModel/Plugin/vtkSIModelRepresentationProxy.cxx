//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkSIModelRepresentationProxy.h"

#include "vtkClientServerInterpreter.h"
#include "vtkClientServerStream.h"
#include "vtkCollection.h"
#include "vtkCommand.h"
#include "vtkCubeAxesRepresentation.h"
#include "vtkObjectFactory.h"
#include "vtkProcessModule.h"
#include "vtkPVCompositeRepresentation.h"
#include "vtkPVXMLElement.h"
#include "vtkSelectionRepresentation.h"

vtkStandardNewMacro(vtkSIModelRepresentationProxy);
//----------------------------------------------------------------------------
vtkSIModelRepresentationProxy::vtkSIModelRepresentationProxy()
{
  //this->Representation = SURFACE;
}

//----------------------------------------------------------------------------
vtkSIModelRepresentationProxy::~vtkSIModelRepresentationProxy()
{
}

//----------------------------------------------------------------------------
void vtkSIModelRepresentationProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
