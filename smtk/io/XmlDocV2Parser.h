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

#ifndef smtk_io_XmlDocV2Parser_h
#define smtk_io_XmlDocV2Parser_h

#include "smtk/io/XmlDocV1Parser.h"
#include "smtk/view/Configuration.h"

namespace smtk
{
namespace io
{
class SMTKCORE_EXPORT XmlDocV2Parser : public XmlDocV1Parser
{
public:
  XmlDocV2Parser(smtk::attribute::ResourcePtr resource, smtk::io::Logger& logger);
  ~XmlDocV2Parser() override;
  using XmlDocV1Parser::process;
  void process(pugi::xml_document& doc) override;

  static bool canParse(pugi::xml_node& node);
  static bool canParse(pugi::xml_document& doc);
  static pugi::xml_node getRootNode(pugi::xml_document& doc);

protected:
  void processDefinitionAtts(pugi::xml_node& defNode, smtk::attribute::DefinitionPtr& def) override;
  void processDirectoryItem(pugi::xml_node& node, smtk::attribute::DirectoryItemPtr item) override;
  void processDirectoryDef(pugi::xml_node& node, smtk::attribute::DirectoryItemDefinitionPtr idef)
    override;
  void processFileItem(pugi::xml_node& node, smtk::attribute::FileItemPtr item) override;
  void processFileDef(pugi::xml_node& node, smtk::attribute::FileItemDefinitionPtr idef) override;
  void processModelEntityItem(pugi::xml_node& node, smtk::attribute::ComponentItemPtr item)
    override;
  void processMeshEntityDef(pugi::xml_node& node, smtk::attribute::ComponentItemDefinitionPtr idef)
    override;
  void processStringDefAtts(
    pugi::xml_node& node,
    const smtk::attribute::StringItemDefinitionPtr& idef) override;
  void processViews(pugi::xml_node& root) override;
  void processViewComponent(
    smtk::view::Configuration::Component& comp,
    pugi::xml_node& node,
    bool isTopComp);

  smtk::common::UUID getAttributeID(pugi::xml_node& attNode) override;

private:
};
} // namespace io
} // namespace smtk

#endif /* smtk_io_XmlDocV2Parser_h */
