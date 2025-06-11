//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME view/Configuration.h -  Base class for SMTK views
// .SECTION Description
//   A SMTK view is used to describe workflows
// .SECTION See Also

#ifndef smtk_view_Configuration_h
#define smtk_view_Configuration_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include <memory>
#include <string>

namespace smtk
{
namespace view
{
/// @brief Configure a view, specifying types and attributes, without specifying a UI library.
class SMTKCORE_EXPORT Configuration : smtkEnableSharedPtr(Configuration)
{
public:
  smtkTypeMacroBase(smtk::view::Configuration);

  /// @brief Configure one item in a view, which may contain children.
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
    ~Component() = default;

    const std::string& name() const { return m_name; }
    const std::string& contents() const { return m_contents; }
    bool contentsAsVector(std::vector<double>& vec) const;
    bool contentsAsInt(int& val) const;

    Component& setContents(const std::string& c);

    Component& setAttribute(const std::string& attname, const std::string& value);
    Component& unsetAttribute(const std::string& attname);

    /// Returns true if the component has an attribute called name and will
    /// set value to the attribute's values.  Else it returns false
    bool attribute(const std::string& attname, std::string& value) const;
    /// Simply tests to see if the attribute exists
    bool attribute(const std::string& attname) const;
    /// Returns true if the component has an attribute called name and if it has the
    /// string value of true, t, false, or f (ignoring case). Value will be true if the attribute
    /// is t or true, false if attribute is f or false and not set otherwise
    /// set value to the attribute's values.  Else it returns false
    bool attributeAsBool(const std::string& attname, bool& value) const;
    /// Returns true if the component has an attribute called name and if its value is
    /// either t or true (ignoring case).  Else it returns false.
    bool attributeAsBool(const std::string& attname) const;

    /// Returns the value of the attribute named \a attname as a string. It will return
    /// an empty string if the attribute does not exists
    std::string attributeAsString(const std::string& attname) const;

    bool attributeAsInt(const std::string& attname, int& val) const;
    bool attributeAsDouble(const std::string& attname, double& val) const;

    const std::map<std::string, std::string>& attributes() const { return m_attributes; }

    Component& addChild(const std::string& childName);

    void copyContents(const Component& comp);

    const std::vector<Component>& children() const { return m_children; }
    std::vector<Component>& children() { return m_children; }

    std::size_t numberOfChildren() const { return m_children.size(); }

    Component& child(std::size_t i) { return m_children[i]; }
    const Component& child(std::size_t i) const { return m_children[i]; }

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

  Configuration(const std::string& myType, const std::string& myName);
  static smtk::view::ConfigurationPtr New(const std::string& myType, const std::string& myName)
  {
    return std::make_shared<smtk::view::Configuration>(myType, myName);
  }

  ~Configuration();

  /// Copy the contents of one Configuration into another - this Configuration will be the same as
  /// v with the exception of its name and type
  void copyContents(const Configuration& v);
  const std::string& name() const { return m_name; }
  /// Returns the label to be used in the GUI for the Configuration - if there is none
  /// defined the name is returned.
  std::string label() const;

  const std::string& type() const { return m_type; }
  void setType(const std::string& type) { m_type = type; }

  const std::string& iconName() const { return m_iconName; }
  void setIconName(const std::string& iname) { m_iconName = iname; }

  Component& details() { return m_details; }
  const Component& details() const { return m_details; }

  bool operator==(const Configuration& other) const
  {
    return m_name == other.m_name && m_type == other.m_type && m_iconName == other.m_iconName &&
      m_details == other.m_details;
  }

  /// These methods are use primarily by I/O operations.  The include ID corresponds to
  /// the include directory information store in the attribute reosurce and is used
  /// when writing out the resource to use include files
  void setIncludeIndex(std::size_t index) { m_includeIndex = index; }

  std::size_t includeIndex() const { return m_includeIndex; }

protected:
  std::string m_name;
  std::string m_type;
  std::string m_iconName;
  Component m_details;
  std::size_t m_includeIndex{ 0 };
};

/// Print component information to a stream (for debugging).
SMTKCORE_EXPORT std::ostream& operator<<(std::ostream& os, const Configuration::Component& comp);
/// Print configuration information to a stream (for debugging).
SMTKCORE_EXPORT std::ostream& operator<<(std::ostream& os, const Configuration& conf);
} // namespace view
} // namespace smtk

#endif /* smtk_view_Configuration_h */
