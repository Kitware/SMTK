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

#include "smtk/common/View.h"
#include <algorithm>
#include <sstream> // std::istringstream

namespace smtk
{
namespace common
{

View::Component& View::Component::setContents(const std::string& c)
{
  this->m_contents = c;
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
  std::istringstream iss(this->m_contents);
  iss >> val;
  if (!iss.good())
  {
    return false;
  }
  return true;
}

bool View::Component::contentsAsVector(std::vector<double>& vec) const
{
  std::istringstream iss(this->m_contents);
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

bool View::Component::attribute(const std::string& attname, std::string& value) const
{
  std::map<std::string, std::string>::const_iterator it;
  it = this->m_attributes.find(attname);
  if (it == this->m_attributes.end())
  {
    return false;
  }
  value = it->second;
  return true;
}

View::Component& View::Component::setAttribute(const std::string& attname, const std::string& value)
{
  this->m_attributes[attname] = value;
  return *this;
}

View::Component& View::Component::unsetAttribute(const std::string& attname)
{
  this->m_attributes.erase(attname);
  return *this;
}

View::Component& View::Component::addChild(const std::string& childName)
{
  this->m_children.push_back(Component(childName));
  return this->m_children.back();
}

int View::Component::findChild(const std::string& compName) const
{
  int i, n = static_cast<int>(this->m_children.size());
  for (i = 0; i < n; i++)
  {
    if (this->m_children[i].name() == compName)
    {
      return i;
    }
  }
  return -1;
}

void View::Component::copyContents(const Component& comp)
{
  this->m_name = comp.m_name;
  this->m_attributes = comp.m_attributes;
  this->m_contents = comp.m_contents;
  for (const Component& child : comp.m_children)
  {
    Component& myChild = this->addChild(child.name());
    myChild.copyContents(child);
  }
}

View::View(const std::string& myType, const std::string& myTitle)
  : m_title(myTitle)
  , m_type(myType)
  , m_details("Details")
{
}

View::~View()
{
}

void View::copyContents(const View& view)
{
  this->m_iconName = view.m_iconName;
  this->m_details.copyContents(view.m_details);
}
} // namespace common
} // namespace smtk
