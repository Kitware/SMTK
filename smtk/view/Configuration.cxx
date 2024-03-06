//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkView.cxx - Base class for SMTK views
// .SECTION Description
// .SECTION See Also

#include "smtk/view/Configuration.h"
#include <algorithm>
#include <sstream> // std::istringstream

namespace smtk
{
namespace view
{

Configuration::Component& Configuration::Component::setContents(const std::string& c)
{
  m_contents = c;
  return *this;
}

bool Configuration::Component::attributeAsBool(const std::string& attname, bool& val) const
{
  std::string s;
  if (!this->attribute(attname, s))
  {
    return false; // name doesn't exist
  }

  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  if ((s == "true") || (s == "t"))
  {
    val = true;
    return true;
  }

  if ((s == "false") || (s == "f"))
  {
    val = false;
    return true;
  }
  return false;
}

std::string Configuration::Component::attributeAsString(const std::string& attname) const
{
  std::string s;
  this->attribute(attname, s);
  return s;
}

bool Configuration::Component::attributeAsInt(const std::string& attname, int& val) const
{
  std::string s;
  if (!this->attribute(attname, s))
  {
    return false; // name doesn't exist
  }

  val = std::stoi(s);
  return true;
}

bool Configuration::Component::attributeAsDouble(const std::string& attname, double& val) const
{
  std::string s;
  if (!this->attribute(attname, s))
  {
    return false; // name doesn't exist
  }

  val = std::stod(s);
  return true;
}

bool Configuration::Component::attributeAsBool(const std::string& attname) const
{
  bool v;
  if (this->attributeAsBool(attname, v))
  {
    return v;
  }
  return false;
}

bool Configuration::Component::contentsAsInt(int& val) const
{
  std::istringstream iss(m_contents);
  iss >> val;
  return iss.good();
}

bool Configuration::Component::contentsAsVector(std::vector<double>& vec) const
{
  std::istringstream iss(m_contents);
  char c;
  double d;
  vec.clear();
  while (!iss.eof())
  {
    iss.get(c);
    if ((c == ' ') || (c == ','))
    {
      continue;
    }
    iss.putback(c);
    iss >> d;
    if (!iss.good())
    {
      return !vec.empty();
    }
    vec.push_back(d);
  }
  return !vec.empty();
}

bool Configuration::Component::attribute(const std::string& attname) const
{
  std::string dummy;
  return this->attribute(attname, dummy);
}

bool Configuration::Component::attribute(const std::string& attname, std::string& value) const
{
  std::map<std::string, std::string>::const_iterator it;
  it = m_attributes.find(attname);
  if (it == m_attributes.end())
  {
    return false;
  }
  value = it->second;
  return true;
}

Configuration::Component& Configuration::Component::setAttribute(
  const std::string& attname,
  const std::string& value)
{
  m_attributes[attname] = value;
  return *this;
}

Configuration::Component& Configuration::Component::unsetAttribute(const std::string& attname)
{
  m_attributes.erase(attname);
  return *this;
}

Configuration::Component& Configuration::Component::addChild(const std::string& childName)
{
  m_children.emplace_back(childName);
  return m_children.back();
}

int Configuration::Component::findChild(const std::string& compName) const
{
  int i, n = static_cast<int>(m_children.size());
  for (i = 0; i < n; i++)
  {
    if (m_children[i].name() == compName)
    {
      return i;
    }
  }
  return -1;
}

void Configuration::Component::copyContents(const Component& comp)
{
  m_name = comp.m_name;
  m_attributes = comp.m_attributes;
  m_contents = comp.m_contents;
  for (const Component& child : comp.m_children)
  {
    Component& myChild = this->addChild(child.name());
    myChild.copyContents(child);
  }
}

Configuration::Configuration(const std::string& myType, const std::string& myName)
  : m_name(myName)
  , m_type(myType)
  , m_details("Details")
{
}

Configuration::~Configuration() = default;

void Configuration::copyContents(const Configuration& view)
{
  m_iconName = view.m_iconName;
  m_details.copyContents(view.m_details);
}

std::string Configuration::label() const
{
  std::string l;
  if (m_details.attribute("Label", l))
  {
    return l;
  }
  return m_name;
}

static thread_local std::size_t g_indentCount = 0;

std::ostream& operator<<(std::ostream& os, const Configuration::Component& comp)
{
  g_indentCount += 2;
  std::string indent(g_indentCount, ' ');
  os << indent << "Name: " << comp.name() << "\n";
  if (!comp.attributes().empty())
  {
    os << indent << "Attributes:\n";
    for (const auto& attr : comp.attributes())
    {
      os << indent << "  " << attr.first << ": " << attr.second << "\n";
    }
  }
  if (!comp.contents().empty())
  {
    os << indent << "Content:\n" << indent << comp.contents() << "\n";
  }
  if (!comp.children().empty())
  {
    os << indent << "Children:\n";
    for (const auto& child : comp.children())
    {
      os << child;
    }
  }
  g_indentCount -= 2;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Configuration& conf)
{
  g_indentCount += 2;
  std::string indent(g_indentCount, ' ');
  os << indent << "Configuration\n"
     << indent << "Name:  " << conf.name() << "\n"
     << indent << "Type:  " << conf.type() << "\n"
     << indent << "Index: " << conf.includeIndex() << "\n";
  if (!conf.iconName().empty())
  {
    os << indent << "Icon:  " << conf.iconName() << "\n";
  }
  if (conf.label() != conf.name())
  {
    os << indent << "Label: " << conf.label() << "\n";
  }
  os << indent << "Component Data:\n" << conf.details();
  g_indentCount -= 2;
  return os;
}

} // namespace view
} // namespace smtk
