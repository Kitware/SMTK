//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkFileLocation.h - A class for SMTK file locations
// .SECTION Description
// This class will allow user specify the path of a file and a reference
// path that a relative path w.r.t the reference path can be constructed.
// This is useful when we try to export a json file (.smtk file) that will
// include files for the native models and meshes, which should be relative
// to the json (.smtk) file location, so that all the files can be shared
// easily with other users.
// .SECTION See Also

#ifndef __smtk_common_FileLocation_h
#define __smtk_common_FileLocation_h

#include "smtk/CoreExports.h"
#include "smtk/common/CompilerInformation.h"
#include <string>

#ifdef SMTK_MSVC
// Ignore symbol exposure warnings for STL classes.
#pragma warning(disable : 4251)
#endif

namespace smtk
{
namespace common
{
class SMTKCORE_EXPORT FileLocation
{
public:
  FileLocation()
  {
    m_filePath = std::string();
    m_referencePath = std::string();
  }
  FileLocation(const std::string& filePath, const std::string& refPath = std::string());
  FileLocation(const FileLocation& other);
  virtual ~FileLocation() {}

  std::string absolutePath() const;
  std::string relativePath() const;
  std::string referencePath() const;

  bool empty() const { return this->m_filePath.empty(); }
  void clear()
  {
    m_filePath.clear();
    m_referencePath.clear();
  }
  FileLocation& operator=(const FileLocation& from)
  {
    m_filePath = from.m_filePath;
    m_referencePath = from.m_referencePath;
    return *this;
  }
  bool operator==(const FileLocation& from) const;
  bool operator==(const std::string& from) const { return *this == FileLocation(from); }

protected:
  std::string m_filePath;
  std::string m_referencePath;
};
}
}

#endif /* __smtk_common_FileLocation_h */
