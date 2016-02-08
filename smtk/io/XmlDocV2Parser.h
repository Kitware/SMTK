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
#include "smtk/common/View.h"

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
      virtual void processFileItem(pugi::xml_node &node,
                           smtk::attribute::FileItemPtr item);
      virtual void processModelInfo(pugi::xml_node &root);
      virtual void processModelEntityItem(pugi::xml_node &node,
                                          smtk::attribute::ModelEntityItemPtr item);
      virtual void processMeshSelectionItem(pugi::xml_node &node,
                          attribute::MeshSelectionItemPtr idef);
      virtual void processMeshSelectionDef(pugi::xml_node &node,
                            smtk::attribute::MeshSelectionItemDefinitionPtr idef);
      virtual void processMeshEntityItem(pugi::xml_node &node,
                          attribute::MeshItemPtr item);
      virtual void processMeshEntityDef(pugi::xml_node &node,
                            smtk::attribute::MeshItemDefinitionPtr idef);
      virtual void processStringDef(pugi::xml_node &node,
                                    smtk::attribute::StringItemDefinitionPtr idef);
      virtual void processViews(pugi::xml_node &root);
      virtual void processViewComponent(smtk::common::View::Component &comp,
                                        pugi::xml_node &node, bool isTopComp);
      
      virtual smtk::common::UUID getAttributeID(pugi::xml_node &attNode);
    private:

    };
  }
}


#endif /* __smtk_io_XmlDocV2Parser_h */
