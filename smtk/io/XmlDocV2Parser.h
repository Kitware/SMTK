//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME XmlDocV2Parser.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_io_XmlDocV2Parser_h
#define __smtk_io_XmlDocV2Parser_h

#include "smtk/io/XmlDocV1Parser.h"

namespace smtk
{
  namespace io
  {
    class SMTKCORE_EXPORT XmlDocV2Parser : public XmlDocV1Parser
    {
    public:
      XmlDocV2Parser(smtk::attribute::System &system);
      virtual ~XmlDocV2Parser();
      virtual void process(pugi::xml_document &doc);
      virtual void process(pugi::xml_node &rootNode)
      { XmlDocV1Parser::process(rootNode); }

      static bool canParse(pugi::xml_node &node);
      static bool canParse(pugi::xml_document &doc);
      static pugi::xml_node getRootNode(pugi::xml_document &doc);

    protected:
      virtual void processModelInfo(pugi::xml_node &root);
      virtual void processModelEntityItem(pugi::xml_node &node,
                                          smtk::attribute::ModelEntityItemPtr item);
      virtual void processMeshEntityItem(pugi::xml_node &node,
                          attribute::MeshEntityItemPtr idef);
      virtual void processMeshEntityDef(pugi::xml_node &node,
                            smtk::attribute::MeshEntityItemDefinitionPtr idef);
      virtual smtk::common::UUID getAttributeID(pugi::xml_node &attNode);
    private:

    };
  }
}


#endif /* __smtk_io_XmlDocV2Parser_h */
