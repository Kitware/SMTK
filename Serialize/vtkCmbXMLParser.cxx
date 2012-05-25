//=============================================================================
//   This file is part of VTKEdge. See vtkedge.org for more information.
//
//   Copyright (c) 2010 Kitware, Inc.
//
//   VTKEdge may be used under the terms of the BSD License
//   Please see the file Copyright.txt in the root directory of
//   VTKEdge for further information.
//
//   Alternatively, you may see: 
//
//   http://www.vtkedge.org/vtkedge/project/license.html
//
//
//   For custom extensions, consulting services, or training for
//   this or any other Kitware supported open source project, please
//   contact Kitware at sales@kitware.com.
//
//
//=============================================================================
#include "vtkCmbXMLParser.h"
#include "vtkObjectFactory.h"
#include "vtkXMLElement.h"
#include "vtksys/ios/sstream"

vtkCxxRevisionMacro(vtkCmbXMLParser, "1774");
vtkStandardNewMacro(vtkCmbXMLParser);

//----------------------------------------------------------------------------
vtkCmbXMLParser::vtkCmbXMLParser()
{
  this->FileName = 0;
  this->InputString = 0;
  this->NumberOfOpenElements = 0;
  this->OpenElementsSize = 10;
  this->OpenElements = new vtkXMLElement*[this->OpenElementsSize];
  this->ElementIdIndex = 0;
  this->RootElement = 0;
}

//----------------------------------------------------------------------------
vtkCmbXMLParser::~vtkCmbXMLParser()
{
  unsigned int i;
  for(i=0;i < this->NumberOfOpenElements;++i)
    {
    this->OpenElements[i]->Delete();
    }
  delete [] this->OpenElements;
  if(this->RootElement)
    {
    this->RootElement->Delete();
    }
  this->SetFileName(0);
}

//----------------------------------------------------------------------------
void vtkCmbXMLParser::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << (this->FileName? this->FileName : "(none)")
     << "\n";
}

//----------------------------------------------------------------------------
void vtkCmbXMLParser::StartElement(const char* name, const char** atts)
{
  vtkXMLElement* element = vtkXMLElement::New();
  element->SetName(name);
  element->ReadXMLAttributes(atts);
  const char* id = element->GetAttribute("id");
  if(id)
    {
    element->SetId(id);
    }
  else
    {
    vtksys_ios::ostringstream idstr;
    idstr << this->ElementIdIndex++ << ends;
    element->SetId(idstr.str().c_str());
    }
  this->PushOpenElement(element);
}

//----------------------------------------------------------------------------
void vtkCmbXMLParser::EndElement(const char* vtkNotUsed(name))
{
  vtkXMLElement* finished = this->PopOpenElement();
  unsigned int numOpen = this->NumberOfOpenElements;
  if(numOpen > 0)
    {
    this->OpenElements[numOpen-1]->AddNestedElement(finished);
    finished->Delete();
    }
  else
    {
    this->RootElement = finished;
    }
}

//----------------------------------------------------------------------------
void vtkCmbXMLParser::CharacterDataHandler(const char* data, int length)
{
  unsigned int numOpen = this->NumberOfOpenElements;
  if(numOpen > 0)
    {
    this->OpenElements[numOpen-1]->AddCharacterData(data, length);
    }
}

//----------------------------------------------------------------------------
void vtkCmbXMLParser::PushOpenElement(vtkXMLElement* element)
{
  if(this->NumberOfOpenElements == this->OpenElementsSize)
    {
    unsigned int newSize = this->OpenElementsSize*2;
    vtkXMLElement** newOpenElements = new vtkXMLElement*[newSize];
    unsigned int i;
    for(i=0; i < this->NumberOfOpenElements;++i)
      {
      newOpenElements[i] = this->OpenElements[i];
      }
    delete [] this->OpenElements;
    this->OpenElements = newOpenElements;
    this->OpenElementsSize = newSize;
    }

  unsigned int pos = this->NumberOfOpenElements++;
  this->OpenElements[pos] = element;
}

//----------------------------------------------------------------------------
vtkXMLElement* vtkCmbXMLParser::PopOpenElement()
{
  if(this->NumberOfOpenElements > 0)
    {
    --this->NumberOfOpenElements;
    return this->OpenElements[this->NumberOfOpenElements];
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkCmbXMLParser::PrintXML(ostream& os)
{
  this->RootElement->PrintXML(os, vtkIndent());
}

//----------------------------------------------------------------------------
int vtkCmbXMLParser::ParseXML()
{
  if (this->RootElement)
    {
    this->RootElement->Delete();
    this->RootElement = 0;
    }
  return this->Superclass::ParseXML();
}

//----------------------------------------------------------------------------
vtkXMLElement* vtkCmbXMLParser::GetRootElement()
{
  return this->RootElement;
}
