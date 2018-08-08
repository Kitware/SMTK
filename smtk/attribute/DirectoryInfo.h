//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME DirectoryInfo.h - represnts the directory structure of an Attribute Resource
// .SECTION Description
// DirectoryInfo is meant to store the directory structure associated with an
// attribute resource
// .SECTION See Also
/* Intended Use
  A FileInfo instance represents the information found in an included attribute file
   which includes the following:
    - Catagories specified in the file
    - The set of include files that file uses
    - The default catagory that the file specified (if any)

  The DirectoryInfo is simply a vector of FileInfos.  The IncludeIndex property
  found in Attributes, Definitions, and View corresponds to an index into this
  vector indicating that the instanced was read in from that file.

  The first index in the array is "special" in that it represents the ressource
  itself.  Any new Defintions, Attributes, or Views created after the resource
  is loaded into memory will be considered part of that "file".  This is why
  the default value for the IncludeIndex Property is 0!
*/

#ifndef __smtk_attribute_DirectoryInfo_h
#define __smtk_attribute_DirectoryInfo_h

#include "smtk/CoreExports.h"

#include <set>
#include <string>
#include <vector>

namespace smtk
{
namespace attribute
{

class SMTKCORE_EXPORT FileInfo
{
public:
  FileInfo(const std::string& f)
    : m_filename(f)
  {
  }
  const std::string& filename() const { return m_filename; }
  void appendInclude(const std::string& includeFile) { m_includes.push_back(includeFile); }
  const std::vector<std::string>& includeFiles() const { return m_includes; }
  void setCatagories(const std::set<std::string>& newCatagories) { m_catagories = newCatagories; }
  const std::set<std::string>& catagories() const { return m_catagories; }
  void setDefaultCatagory(const std::string& catagory) { m_defaultCategory = catagory; }
  const std::string& defaultCatagory() const { return m_defaultCategory; }

  void print(const std::string& pre);

protected:
  std::string m_filename;
  std::string m_defaultCategory;
  std::vector<std::string> m_includes;
  std::set<std::string> m_catagories;
};

typedef std::vector<FileInfo> DirectoryInfo;
}; // namespace attribute
}; // namespace smtk

#endif
