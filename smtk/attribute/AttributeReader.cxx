/*=========================================================================
Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
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


#include "smtk/attribute/AttributeReader.h"
#include "smtk/attribute/XmlDocV1Parser.h"
#define PUGIXML_HEADER_ONLY
#include "pugixml-1.2/src/pugixml.cpp"

#include <iostream>

using namespace smtk::attribute; 
using namespace pugi;

//----------------------------------------------------------------------------
inline std::string Internal_parseXmlDoc(Manager &manager, xml_document& doc)
{
  std::string errorMessages="";
  // Lets get the version of the Attribute File Format
  xml_node root = doc.child("SMTK_AttributeManager");
  if (!root)
    {
    errorMessages = "Error: Can not find root node: SMTK_AttributeManager\n";
    return errorMessages;
    }

  xml_attribute xatt = root.attribute("Version");
  if (!xatt)
    {
    errorMessages = "Error: Can not find XML Attribute Version in node: SMTK_AttributeManager\n";
    return errorMessages;
    }
   
  int versionNum = xatt.as_int();
  if (versionNum != 1)
    {
    std::stringstream s;
    s << "Error: Unsupported Attribute Version: " << versionNum << "\n";
    errorMessages = s.str();
    return errorMessages;
    }
  XmlDocV1Parser theReader(manager);
  theReader.process(doc);
  errorMessages = theReader.errorStatus();
  if(errorMessages != "")
    {
    std::cerr << "============== AttributeReader.cxx Error Messages ==============\n";
    std::cerr << errorMessages << std::endl;
    std::cerr << "================================================================\n";
    }
  return errorMessages;
}

//----------------------------------------------------------------------------
bool AttributeReader::read(Manager &manager, const std::string &filename)
{
  // First load in the xml document
  xml_document doc;
  xml_parse_result presult = doc.load_file(filename.c_str());
  if (presult.status != status_ok)
    {
    this->m_errorMessages = presult.description();
    return true;
    }
  this->m_errorMessages = Internal_parseXmlDoc(manager, doc);
  return this->m_errorMessages != "";
}

//----------------------------------------------------------------------------
bool AttributeReader::readContents(Manager &manager, const std::string &filecontents)
{
  // First load in the xml document
  xml_document doc;
  xml_parse_result presult = doc.load(filecontents.c_str());
  if (presult.status != status_ok)
    {
    this->m_errorMessages = presult.description();
    return true;
    }
  this->m_errorMessages = Internal_parseXmlDoc(manager, doc);
  return this->m_errorMessages != "";
}

//----------------------------------------------------------------------------
