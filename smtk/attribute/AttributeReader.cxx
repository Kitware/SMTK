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

using namespace smtk::attribute; 
using namespace pugi;
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
  // Lets get the version of the Attribute File Format
  xml_node root = doc.child("SMTK_AttributeManager");
  if (!root)
    {
    this->m_errorMessages = "Error: Can not find root node: SMTK_AttributeManager\n";
    return true;
    }

  xml_attribute xatt = root.attribute("Version");
  if (!xatt)
    {
    this->m_errorMessages = "Error: Can not find XML Attribute Version in node: SMTK_AttributeManager\n";
    return true;
    }
   
  int versionNum = xatt.as_int();
  if (versionNum != 1)
    {
    std::stringstream s;
    s << "Error: Unsupported Attribute Version: " << versionNum << "\n";
    this->m_errorMessages = s.str();
    return true;
    }
  XmlDocV1Parser theReader(manager);
  theReader.process(doc);
  this->m_errorMessages = theReader.errorStatus();
  return this->m_errorMessages != "";
}

//----------------------------------------------------------------------------
