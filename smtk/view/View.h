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

#ifndef __smtk_view_View_h
#define __smtk_view_View_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include <string>

namespace smtk
{
namespace view
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

    const std::string& name() const { return m_name; }
    const std::string& contents() const { return m_contents; }
    bool contentsAsVector(std::vector<double>& vec) const;
    bool contentsAsInt(int& val) const;

    Component& setContents(const std::string& c);

    Component& setAttribute(const std::string& attname, const std::string& value);
    Component& unsetAttribute(const std::string& attname);

    //Description:
    // Returns true if the component has an attribute called name and will
    // set value to the attribute's values.  Else it returns false
    bool attribute(const std::string& attname, std::string& value) const;
    // Description:
    // Simply tests to see if the attribute exists
    bool attribute(const std::string& attname) const;
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

    bool attributeAsInt(const std::string& attname, int& val) const;
    bool attributeAsDouble(const std::string& attname, double& val) const;

    const std::map<std::string, std::string>& attributes() const { return m_attributes; }

    Component& addChild(const std::string& childName);

    void copyContents(const Component& comp);

    const std::vector<Component>& children() const { return m_children; }

    std::size_t numberOfChildren() const { return m_children.size(); }

    Component& child(std::size_t i) { return m_children[i]; }

    // Returns -1 if there is no child by that name
    int findChild(const std::string& compName) const;

    bool operator==(const Component& other) const
    {
      return m_name == other.m_name && m_contents == other.m_contents &&
        m_attributes == other.m_attributes && m_children == other.m_children;
    }

  protected:
    std::string m_name;
    std::string m_contents;
    std::map<std::string, std::string> m_attributes;
    std::vector<Component> m_children;
  };

  View(const std::string& myType, const std::string& myName);
  static smtk::view::ViewPtr New(const std::string& myType, const std::string& myName)
  {
    return smtk::view::ViewPtr(new smtk::view::View(myType, myName));
  }

  ~View();

  // Copy the contents of one View into another - this View will be the same as
  // v with the exception of its name and type
  void copyContents(const View& v);
  const std::string& name() const { return m_name; }
  // Returns the name to be used in the GUI for the View - if there is none
  // defined the name is returned.
  std::string label() const;

  const std::string& type() const { return m_type; }
  void setType(const std::string& type) { m_type = type; }

  const std::string& iconName() const { return m_iconName; }
  void setIconName(const std::string& iname) { m_iconName = iname; }

  Component& details() { return m_details; }

  bool operator==(const View& other) const
  {
    return m_name == other.m_name && m_type == other.m_type && m_iconName == other.m_iconName &&
      m_details == other.m_details;
  }

  // These methods are use primarily by I/O operations.  The include ID corresponds to
  // the include directory information store in the attribute reosurce and is used
  // when writing out the resource to use include files
  void setIncludeIndex(std::size_t index) { m_includeIndex = index; }

  std::size_t includeIndex() const { return m_includeIndex; }

protected:
  std::string m_name;
  std::string m_type;
  std::string m_iconName;
  Component m_details;
  std::size_t m_includeIndex;
};
}
}

#endif /* __smtk_view_View_h */
