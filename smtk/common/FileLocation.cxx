//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkFileLocation.cxx - A convenient class for smtk file locations
// .SECTION Description
// .SECTION See Also


#include "smtk/common/FileLocation.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

using namespace boost::filesystem;

namespace smtk {
  namespace common {

//----------------------------------------------------------------------------
FileLocation::FileLocation(
  const std::string& filePath,
  const std::string& refPath) :
  m_filePath(filePath),
  m_referencePath(refPath)
{
}

FileLocation::FileLocation(const FileLocation& other)
{
  this->m_filePath = other.m_filePath;
  this->m_referencePath = other.m_referencePath;
}

//----------------------------------------------------------------------------
bool FileLocation::operator == (const FileLocation &from) const
{
  // just doing a simple string comparison, should we also verify the file
  return ( m_filePath == from.m_filePath ) &&
         ( m_referencePath == from.m_referencePath );
}

//----------------------------------------------------------------------------
std::string FileLocation::absolutePath() const
{
  path absPath(this->m_filePath);
  if (!this->m_referencePath.empty() && !absPath.is_absolute())
    {
    absPath = this->m_referencePath / absPath;
    if(exists(absPath))
      {
      // remove potential ".." from the path
      absPath = canonical(absPath, this->m_referencePath);
      }
    }

  return absPath.is_absolute() ? absPath.string() : std::string();
}

//----------------------------------------------------------------------------
std::string FileLocation::relativePath() const
{
  path url(this->m_filePath);
  if (!this->m_referencePath.empty() && url.is_absolute())
    {
    boost::system::error_code err;
    path tryme = relative(url, this->m_referencePath, err);
    if (err == boost::system::errc::success)
      {
      url = tryme;
      }
    }

  return url.is_absolute() ? url.relative_path().string() : url.string();
}

//----------------------------------------------------------------------------
std::string FileLocation::referencePath() const
{
  return this->m_referencePath;
}

  } // namespace common
} // namespace smtk
