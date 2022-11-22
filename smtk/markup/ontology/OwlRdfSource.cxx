//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/ontology/OwlRdfSource.h"

#include "smtk/io/Logger.h"

#define PUGIXML_HEADER_ONLY
// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "thirdparty/pugixml/src/pugixml.cpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// Define the following to get summary ontology information printed.
#undef SMTK_DEBUG_ONTOLOGY

namespace smtk
{
namespace markup
{
namespace ontology
{

class OwlRdfSource::Internal
{
public:
  Internal(OwlRdfSource* self)
    : m_self(self)
  {
  }

  std::string parseClass(const pugi::xml_node& classNode)
  {
    static int classId = 0;
    std::string label = Internal::labelOfNode(classNode, classId);
    std::string url = classNode.attribute("rdf:about").as_string(label.c_str());
    std::string inherits = Internal::inheritsNode(classNode);
    Identifier node{ label, url, inherits, "", {} };
    m_self->m_classes.push_back(node);

    // If it is a union of other classes, record the URLs in the rdf:about attributes:
    // <owl:unionOf rdf:parseType="Collection">
    //   <rdf:Description rdf:about="xxx"/>
    //   ...
    // </owl:unionOf>
    auto collNodes = classNode.select_nodes(
      "./owl:unionOf[@rdf:parseType='Collection']/rdf:Description[@rdf:about]");
    if (collNodes.empty())
    {
      collNodes = classNode.select_nodes("./rdfs:subClassOf[@rdf:resource]");
    }
    for (const auto& node : collNodes)
    {
      std::string base = node.node().attribute("rdf:about").value();
      if (base.empty())
      {
        base = node.node().attribute("rdf:resource").value();
      }
      m_self->m_classes.back().collection.insert(base);
    }
    auto descNodes = classNode.select_nodes("./obo:IAO_0000115");
    if (descNodes.empty())
    {
      descNodes = classNode.select_nodes("./rdfs:comment");
    }
    for (const auto& node : descNodes)
    {
      m_self->m_classes.back().description += node.node().text().get();
    }
    return url;
  }

  std::string parseObjectProperty(const pugi::xml_node& arcNode)
  {
    static int objectPropertyId = 0;
    std::string label = Internal::labelOfNode(arcNode, objectPropertyId);
    std::string url = arcNode.attribute("rdf:about").as_string(label.c_str());

    // If the ObjectProperty has a domain, parse it:
    auto domainNode = arcNode.select_node("./rdfs:domain");
    // std::string domainUrl = domainNode.node().find_attribute([](const pugi::xml_attribute& att) { return std::string(att.name()) == "rdf:resource"; }).value();
    std::string domainUrl = domainNode.node().attribute("rdf:resource").as_string("");
    if (domainUrl.empty())
    {
      auto domainClass = domainNode.node().select_node("./owl:Class").node();
      if (!domainClass.empty())
      {
        domainUrl = this->parseClass(domainClass);
      }
    }

    // If the ObjectProperty has a range, parse it:
    auto rangeNode = arcNode.select_node("./rdfs:range");
    std::string rangeUrl = rangeNode.node()
                             .find_attribute([](const pugi::xml_attribute& att) {
                               return std::string(att.name()) == "rdf:resource";
                             })
                             .value();
    if (rangeUrl.empty())
    {
      auto rangeClass = rangeNode.node().select_node("./owl:Class").node();
      if (!rangeClass.empty())
      {
        rangeUrl = this->parseClass(rangeClass);
      }
    }

    Relation arc{ url, label, domainUrl, rangeUrl, 0 };
    m_self->m_relations.push_back(arc);
    return url;
  }

  std::string parseAnnotationProperty(const pugi::xml_node& propNode)
  {
    (void)propNode;
    return std::string();
  }

  static std::string labelOfNode(const pugi::xml_node& node, int& number)
  {
    std::string label;
    auto labelSel = node.select_node("./rdfs:label");
    if (!labelSel.node().empty())
    {
      // TODO: Handle multiple labels. This gets tricky because we assume
      //       URLs and names are 1-to-1 elsewhere.
      label = labelSel.node().text().get();
    }
    else
    {
      std::ostringstream labelStr;
      labelStr << "anonymous class " << number++;
      label = labelStr.str();
    }
    return label;
  }

  static std::string inheritsNode(const pugi::xml_node& node)
  {
    std::string inherits;
    auto subclassSel = node.select_node("./rdfs:subClassOf[@rdf:resource]");
    if (!subclassSel.node().empty())
    {
      inherits = subclassSel.node().attribute("rdf:resource").as_string("");
    }
    return inherits;
  }

  OwlRdfSource* m_self{ nullptr };
  pugi::xml_document m_doc;
};

OwlRdfSource::OwlRdfSource(const std::string& xml, const std::string& url, const std::string& name)
  : Source(url, name)
{
  m_p = new Internal(this);
  pugi::xml_parse_result presult = m_p->m_doc.load_string(xml.c_str());
  if (presult.status != pugi::status_ok)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), presult.description());
    return;
  }

  this->parse();

  std::sort(m_classes.begin(), m_classes.end());
}

void OwlRdfSource::parse()
{
  pugi::xml_node root = m_p->m_doc.child("rdf:RDF");
  if (!root)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "No root node in OWL XML.");
    return;
  }
#ifdef SMTK_DEBUG_ONTOLOGY
  std::map<std::string, int> nodeNames;
#endif
  for (const auto& node : root)
  {
    std::string nodeName = node.name();
#ifdef SMTK_DEBUG_ONTOLOGY
    ++nodeNames[nodeName];
#endif
    if (nodeName == "owl:Class")
    {
      m_p->parseClass(node);
    }
    else if (nodeName == "owl:AnnotationProperty")
    {
      m_p->parseAnnotationProperty(node);
    }
    else if (nodeName == "owl:ObjectProperty")
    {
      m_p->parseObjectProperty(node);
    }
  }
#ifdef SMTK_DEBUG_ONTOLOGY
  // For debugging, print out a summary of the XML tag names encountered:
  for (const auto& name : nodeNames)
  {
    std::cout << name.first << " " << name.second << "\n";
  }
#endif
}

} // namespace ontology
} // namespace markup
} // namespace smtk
