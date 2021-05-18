//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME XmlDocV4Parser.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_io_XmlDocV4Parser_h
#define __smtk_io_XmlDocV4Parser_h

#include "smtk/io/XmlDocV3Parser.h"

#include <functional>

namespace smtk
{
namespace io
{
class SMTKCORE_EXPORT XmlDocV4Parser : public XmlDocV3Parser
{
public:
  XmlDocV4Parser(smtk::attribute::ResourcePtr resource, smtk::io::Logger& logger);
  ~XmlDocV4Parser() override;

  void process(pugi::xml_document& doc) override;
  void process(pugi::xml_node& rootNode) override;

  static bool canParse(pugi::xml_node& node);
  static bool canParse(pugi::xml_document& doc);
  static pugi::xml_node getRootNode(pugi::xml_document& doc);

protected:
  void processDefinition(pugi::xml_node& defNode, smtk::attribute::DefinitionPtr def) override;
  void processItemDef(pugi::xml_node& node, smtk::attribute::ItemDefinitionPtr idef) override;
  void processItem(pugi::xml_node& node, smtk::attribute::ItemPtr item) override;
  void processViews(pugi::xml_node& root) override;
  void processAssociationRules(pugi::xml_node& root) override;
  void processEvaluators(pugi::xml_node& evaluatorsNode);
};
} // namespace io
} // namespace smtk

#endif /* __smtk_io_XmlDocV4Parser_h */
