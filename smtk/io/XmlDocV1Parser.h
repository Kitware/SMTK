//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME XmlDocV1Parser.h -
// .SECTION Description
// .SECTION See Also

#ifndef smtk_io_XmlDocV1Parser_h
#define smtk_io_XmlDocV1Parser_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Categories.h"
#include "smtk/attribute/Resource.h"

#include "smtk/model/EntityTypeBits.h"

#include <string>
#include <utility>
#include <vector>

namespace pugi
{
class xml_attribute;
class xml_document;
class xml_node;
} // namespace pugi

namespace smtk
{
namespace io
{
// Class for recording problems
class Logger;

// Helper struct needed for dealing with attribute references
struct AttRefInfo
{
  smtk::attribute::ComponentItemPtr item;
  int pos;
  std::string attName;
};

struct ItemExpressionInfo
{
  smtk::attribute::ValueItemPtr item;
  int pos;
  std::string expName;
};

class SMTKCORE_EXPORT XmlDocV1Parser
{

  friend class ItemDefinitionsHelper;

public:
  XmlDocV1Parser(smtk::attribute::ResourcePtr resource, smtk::io::Logger& logger);
  virtual ~XmlDocV1Parser();
  virtual void process(pugi::xml_document& doc);
  virtual void process(pugi::xml_node& rootNode);
  virtual void process(
    pugi::xml_node& rootNode,
    std::map<std::string, std::map<std::string, std::string>>& globalItemBlocks);

  // This function has no implementation!
  static void convertStringToXML(std::string& str);

  void setReportDuplicateDefinitionsAsErrors(bool mode) { m_reportAsError = mode; }

  static bool canParse(pugi::xml_document& doc);
  static bool canParse(pugi::xml_node& node);
  static pugi::xml_node getRootNode(pugi::xml_document& doc);
  static void getCategories(
    pugi::xml_node& rootNode,
    std::set<std::string>& categories,
    std::string& defaultCategory);
  void setIncludeFileIndex(std::size_t index) { m_includeIndex = index; }
  static bool getCategoryComboMode(
    pugi::xml_attribute& xmlAtt,
    smtk::attribute::Categories::Set::CombinationMode& val);

protected:
  void processAttributeInformation(pugi::xml_node& root);
  virtual void processViews(pugi::xml_node& root);
  virtual void processAssociationRules(pugi::xml_node&) {}
  virtual void processCategories(
    pugi::xml_node& node,
    attribute::Categories::Set& catSet,
    attribute::Categories::CombinationMode& inheritanceMode);

  void createDefinition(pugi::xml_node& defNode);
  virtual void processDefinitionInformation(pugi::xml_node& defNode);
  virtual void processDefinition(pugi::xml_node& defNode, smtk::attribute::DefinitionPtr def);
  virtual void processAssociationDef(pugi::xml_node& node, smtk::attribute::DefinitionPtr def);
  virtual void processAttribute(pugi::xml_node& attNode);
  virtual void processItem(pugi::xml_node& node, smtk::attribute::ItemPtr item);
  virtual void processItemDef(pugi::xml_node& node, smtk::attribute::ItemDefinitionPtr idef);
  void processRefItem(pugi::xml_node& node, smtk::attribute::ComponentItemPtr item);
  void processRefDef(pugi::xml_node& node, smtk::attribute::ComponentItemDefinitionPtr idef);
  void processDoubleItem(pugi::xml_node& node, smtk::attribute::DoubleItemPtr item);
  void processDoubleDef(pugi::xml_node& node, smtk::attribute::DoubleItemDefinitionPtr idef);
  virtual void processDirectoryItem(pugi::xml_node& node, smtk::attribute::DirectoryItemPtr item);
  virtual void processDirectoryDef(
    pugi::xml_node& node,
    smtk::attribute::DirectoryItemDefinitionPtr idef);
  virtual void processFileItem(pugi::xml_node& node, smtk::attribute::FileItemPtr item);
  virtual void processFileDef(pugi::xml_node& node, smtk::attribute::FileItemDefinitionPtr idef);
  void processGroupItem(pugi::xml_node& node, smtk::attribute::GroupItemPtr item);
  void processGroupDef(pugi::xml_node& node, smtk::attribute::GroupItemDefinitionPtr idef);
  void processIntItem(pugi::xml_node& node, smtk::attribute::IntItemPtr item);
  void processIntDef(pugi::xml_node& node, smtk::attribute::IntItemDefinitionPtr idef);
  void processStringItem(pugi::xml_node& node, smtk::attribute::StringItemPtr item);
  virtual void processStringDef(
    pugi::xml_node& node,
    smtk::attribute::StringItemDefinitionPtr idef);
  virtual void processModelEntityItem(pugi::xml_node& node, smtk::attribute::ComponentItemPtr item);
  void processModelEntityDef(
    pugi::xml_node& node,
    smtk::attribute::ReferenceItemDefinitionPtr idef);
  virtual void processMeshEntityItem(pugi::xml_node& node, attribute::ComponentItemPtr item);
  virtual void processMeshEntityDef(
    pugi::xml_node& node,
    smtk::attribute::ComponentItemDefinitionPtr idef);
  virtual void processDateTimeItem(pugi::xml_node& node, smtk::attribute::DateTimeItemPtr item);
  virtual void processDateTimeDef(
    pugi::xml_node& node,
    smtk::attribute::DateTimeItemDefinitionPtr idef);
  virtual void processReferenceItem(pugi::xml_node& node, smtk::attribute::ReferenceItemPtr item);
  virtual void processReferenceDef(
    pugi::xml_node& node,
    smtk::attribute::ReferenceItemDefinitionPtr idef,
    const std::string& labelsElement = "ReferenceLabels");
  virtual void processResourceItem(pugi::xml_node& node, smtk::attribute::ResourceItemPtr item);
  virtual void processResourceDef(
    pugi::xml_node& node,
    smtk::attribute::ResourceItemDefinitionPtr idef);
  virtual void processComponentItem(pugi::xml_node& node, smtk::attribute::ComponentItemPtr item);
  virtual void processComponentDef(
    pugi::xml_node& node,
    smtk::attribute::ComponentItemDefinitionPtr idef);
  void processValueItem(pugi::xml_node& node, smtk::attribute::ValueItemPtr item);
  void processValueDef(pugi::xml_node& node, smtk::attribute::ValueItemDefinitionPtr idef);

  void processAttributeView(pugi::xml_node& node, smtk::view::ConfigurationPtr v);

  void processInstancedView(pugi::xml_node& node, smtk::view::ConfigurationPtr v);

  void processModelEntityView(pugi::xml_node& node, smtk::view::ConfigurationPtr v);

  void processSimpleExpressionView(pugi::xml_node& node, smtk::view::ConfigurationPtr v);

  void processGroupView(pugi::xml_node& node, smtk::view::ConfigurationPtr v);

  smtk::view::ConfigurationPtr createView(pugi::xml_node& node, const std::string& viewType);

  bool getColor(pugi::xml_node& node, double color[3], const std::string& colorName);

  virtual smtk::common::UUID getAttributeID(pugi::xml_node& attNode);

  // For processing item definition blocks
  void processItemDefinitionBlocks(
    pugi::xml_node& rootNode,
    std::map<std::string, std::map<std::string, std::string>>& globalItemBlocks);

  // For processing the child item definition block for attribute
  // definitions
  void processItemDefinitions(
    pugi::xml_node& itemDefs,
    smtk::attribute::DefinitionPtr& def,
    std::set<std::string>& activeBlockNames);

  /// Process hints that may be on the document's root node.
  ///
  /// This is first used by XmlDocV5Parser.
  virtual void processHints(pugi::xml_node& root);

  smtk::model::BitFlags decodeModelEntityMask(const std::string& s);
  static int decodeColorInfo(const std::string& s, double* color);
  bool m_reportAsError;
  smtk::attribute::ResourcePtr m_resource;
  std::vector<ItemExpressionInfo> m_itemExpressionInfo;
  std::vector<AttRefInfo> m_attRefInfo;
  std::string m_defaultCategory;
  smtk::io::Logger& m_logger;
  std::size_t m_includeIndex;
  std::map<std::string, std::map<std::string, pugi::xml_node>> m_itemDefintionBlocks;

private:
};
} // namespace io
} // namespace smtk

#endif /* smtk_io_XmlDocV1Parser_h */
