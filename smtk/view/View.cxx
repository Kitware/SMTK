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

#include "smtk/view/View.h"
#include <algorithm>
#include <sstream> // std::istringstream

namespace smtk
{
namespace view
{

View::Component& View::Component::setContents(const std::string& c)
{
  m_contents = c;
  return *this;
}

bool View::Component::attributeAsBool(const std::string& attname, bool& val) const
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

bool View::Component::attributeAsInt(const std::string& attname, int& val) const
{
  std::string s;
  if (!this->attribute(attname, s))
  {
    return false; // name doesn't exist
  }

  val = std::stoi(s);
  return true;
}

bool View::Component::attributeAsDouble(const std::string& attname, double& val) const
{
  std::string s;
  if (!this->attribute(attname, s))
  {
    return false; // name doesn't exist
  }

  val = std::stod(s);
  return true;
}

bool View::Component::attributeAsBool(const std::string& attname) const
{
  bool v;
  if (this->attributeAsBool(attname, v))
  {
    return v;
  }
  return false;
}

bool View::Component::contentsAsInt(int& val) const
{
  std::istringstream iss(m_contents);
  iss >> val;
  if (!iss.good())
  {
    return false;
  }
  return true;
}

bool View::Component::contentsAsVector(std::vector<double>& vec) const
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
      return (vec.size() > 0);
    }
    vec.push_back(d);
  }
  return (vec.size() > 0);
}

bool View::Component::attribute(const std::string& attname) const
{
  std::string dummy;
  return this->attribute(attname, dummy);
}

bool View::Component::attribute(const std::string& attname, std::string& value) const
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

View::Component& View::Component::setAttribute(const std::string& attname, const std::string& value)
{
  m_attributes[attname] = value;
  return *this;
}

View::Component& View::Component::unsetAttribute(const std::string& attname)
{
  m_attributes.erase(attname);
  return *this;
}

View::Component& View::Component::addChild(const std::string& childName)
{
  m_children.push_back(Component(childName));
  return m_children.back();
}

int View::Component::findChild(const std::string& compName) const
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

void View::Component::copyContents(const Component& comp)
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

View::View(const std::string& myType, const std::string& myName)
  : m_name(myName)
  , m_type(myType)
  , m_details("Details")
  , m_includeIndex(0)
{
}

View::~View()
{
}

void View::copyContents(const View& view)
{
  m_iconName = view.m_iconName;
  m_details.copyContents(view.m_details);
}

std::string View::label() const
{
  std::string l;
  if (m_details.attribute("Label", l))
  {
    return l;
  }
  return m_name;
}

} // namespace view
} // namespace smtk
