//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_filter_GrammarInfo_h
#define smtk_attribute_filter_GrammarInfo_h

namespace smtk
{
namespace attribute
{
namespace filter
{

///\brief Class used to determine the information contained in an
/// attribute query string
///
/// This is used by GrammarInfoActions.
class SMTKCORE_EXPORT GrammarInfo
{
public:
  ///\brief Set the attribute definition type information.
  ///
  /// If \a regexFormat is true, then the information is a regular expression
  /// else it is the type name of an attribute definition.
  void setTypeInfo(const std::string& type, bool regexFormat = false)
  {
    m_typeInfo = type;
    m_isRegex = regexFormat;
  }
  ///\brief Returns the type information from the query string
  const std::string& typeInfo() const { return m_typeInfo; }

  ///\brief Indicates if the type information is a regular expression
  bool isRegex() const { return m_isRegex; }
  ///\brief Indicate that the query has property constraints
  void setHasProperties() { m_containsProperties = true; }
  ///\brief Indicates that the query has property constraints
  bool hasProperties() const { return m_containsProperties; }

protected:
  std::string m_typeInfo;
  bool m_containsProperties = false;
  bool m_isRegex = false;
};

} // namespace filter
} // namespace attribute
} // namespace smtk

#endif
