//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/Archive.h"

#include "smtk/common/CompilerInformation.h"

#include <map>

#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#ifdef _WIN32
#include <io.h>
#endif
#include <cstdio>
#include <cstdlib>

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"

#include <archive.h>
#include <archive_entry.h>
SMTK_THIRDPARTY_POST_INCLUDE

namespace smtk
{
namespace common
{
struct Archive::Internals
{
  Internals(const std::string& archivePath, bool archived)
    : archivePath(archivePath)
    , archived(archived)
  {
  }

  std::string archivePath;
  std::map<std::string, std::string> filePaths;
  bool archived;
  mutable std::set<std::string> temporaryDirectories;
};

Archive::Archive(const std::string& archivePath)
  : m_internals(
      new Internals(archivePath, boost::filesystem::exists(boost::filesystem::path(archivePath))))
{
}

Archive::~Archive()
{
  for (auto& tempDir : m_internals->temporaryDirectories)
  {
    auto path = boost::filesystem::path(tempDir);
    if (boost::filesystem::exists(path))
    {
      boost::filesystem::remove_all(path);
    }
  }
}

bool Archive::insert(const std::string& filePath, const std::string& archivedPath)
{
  if (m_internals->filePaths.find(archivedPath) != m_internals->filePaths.end())
  {
    return false;
  }

  m_internals->filePaths[archivedPath] = filePath;
  return true;
}

bool Archive::archive() const
{
  struct archive* a;
  struct archive_entry* entry;
  struct stat st;
  char buff[8192];
  int len;
  int fd;

  a = archive_write_new();

  // do not use compression
  archive_write_add_filter_none(a);

  // from libarchive's doc
  // (https://github.com/libarchive/libarchive/wiki/Examples):
  //
  // Libarchive's "pax restricted" format is a tar format that uses pax
  // extensions only when absolutely necessary. Most of the time, it will write
  // plain ustar entries. This is the recommended tar format for most uses. You
  // should explicitly use ustar format only when you have to create archives
  // that will be readable on older systems; you should explicitly request pax
  // format only when you need to preserve as many attributes as possible.
  archive_write_set_format_pax_restricted(a);

  // check that the archive can be created
  if (archive_write_open_filename(a, m_internals->archivePath.c_str()) != ARCHIVE_OK)
  {
    return false;
  }

  bool allArchived = true;

  for (auto& filepath : m_internals->filePaths)
  {
    // access file info (namely the file size)
    if (stat(filepath.second.c_str(), &st) != 0)
    {
      allArchived = false;
      continue;
    }

    entry = archive_entry_new();

    // set the file path relative to the archive (i.e. just the filename)
    archive_entry_set_pathname(entry, filepath.first.c_str());

    // set the file information as reported by stat()
    archive_entry_copy_stat(entry, &st);

    // input file is a regular file (not a link, directory, etc.)
    archive_entry_set_filetype(entry, AE_IFREG);

    // file permissions: user can read/write, everybody else can read
    archive_entry_set_perm(entry, 0644);

    // write the file's description into the archive
    archive_write_header(a, entry);

// open the file for reading
#ifdef _WIN32
    fd = _open(filepath.second.c_str(), _O_RDONLY);
#else
    fd = open(filepath.second.c_str(), O_RDONLY);
#endif

    // transfer the file's contents into the archive
    len = read(fd, buff, sizeof(buff));
    while (len > 0)
    {
      archive_write_data(a, buff, len);
      len = read(fd, buff, sizeof(buff));
    }

#ifdef _WIN32
    _close(fd);
#else
    close(fd);
#endif
    archive_entry_free(entry);
  }

  if (archive_write_close(a) != ARCHIVE_OK)
  {
    allArchived = false;
  }

  archive_write_free(a);

  return allArchived;
}

namespace
{
// Transfer data from one archive to another
la_int64_t copy_data(struct archive* ar, struct archive* aw)
{
  la_int64_t r;
  const void* buff;
  size_t size;
  la_int64_t offset;

  for (;;)
  {
    r = archive_read_data_block(ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF)
    {
      return (ARCHIVE_OK);
    }
    if (r < ARCHIVE_OK)
    {
      return (r);
    }
    r = archive_write_data_block(aw, buff, size, offset);
    if (r < ARCHIVE_OK)
    {
      fprintf(stderr, "%s\n", archive_error_string(aw));
      return (r);
    }
  }
}
}

bool Archive::extract()
{
  m_internals->archived = false;

  struct archive* a;
  struct archive* ext;
  struct archive_entry* entry;
  int flags;
  la_int64_t r;

  // Select which attributes we want to restore.
  flags = ARCHIVE_EXTRACT_TIME;
  flags |= ARCHIVE_EXTRACT_PERM;
  flags |= ARCHIVE_EXTRACT_ACL;
  flags |= ARCHIVE_EXTRACT_FFLAGS;

  // the first archive represents the archive associated with this class
  // instance
  a = archive_read_new();

  // read all supported filter types
  archive_read_support_format_all(a);

  // the second "archive" is a disk archive (i.e. it restores the files to disk)
  ext = archive_write_disk_new();

  archive_write_disk_set_options(ext, flags);
  archive_write_disk_set_standard_lookup(ext);

  // open the first archive
  if ((r = archive_read_open_filename(a, m_internals->archivePath.c_str(), 10240)))
  {
    return false;
  }

  // construct a temporary directory to write the extracted archive
  boost::filesystem::path temp =
    boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();

  if (!boost::filesystem::create_directories(temp))
  {
    return false;
  }
  // track the temporary directory for removal when this class goes out of scope
  m_internals->temporaryDirectories.insert(temp.string());

  // copy the files from one archive into the other
  for (;;)
  {
    r = archive_read_next_header(a, &entry);
    if (r == ARCHIVE_EOF)
    {
      break;
    }
    if (r < ARCHIVE_OK)
    {
      fprintf(stderr, "%s\n", archive_error_string(a));
    }
    if (r < ARCHIVE_WARN)
    {
      return false;
    }

    // construct and set the path for the extracted file
    boost::filesystem::path filePath(archive_entry_pathname(entry));
    const std::string fullOutputPath = (temp / filePath).string();
    archive_entry_set_pathname(entry, fullOutputPath.c_str());

    // write the file's header information into the new archive
    r = archive_write_header(ext, entry);

    if (r < ARCHIVE_OK)
    {
      fprintf(stderr, "%s\n", archive_error_string(ext));
    }
    else if (archive_entry_size(entry) > 0)
    {
      // copy the contents of a file from one archive to the other
      r = copy_data(a, ext);
      if (r < ARCHIVE_OK)
      {
        fprintf(stderr, "%s\n", archive_error_string(ext));
      }
      if (r < ARCHIVE_WARN)
      {
        return false;
      }

      // add the newly extracted file to our list of filees
      m_internals->filePaths[filePath.string()] = fullOutputPath;
    }

    // finalize the file write
    r = archive_write_finish_entry(ext);
    if (r < ARCHIVE_OK)
    {
      fprintf(stderr, "%s\n", archive_error_string(ext));
    }
    if (r < ARCHIVE_WARN)
    {
      return false;
    }
  }

  // finally, close and free both archives
  archive_read_close(a);
  archive_read_free(a);
  archive_write_close(ext);
  archive_write_free(ext);

  return true;
}

std::set<std::string> Archive::contents() const
{
  std::set<std::string> filenames;

  // if the contents of the archive are not yet extracted, we can gather the
  // archive's contained file information from the archive's header
  if (m_internals->archived)
  {
    struct archive* a;
    struct archive_entry* entry;

    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    // open the archive for query
    if (archive_read_open_filename(a, m_internals->archivePath.c_str(), 10240) == ARCHIVE_OK)
    {
      // for each file, extract its filename and copy it to our list of
      // filenames
      while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
      {
        filenames.insert(archive_entry_pathname(entry));
      }
      archive_read_free(a);
    }
  }

  // all extracted files that are actively tracked by the archive must also be
  // reported
  for (auto& filename : m_internals->filePaths)
  {
    filenames.insert(filename.first);
  }

  return filenames;
}

std::ifstream Archive::get(const std::string& archivedFilePath)
{
#if defined(SMTK_CLANG) || (defined(SMTK_GCC) && __GNUC__ > 4) || defined(SMTK_MSVC)
  // if there are any files archived, extract them first
  if (m_internals->archived)
  {
    this->extract();
  }

  // find the file whose filename corresponds to the query
  auto filepath = m_internals->filePaths.find(archivedFilePath);

  if (filepath != m_internals->filePaths.end())
  {
    // return a stream to that file
    return std::ifstream(filepath->second);
  }
#else
  throw std::ios_base::failure("This method cannot be used with GCC < 5 due to a bug in GCC (see "
                               "https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53626)");
#endif

  // we were unable to find an associated file; return the empty stream
  return std::ifstream();
}

void Archive::get(const std::string& archivedFilePath, std::ifstream& stream)
{
  // if there are any files archived, extract them first
  if (m_internals->archived)
  {
    this->extract();
  }

  // find the file whose filename corresponds to the query
  auto filepath = m_internals->filePaths.find(archivedFilePath);

  if (filepath != m_internals->filePaths.end())
  {
    stream.open(filepath->second);
  }
}
}
}
