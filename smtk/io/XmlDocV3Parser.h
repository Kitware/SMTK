//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME XmlDocV3Parser.h -
// .SECTION Description
// .SECTION See Also

#ifndef smtk_io_XmlDocV3Parser_h
#define smtk_io_XmlDocV3Parser_h

#include "smtk/io/XmlDocV2Parser.h"

#include <functional>

namespace smtk
{
namespace io
{
class SMTKCORE_EXPORT XmlDocV3Parser : public XmlDocV2Parser
{
public:
  XmlDocV3Parser(smtk::attribute::ResourcePtr resource, smtk::io::Logger& logger);
  ~XmlDocV3Parser() override;
  using XmlDocV1Parser::process;
  void process(pugi::xml_document& doc) override;
  void process(
    pugi::xml_node& rootNode,
    std::map<std::string, std::map<std::string, smtk::io::TemplateInfo>>& globalTemplateMap)
    override;

  static bool canParse(pugi::xml_node& node);
  static bool canParse(pugi::xml_document& doc);
  static pugi::xml_node getRootNode(pugi::xml_document& doc);

protected:
  void processDefinitionInformation(pugi::xml_node& rootNode) override;
  void processDefinitionAtts(pugi::xml_node& node, smtk::attribute::DefinitionPtr& def) override;
  void processDefinitionChildNode(pugi::xml_node& node, smtk::attribute::DefinitionPtr& def)
    override;
  void processDefCategoryInfoNode(pugi::xml_node& node, smtk::attribute::DefinitionPtr& def);
  void processConfigurations(pugi::xml_node& rootNode) override;
  void processExclusion(pugi::xml_node& excludeNode);
  void processPrerequisite(pugi::xml_node& depNode);
  void processAssociationDef(pugi::xml_node& node, smtk::attribute::DefinitionPtr def) override;

  void processDateTimeItem(pugi::xml_node& node, smtk::attribute::DateTimeItemPtr item) override;
  void processDateTimeDef(pugi::xml_node& node, smtk::attribute::DateTimeItemDefinitionPtr idef)
    override;

  void processReferenceItem(pugi::xml_node& node, smtk::attribute::ReferenceItemPtr item) override;
  void processReferenceDef(
    pugi::xml_node& node,
    smtk::attribute::ReferenceItemDefinitionPtr idef,
    const std::string& labelsElement = "ReferenceLabels") override;

  void processResourceItem(pugi::xml_node& node, smtk::attribute::ResourceItemPtr item) override;
  void processResourceDef(pugi::xml_node& node, smtk::attribute::ResourceItemDefinitionPtr idef)
    override;

  void processComponentItem(pugi::xml_node& node, smtk::attribute::ComponentItemPtr item) override;
  void processComponentDef(pugi::xml_node& node, smtk::attribute::ComponentItemDefinitionPtr idef)
    override;
  void getUniqueRoles(pugi::xml_node& rootNode);

private:
};
} // namespace io
} // namespace smtk

#endif /* smtk_io_XmlDocV3Parser_h */
