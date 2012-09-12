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
      static void convertStringToXML(std::string &str);
      static std::string encodeModelEntityMask(unsigned long m);
    protected:
      void processDefinitions();
      void processInstances();
      void processSections();
      void processModelInfo();

      void processDefinition(pugi::xml_node &node,
                             slctk::AttributeDefinitionPtr def);
      void processItemDefinition(pugi::xml_node &node, 
                                 AttributeItemDefinitionPtr idef);
      void processAttributeRefDef(pugi::xml_node &node,
                                  AttributeRefItemDefinitionPtr idef);
      void processDoubleDef(pugi::xml_node &node,
                            DoubleItemDefinitionPtr idef);
      void processDirectoryDef(pugi::xml_node &node,
                               DirectoryItemDefinitionPtr idef);
      void processFileDef(pugi::xml_node &node,
                          FileItemDefinitionPtr idef);
      void processGroupDef(pugi::xml_node &node,
                           GroupItemDefinitionPtr idef);
      void processIntDef(pugi::xml_node &node,
                         IntItemDefinitionPtr idef);
      void processStringDef(pugi::xml_node &node,
                            StringItemDefinitionPtr idef);
      void processValueDef(pugi::xml_node &node,
                           ValueItemDefinitionPtr idef);

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

      const slctk::attribute::Manager &m_manager;
      pugi::xml_document m_doc;
      pugi::xml_node m_root;
    private:
      
    };
  };
};


#endif /* __slctk_attribute_XmlV1StringWriter_h */
