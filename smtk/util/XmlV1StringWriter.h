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

#ifndef __smtk_attribute_XmlV1StringWriter_h
#define __smtk_attribute_XmlV1StringWriter_h
#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Manager.h"
#include "smtk/util/Logger.h"
#include <string>
#include <sstream>
#include <vector>
#include "pugixml-1.2/src/pugixml.hpp"

namespace smtk
{
  namespace util
  {
    class SMTKCORE_EXPORT XmlV1StringWriter
    {
    public:
      XmlV1StringWriter(const smtk::attribute::Manager &manager);
      virtual ~XmlV1StringWriter();
      std::string convertToString(smtk::util::Logger &logger);
      const smtk::util::Logger &messageLog() const
      {return this->m_logger;}
    protected:
      void processAttributeInformation();
      void processViews();
      void processModelInfo();

      void processDefinition(pugi::xml_node &definitions,
                             pugi::xml_node &attributes,
                             smtk::attribute::DefinitionPtr def);
      void processAttribute(pugi::xml_node &attributes,
                            smtk::attribute::AttributePtr att);
      void processItem(pugi::xml_node &node,
                       smtk::attribute::ItemPtr item);
      void processItemDefinition(pugi::xml_node &node,
                                 smtk::attribute::ItemDefinitionPtr idef);
      void processRefItem(pugi::xml_node &node,
                                   smtk::attribute::RefItemPtr item);
      void processRefDef(pugi::xml_node &node,
                         smtk::attribute::RefItemDefinitionPtr idef);
      void processDoubleItem(pugi::xml_node &node,
                             smtk::attribute::DoubleItemPtr item);
      void processDoubleDef(pugi::xml_node &node,
                            smtk::attribute::DoubleItemDefinitionPtr idef);
      void processDirectoryItem(pugi::xml_node &node,
                                smtk::attribute::DirectoryItemPtr item);
      void processDirectoryDef(pugi::xml_node &node,
                               smtk::attribute::DirectoryItemDefinitionPtr idef);
      void processFileItem(pugi::xml_node &node,
                           smtk::attribute::FileItemPtr item);
      void processFileDef(pugi::xml_node &node,
                          smtk::attribute::FileItemDefinitionPtr idef);
      void processGroupItem(pugi::xml_node &node,
                             smtk::attribute::GroupItemPtr item);
      void processGroupDef(pugi::xml_node &node,
                           smtk::attribute::GroupItemDefinitionPtr idef);
      void processIntItem(pugi::xml_node &node,
                          smtk::attribute::IntItemPtr item);
      void processIntDef(pugi::xml_node &node,
                         smtk::attribute::IntItemDefinitionPtr idef);
      void processStringItem(pugi::xml_node &node,
                             smtk::attribute::StringItemPtr item);
      void processStringDef(pugi::xml_node &node,
                            smtk::attribute::StringItemDefinitionPtr idef);
      void processValueItem(pugi::xml_node &node,
                             smtk::attribute::ValueItemPtr item);
      void processValueDef(pugi::xml_node &node,
                           smtk::attribute::ValueItemDefinitionPtr idef);

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

      static std::string encodeModelEntityMask(unsigned long m);
      static std::string encodeColor(const double *color);

      const smtk::attribute::Manager &m_manager;
      pugi::xml_document m_doc;
      pugi::xml_node m_root;
      smtk::util::Logger  m_logger;
    private:

    };
  }
}


#endif /* __smtk_util_XmlV1StringWriter_h */
