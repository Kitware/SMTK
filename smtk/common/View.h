//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkView.h -  Base class for SMTK views
// .SECTION Description
//   A SMTK view is used to describe workflows
// .SECTION See Also

#ifndef __smtk_common_View_h
#define __smtk_common_View_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include <string>

namespace smtk
{
namespace common
{
class SMTKCORE_EXPORT View
{
public:
  class SMTKCORE_EXPORT Component
  {
  public:
    Component(const std::string& myName)
      : m_name(myName)
    {
    }

    Component()
      : m_name("Default")
    {
    }
    ~Component() {}

    const std::string& name() const { return this->m_name; }

    const std::string& contents() const { return this->m_contents; }
    bool contentsAsVector(std::vector<double>& vec) const;
    bool contentsAsInt(int& val) const;

    Component& setContents(const std::string& c);

    Component& setAttribute(const std::string& attname, const std::string& value);
    Component& unsetAttribute(const std::string& attname);

    //Description:
    // Returns true if the component has an attribute called name and will
    // set value to the attribute's values.  Else it returns false
    bool attribute(const std::string& attname, std::string& value) const;
    //Description:
    // Returns true if the component has an attribute called name and if it has the
    // string value of true, t, false, or f (ignoring case). Value will be true if the attribute
    // is t or true, false if attribute is f or false and not set otherwise
    // set value to the attribute's values.  Else it returns false
    bool attributeAsBool(const std::string& attname, bool& value) const;
    //Description:
    // Returns true if the component has an attribute called name and if it's value is
    // either t or true (ignoring case).  Else it returns false.

    bool attributeAsBool(const std::string& attname) const;
    const std::map<std::string, std::string>& attributes() const { return this->m_attributes; }

    Component& addChild(const std::string& childName);

    void copyContents(const Component& comp);

    std::size_t numberOfChildren() const { return this->m_children.size(); }

    Component& child(std::size_t i) { return this->m_children[i]; }

    int findChild(const std::string& compName) const;

  protected:
    std::string m_name;
    std::string m_contents;
    std::map<std::string, std::string> m_attributes;
    std::vector<Component> m_children;
  };

  View(const std::string& myType, const std::string& myTitle);
  static smtk::common::ViewPtr New(const std::string& myType, const std::string& myTitle)
  {
    return smtk::common::ViewPtr(new smtk::common::View(myType, myTitle));
  }

  ~View();

// Copy the contents of one View into another - this View will be the same as
// v with the exception of its title and type
#ifndef SHIBOKEN_SKIP
  void copyContents(const View& v);
#endif
  const std::string& title() const { return this->m_title; }

  const std::string& type() const { return this->m_type; }

  const std::string& iconName() const { return this->m_iconName; }
  void setIconName(const std::string& name) { this->m_iconName = name; }

  Component& details() { return this->m_details; }

protected:
  std::string m_title;
  std::string m_type;
  std::string m_iconName;
  Component m_details;
};
}
}

#endif /* __smtk_common_View_h */
