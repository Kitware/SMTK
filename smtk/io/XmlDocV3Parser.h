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

#ifndef __smtk_io_XmlDocV3Parser_h
#define __smtk_io_XmlDocV3Parser_h

#include "smtk/io/XmlDocV2Parser.h"

namespace smtk
{
namespace io
{
class SMTKCORE_EXPORT XmlDocV3Parser : public XmlDocV2Parser
{
public:
  XmlDocV3Parser(smtk::attribute::System& system);
  virtual ~XmlDocV3Parser();
  virtual void process(pugi::xml_document& doc);
  virtual void process(pugi::xml_node& rootNode) { XmlDocV2Parser::process(rootNode); }

  static bool canParse(pugi::xml_node& node);
  static bool canParse(pugi::xml_document& doc);
  static pugi::xml_node getRootNode(pugi::xml_document& doc);

protected:
  virtual void processDateTimeItem(pugi::xml_node& node, smtk::attribute::DateTimeItemPtr item);
  virtual void processDateTimeDef(
    pugi::xml_node& node, smtk::attribute::DateTimeItemDefinitionPtr idef);

private:
};
}
}

#endif /* __smtk_io_XmlDocV3Parser_h */
