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
#include "smtk/common/UUID.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <map>
#include <string>
#include <utility>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{
//SMTK_DATA_DIR and SMTK_SCRATCH_DIR are defined by cmake
std::string write_root = SMTK_SCRATCH_DIR;

void cleanup(const std::string& file_path)
{
  //first verify the file exists
  ::boost::filesystem::path path(file_path);
  if (::boost::filesystem::is_regular_file(path))
  {
    //remove the file_path if it exists.
    ::boost::filesystem::remove(path);
  }
}
}

int TestArchive(int, char** const)
{
  std::size_t numberOfFiles = 10;
  std::size_t contentLength = 20;

  std::map<std::string, std::pair<std::string, std::string> > filesAndContents;
  {
    for (std::size_t i = 0; i < numberOfFiles; ++i)
    {
      std::pair<std::string, std::string> fileAndContents;
      std::string name = smtk::common::UUID::random().toString();
      fileAndContents.first = write_root + "/" + name;
      for (std::size_t j = 0; j < contentLength; ++j)
      {
        fileAndContents.second += smtk::common::UUID::random().toString() + "\n";
      }

      std::ofstream ofs(fileAndContents.first.c_str(), std::ofstream::out);
      ofs << fileAndContents.second;
      ofs.close();

      filesAndContents["path/to/" + name] = fileAndContents;
    }
  }

  std::string archivePath = write_root + "/archive";
  {
    smtk::common::Archive archive(archivePath);
    for (auto& fileAndContents : filesAndContents)
    {
      archive.insert(fileAndContents.second.first, fileAndContents.first);
    }

    smtkTest(archive.archive(), "Archive failed to archive");
  }

  {
    smtk::common::Archive archive(archivePath);

    auto contents = archive.contents();

    smtkTest(contents.size() == filesAndContents.size(), "Incorrect number of files.");

    smtkTest(archive.extract(), "Archive failed to extract");

    for (auto& name : contents)
    {
      std::ifstream readFile(archive.get(name));

      std::ifstream testFile(filesAndContents[name].first);

      smtkTest(
        std::equal(std::istreambuf_iterator<char>(testFile.rdbuf()),
          std::istreambuf_iterator<char>(), std::istreambuf_iterator<char>(readFile.rdbuf())),
        "Archived file content differs from original file.");
    }
  }

  for (auto& fileAndContents : filesAndContents)
  {
    cleanup(fileAndContents.second.first);
  }

  cleanup(archivePath);

  return 0;
}
