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
// .NAME XmlV1StringWriter.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_XmlV1StringWriter_h
#define __slctk_attribute_XmlV1StringWriter_h
#include "AttributeExports.h"
#include "attribute/PublicPointerDefs.h"
#include "attribute/Manager.h"
#include <string>
#include <sstream>
#include <vector>
#include "pugixml-1.2/src/pugixml.hpp"

namespace slctk
{
  namespace attribute
  {
    class SLCTKATTRIBUTE_EXPORT XmlV1StringWriter
    {
    public:
      XmlV1StringWriter(const slctk::attribute::Manager &manager);
      virtual ~XmlV1StringWriter();
      std::string convertToString();
      std::string errorStatus() const
      {return this->m_errorStatus.str();}
    protected:
      void processAttributeInformation();
      void processSections();
      void processModelInfo();

      void processDefinition(pugi::xml_node &definitions,
                             pugi::xml_node &attributes,
                             slctk::AttributeDefinitionPtr def);
      void processAttribute(pugi::xml_node &attributes,
                            slctk::AttributePtr att);
      void processItem(pugi::xml_node &node, 
                       slctk::AttributeItemPtr item);
      void processItemDefinition(pugi::xml_node &node, 
                                 slctk::AttributeItemDefinitionPtr idef);
      void processAttributeRefItem(pugi::xml_node &node, 
                                   slctk::AttributeRefItemPtr item);
      void processAttributeRefDef(pugi::xml_node &node,
                                  slctk::AttributeRefItemDefinitionPtr idef);
      void processDoubleItem(pugi::xml_node &node, 
                             slctk::DoubleItemPtr item);
      void processDoubleDef(pugi::xml_node &node,
                            slctk::DoubleItemDefinitionPtr idef);
      void processDirectoryItem(pugi::xml_node &node, 
                                slctk::DirectoryItemPtr item);
      void processDirectoryDef(pugi::xml_node &node,
                               slctk::DirectoryItemDefinitionPtr idef);
      void processFileItem(pugi::xml_node &node, 
                           slctk::FileItemPtr item);
      void processFileDef(pugi::xml_node &node,
                          slctk::FileItemDefinitionPtr idef);
      void processGroupItem(pugi::xml_node &node, 
                             slctk::GroupItemPtr item);
      void processGroupDef(pugi::xml_node &node,
                           slctk::GroupItemDefinitionPtr idef);
      void processIntItem(pugi::xml_node &node, 
                          slctk::IntItemPtr item);
      void processIntDef(pugi::xml_node &node,
                         slctk::IntItemDefinitionPtr idef);
      void processStringItem(pugi::xml_node &node, 
                             slctk::StringItemPtr item);
      void processStringDef(pugi::xml_node &node,
                            slctk::StringItemDefinitionPtr idef);
      void processValueItem(pugi::xml_node &node, 
                             slctk::ValueItemPtr item);
      void processValueDef(pugi::xml_node &node,
                           slctk::ValueItemDefinitionPtr idef);

      void processAttributeSection(pugi::xml_node &node,
                                   slctk::AttributeSectionPtr sec);

      void processInstancedSection(pugi::xml_node &node,
                                  slctk::InstancedSectionPtr sec);

      void processModelEntitySection(pugi::xml_node &node,
                                     slctk::ModelEntitySectionPtr sec);

      void processSimpleExpressionSection(pugi::xml_node &node,
                                          slctk::SimpleExpressionSectionPtr sec);

      void processGroupSection(pugi::xml_node &node,
                               slctk::GroupSectionPtr sec);

      void processBasicSection(pugi::xml_node &node,
                               slctk::SectionPtr sec);

      std::string encodeModelEntityMask(unsigned long m);

      const slctk::attribute::Manager &m_manager;
      pugi::xml_document m_doc;
      pugi::xml_node m_root;
      std::stringstream m_errorStatus;
    private:
      
    };
  };
};


#endif /* __slctk_attribute_XmlV1StringWriter_h */
