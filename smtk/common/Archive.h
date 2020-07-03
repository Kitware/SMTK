//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_common_Archive_h
#define __smtk_common_Archive_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"

#include <fstream>
#include <memory>
#include <set>
#include <string>

namespace smtk
{
namespace common
{

/**\brief A class for handling SMTK archives
  *
  * An SMTK Archive is a portable collection of files that are stored as a
  * single file on-disk. An archive is described by its filesystem path. Once
  * instantiated, a user can insert files into the archive,
  * serialize/deserialize the archive to/from disk, access a list of files in
  * the archive, and acquire file streams to these files by accessing them via
  * their name. An archive can be considered a directory containing files;
  * as such, each file in the archive must be assigned a unique path.
  */
class SMTKCORE_EXPORT Archive
{
public:
  /// Construct a new archive or access an existing archive at a given file
  /// path
  Archive(const std::string& archivePath);
  virtual ~Archive();

  /// Check if the input file located at <archivePath> is an archive.
  static bool isArchive(const std::string& archivePath);

  /// Add a file (identified by its path) to the archive. Once in the archive,
  /// a stream to the file is accessible using the file's archived path. Each
  /// file in the archive must therefore have a unique path. Return true upon
  /// success.
  bool insert(const std::string& filePath, const std::string& archivedPath);

  /// Serialize the files that comprise the archive to a contiguous block of
  /// memory on disk (located at \a archivePath ). Return true upon success.
  bool archive() const;

  /// Deserialize the files that comprise the archive into a temporary
  /// directory for access via streams. If a stream is requested from an
  /// unextracted archive, the archive is automatically extracted prior to
  /// returning the stream. Return true upon success.
  bool extract();

  /// Acquire a collection of archived file paths from the archive. This method
  // can be called without requiring that the archive be extracted.
  std::set<std::string> contents() const;

  /// Acquire a stream to a file in the archive, accessed by its archived file
  /// path.
  ///
  /// This method cannot be used on GCC < 5 due to a bug (see
  /// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53626). However, since we
  /// put paths where they should be and not where people walk, this API will
  /// remain in place.
  std::ifstream get(const std::string& archivedFilePath);

  /// Acquire a stream to a file in the archive, accessed by its archived file
  /// path. This method is functionally equivalent to the above get() method,
  /// but is provided as a workaround for a bug in GCC <5 (see
  /// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53626).
  void get(const std::string& archivedFilePath, std::ifstream&);

  /// Acquire the path to the file in the archive, accessed by its archived file
  /// path. Return the empty string if the archived file path is not in the
  /// archive.
  std::string location(const std::string& archivedFilePath);

private:
  struct Internals;
  std::unique_ptr<Internals> m_internals;
};
}
}

#endif
