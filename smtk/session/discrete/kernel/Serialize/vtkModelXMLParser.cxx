//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkModelXMLParser.h"
#include "vtkObjectFactory.h"
#include "vtkXMLElement.h"
#include <sstream>

vtkStandardNewMacro(vtkModelXMLParser);

vtkModelXMLParser::vtkModelXMLParser()
{
  this->FileName = 0;
  this->InputString = 0;
  this->NumberOfOpenElements = 0;
  this->OpenElementsSize = 10;
  this->OpenElements = new vtkXMLElement*[this->OpenElementsSize];
  this->ElementIdIndex = 0;
  this->RootElement = 0;
}

vtkModelXMLParser::~vtkModelXMLParser()
{
  unsigned int i;
  for (i = 0; i < this->NumberOfOpenElements; ++i)
  {
    this->OpenElements[i]->Delete();
  }
  delete[] this->OpenElements;
  if (this->RootElement)
  {
    this->RootElement->Delete();
  }
  this->SetFileName(0);
}

void vtkModelXMLParser::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << (this->FileName ? this->FileName : "(none)") << "\n";
}

void vtkModelXMLParser::StartElement(const char* name, const char** atts)
{
  vtkXMLElement* element = vtkXMLElement::New();
  element->SetName(name);
  element->ReadXMLAttributes(atts);
  const char* id = element->GetAttribute("id");
  if (id)
  {
    element->SetId(id);
  }
  else
  {
    std::ostringstream idstr;
    idstr << this->ElementIdIndex++ << ends;
    element->SetId(idstr.str().c_str());
  }
  this->PushOpenElement(element);
}

void vtkModelXMLParser::EndElement(const char* vtkNotUsed(name))
{
  vtkXMLElement* finished = this->PopOpenElement();
  unsigned int numOpen = this->NumberOfOpenElements;
  if (numOpen > 0)
  {
    this->OpenElements[numOpen - 1]->AddNestedElement(finished);
    finished->Delete();
  }
  else
  {
    this->RootElement = finished;
  }
}

void vtkModelXMLParser::CharacterDataHandler(const char* data, int length)
{
  unsigned int numOpen = this->NumberOfOpenElements;
  if (numOpen > 0)
  {
    this->OpenElements[numOpen - 1]->AddCharacterData(data, length);
  }
}

void vtkModelXMLParser::PushOpenElement(vtkXMLElement* element)
{
  if (this->NumberOfOpenElements == this->OpenElementsSize)
  {
    unsigned int newSize = this->OpenElementsSize * 2;
    vtkXMLElement** newOpenElements = new vtkXMLElement*[newSize];
    unsigned int i;
    for (i = 0; i < this->NumberOfOpenElements; ++i)
    {
      newOpenElements[i] = this->OpenElements[i];
    }
    delete[] this->OpenElements;
    this->OpenElements = newOpenElements;
    this->OpenElementsSize = newSize;
  }

  unsigned int pos = this->NumberOfOpenElements++;
  this->OpenElements[pos] = element;
}

vtkXMLElement* vtkModelXMLParser::PopOpenElement()
{
  if (this->NumberOfOpenElements > 0)
  {
    --this->NumberOfOpenElements;
    return this->OpenElements[this->NumberOfOpenElements];
  }
  return 0;
}

void vtkModelXMLParser::PrintXML(ostream& os)
{
  this->RootElement->PrintXML(os, vtkIndent());
}

int vtkModelXMLParser::ParseXML()
{
  if (this->RootElement)
  {
    this->RootElement->Delete();
    this->RootElement = 0;
  }
  return this->Superclass::ParseXML();
}

vtkXMLElement* vtkModelXMLParser::GetRootElement()
{
  return this->RootElement;
}
