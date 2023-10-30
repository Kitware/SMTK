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

// Class for recording problems
#include "smtk/io/Logger.h"

// For storing Template Definition Info
#include "smtk/io/TemplateInfo.h"

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
// Class to hold Parser's pugi data;
class XmlDocV1ParserInternals;

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
    std::map<std::string, std::map<std::string, smtk::io::TemplateInfo>>& globalTemplateMap);

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
  // Version 1 does not support association rules - they are supported in
  // version 4 and later
  virtual void processAssociationRules(pugi::xml_node&) {}
  // Version 1 does not support configurations - they are supported in
  // version 3 and later
  virtual void processConfigurations(pugi::xml_node&) {}

  virtual void processCategoryAtts(
    pugi::xml_node& node,
    attribute::Categories::Set& catSet,
    attribute::Categories::CombinationMode& inheritanceMode);
  void processOldStyleCategoryNode(pugi::xml_node& node, smtk::attribute::Categories::Set& catSet);
  void processItemDefCategoryInfoNode(
    pugi::xml_node& node,
    smtk::attribute::ItemDefinitionPtr idef);
  virtual void processCategoryInfoNode(
    pugi::xml_node& node,
    attribute::Categories::Set& catSet,
    attribute::Categories::CombinationMode& inheritanceMode);

  virtual void processDefinitionInformation(pugi::xml_node& defNode);
  void processDefinitionInformationChildren(pugi::xml_node& node);
  void createDefinition(pugi::xml_node& defNode);

  virtual void processDefinition(pugi::xml_node& defNode, smtk::attribute::DefinitionPtr& def);
  virtual void processDefinitionAtts(pugi::xml_node& defNode, smtk::attribute::DefinitionPtr& def);
  virtual void processDefinitionContents(
    pugi::xml_node& defNode,
    smtk::attribute::DefinitionPtr& def);
  virtual void processDefinitionChildNode(
    pugi::xml_node& node,
    smtk::attribute::DefinitionPtr& def);

  virtual void processAssociationDef(pugi::xml_node& node, smtk::attribute::DefinitionPtr def);
  virtual void processAttribute(pugi::xml_node& attNode);
  virtual void processItem(pugi::xml_node& node, smtk::attribute::ItemPtr item);

  void addDefaultCategoryIfNeeded(const smtk::attribute::ItemDefinitionPtr& idef);

  virtual void processItemDef(pugi::xml_node& node, const smtk::attribute::ItemDefinitionPtr& idef);
  virtual void processItemDefAtts(
    pugi::xml_node& node,
    const smtk::attribute::ItemDefinitionPtr& idef);
  virtual void processItemDefContents(
    pugi::xml_node& node,
    const smtk::attribute::ItemDefinitionPtr& idef);
  virtual void processItemDefChildNode(
    pugi::xml_node& node,
    const smtk::attribute::ItemDefinitionPtr& idef);

  void processRefItem(pugi::xml_node& node, smtk::attribute::ComponentItemPtr item);
  void processRefDef(pugi::xml_node& node, smtk::attribute::ComponentItemDefinitionPtr idef);
  void processDoubleItem(pugi::xml_node& node, smtk::attribute::DoubleItemPtr item);

  void processDoubleDef(pugi::xml_node& node, const smtk::attribute::DoubleItemDefinitionPtr& idef);
  void processDoubleDefContents(
    pugi::xml_node& node,
    const smtk::attribute::DoubleItemDefinitionPtr& idef);
  virtual void processDoubleDefChildNode(
    pugi::xml_node& node,
    const smtk::attribute::DoubleItemDefinitionPtr& idef);

  virtual void processDirectoryItem(pugi::xml_node& node, smtk::attribute::DirectoryItemPtr item);
  virtual void processDirectoryDef(
    pugi::xml_node& node,
    smtk::attribute::DirectoryItemDefinitionPtr idef);
  virtual void processFileItem(pugi::xml_node& node, smtk::attribute::FileItemPtr item);
  virtual void processFileDef(pugi::xml_node& node, smtk::attribute::FileItemDefinitionPtr idef);
  void processGroupItem(pugi::xml_node& node, smtk::attribute::GroupItemPtr item);
  void processGroupDef(pugi::xml_node& node, smtk::attribute::GroupItemDefinitionPtr idef);
  void processIntItem(pugi::xml_node& node, smtk::attribute::IntItemPtr item);

  void processIntDef(pugi::xml_node& node, const smtk::attribute::IntItemDefinitionPtr& idef);
  void processIntDefContents(
    pugi::xml_node& node,
    const smtk::attribute::IntItemDefinitionPtr& idef);
  virtual void processIntDefChildNode(
    pugi::xml_node& node,
    const smtk::attribute::IntItemDefinitionPtr& idef);

  void processStringItem(pugi::xml_node& node, smtk::attribute::StringItemPtr item);

  void processStringDef(pugi::xml_node& node, const smtk::attribute::StringItemDefinitionPtr& idef);
  virtual void processStringDefAtts(
    pugi::xml_node& node,
    const smtk::attribute::StringItemDefinitionPtr& idef);
  void processStringDefContents(
    pugi::xml_node& node,
    const smtk::attribute::StringItemDefinitionPtr& idef);
  virtual void processStringDefChildNode(
    pugi::xml_node& node,
    const smtk::attribute::StringItemDefinitionPtr& idef);

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

  void processValueDefAtts(
    pugi::xml_node& node,
    const smtk::attribute::ValueItemDefinitionPtr& idef);
  void processValueDefChildNode(
    pugi::xml_node& node,
    const smtk::attribute::ValueItemDefinitionPtr& idef);

  void processAttributeView(pugi::xml_node& node, smtk::view::ConfigurationPtr v);

  void processInstancedView(pugi::xml_node& node, smtk::view::ConfigurationPtr v);

  void processModelEntityView(pugi::xml_node& node, smtk::view::ConfigurationPtr v);

  void processSimpleExpressionView(pugi::xml_node& node, smtk::view::ConfigurationPtr v);

  void processGroupView(pugi::xml_node& node, smtk::view::ConfigurationPtr v);

  smtk::view::ConfigurationPtr createView(pugi::xml_node& node, const std::string& viewType);

  bool getColor(pugi::xml_node& node, double color[3], const std::string& colorName);

  virtual smtk::common::UUID getAttributeID(pugi::xml_node& attNode);

  /// For processing item definition blocks
  void processItemDefinitionBlocks(
    pugi::xml_node& rootNode,
    std::map<std::string, std::map<std::string, smtk::io::TemplateInfo>>& globalTemplateMap);

  /// For processing template definitions
  void processTemplatesDefinitions(
    pugi::xml_node& rootNode,
    std::map<std::string, std::map<std::string, smtk::io::TemplateInfo>>& globalTemplateMap);

  /// Process hints that may be on the document's root node.
  ///
  /// This is first used by XmlDocV5Parser.
  virtual void processHints(pugi::xml_node& root);

  smtk::model::BitFlags decodeModelEntityMask(const std::string& s);
  static int decodeColorInfo(const std::string& s, double* color);

  ///\brief Creates a new XML Node that represents the instantiation of a template.
  /// instanceInfo is the XML that is instantiating the template and
  /// instancedNode is the new XML node
  bool createXmlFromTemplate(pugi::xml_node& instanceInfo, pugi::xml_node& instancedNode);
  ///\brief Indicate that this node that represents an instantiated template is no longer needed
  void releaseXmlTemplate(pugi::xml_node& instancedNode);

  bool m_reportAsError{ true };
  smtk::attribute::ResourcePtr m_resource;
  std::vector<ItemExpressionInfo> m_itemExpressionInfo;
  std::vector<AttRefInfo> m_attRefInfo;
  std::string m_defaultCategory;
  smtk::io::Logger& m_logger;
  std::size_t m_includeIndex{ 0 };
  std::map<std::string, std::set<std::string>> m_activeTemplates;
  std::map<std::string, std::map<std::string, smtk::io::TemplateInfo>> m_localTemplateMap;

private:
  XmlDocV1ParserInternals* m_internals;
};
} // namespace io
} // namespace smtk

#endif /* smtk_io_XmlDocV1Parser_h */
