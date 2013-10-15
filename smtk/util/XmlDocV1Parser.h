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
// .NAME XmlDocV1Parser.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_util_XmlDocV1Parser_h
#define __smtk_util_XmlDocV1Parser_h
#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Manager.h"
#include "smtk/util/Logger.h"
#include <utility>
#include <string>
#include <vector>
#include "pugixml-1.2/src/pugixml.hpp"


namespace smtk
{
  typedef std::pair<smtk::ValueItemDefinitionPtr, std::string> ItemExpressionDefInfo;
  typedef std::pair<smtk::AttributeRefItemDefinitionPtr, std::string> AttRefDefInfo;
  namespace util
  {
    // Class for recording problems
    class Logger;

    // Helper struct needed for dealing with attribute references
    struct AttRefInfo
    {
      smtk::AttributeRefItemPtr item;
      int pos;
      std::string attName;
    };

    struct ItemExpressionInfo
    {
      smtk::ValueItemPtr item;
      int pos;
      std::string expName;
    };

    class SMTKCORE_EXPORT XmlDocV1Parser
    {
    public:
      XmlDocV1Parser(smtk::attribute::Manager &manager);
      virtual ~XmlDocV1Parser();
      void process(pugi::xml_document &doc);
      static void convertStringToXML(std::string &str);
      const smtk::util::Logger &messageLog() const
      {return this->m_logger;}

    protected:
      void processAttributeInformation(pugi::xml_node &root);
      void processViews(pugi::xml_node &root);
      void processModelInfo(pugi::xml_node &root);

      void processDefinition(pugi::xml_node &defNode);
      void processAttribute(pugi::xml_node &attNode);
      void processItem(pugi::xml_node &node,
                       smtk::AttributeItemPtr item);
      void processItemDef(pugi::xml_node &node,
                          smtk::AttributeItemDefinitionPtr idef);
      void processRefItem(pugi::xml_node &node,
                                   smtk::AttributeRefItemPtr item);
      void processRefDef(pugi::xml_node &node,
                         smtk::AttributeRefItemDefinitionPtr idef);
      void processDoubleItem(pugi::xml_node &node,
                             smtk::DoubleItemPtr item);
      void processDoubleDef(pugi::xml_node &node,
                            smtk::DoubleItemDefinitionPtr idef);
      void processDirectoryItem(pugi::xml_node &node,
                                smtk::DirectoryItemPtr item);
      void processDirectoryDef(pugi::xml_node &node,
                               smtk::DirectoryItemDefinitionPtr idef);
      void processFileItem(pugi::xml_node &node,
                           smtk::FileItemPtr item);
      void processFileDef(pugi::xml_node &node,
                          smtk::FileItemDefinitionPtr idef);
      void processGroupItem(pugi::xml_node &node,
                             smtk::GroupItemPtr item);
      void processGroupDef(pugi::xml_node &node,
                           smtk::GroupItemDefinitionPtr idef);
      void processIntItem(pugi::xml_node &node,
                          smtk::IntItemPtr item);
      void processIntDef(pugi::xml_node &node,
                         smtk::IntItemDefinitionPtr idef);
      void processStringItem(pugi::xml_node &node,
                             smtk::StringItemPtr item);
      void processStringDef(pugi::xml_node &node,
                            smtk::StringItemDefinitionPtr idef);
      void processValueItem(pugi::xml_node &node,
                             smtk::ValueItemPtr item);
      void processValueDef(pugi::xml_node &node,
                           smtk::ValueItemDefinitionPtr idef);

      void processAttributeView(pugi::xml_node &node,
                                smtk::view::AttributePtr v);

      void processInstancedView(pugi::xml_node &node,
                                smtk::view::InstancedPtr v);

      void processModelEntityView(pugi::xml_node &node,
                                  smtk::view::ModelEntityPtr v);

      void processSimpleExpressionView(pugi::xml_node &node,
                                       smtk::view::SimpleExpressionPtr v);

      void processGroupView(pugi::xml_node &node,
                            smtk::view::GroupPtr v);

      void processBasicView(pugi::xml_node &node,
                            smtk::view::BasePtr v);

      bool getColor(pugi::xml_node &node, double color[3],
                    const std::string &colorName);

      unsigned long  decodeModelEntityMask(const std::string &s);
      static int decodeColorInfo(const std::string &s, double *color);

      smtk::attribute::Manager &m_manager;
      std::vector<ItemExpressionDefInfo> m_itemExpressionDefInfo;
      std::vector<AttRefDefInfo> m_attRefDefInfo;
      std::vector<ItemExpressionInfo> m_itemExpressionInfo;
      std::vector<AttRefInfo> m_attRefInfo;
      std::string m_defaultCategory;
      pugi::xml_node m_root; // Dummy for now!
      smtk::util::Logger m_logger;
    private:

    };
  }
}


#endif /* __smtk_util_XmlDocV1Parser_h */
