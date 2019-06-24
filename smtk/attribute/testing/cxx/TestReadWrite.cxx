//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <iostream>
#include <sstream>

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/attribute/operators/Export.h"
#include "smtk/attribute/operators/Import.h"
#include "smtk/attribute/operators/Read.h"
#include "smtk/attribute/operators/Write.h"

#include "smtk/common/UUID.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{
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

// This test accepts as input one or more .sbi descriptions of an attribute
// resource. For each input file, it then performs the following:
//
// 1. read the .sbi file into an attribute resource
// 2. write the resource to file as a .sbi file (to ensure the output is of the
//    most recent format)
// 3. write the resource to file and as a .smtk file
// 4. read the generated .smtk file
// 5. write the result as a .sbi file
// 6. check that the .sbi files from steps 2 and 5 are equivalent.
int main(int argc, char** argv)
{
  if (argc == 1)
  {
    std::cout << "Usage: TestReadWrite /path/to/input1.sbi /path/to/input2.sbi..." << std::endl;
    return 1;
  }

  std::string write_root(SMTK_SCRATCH_DIR);

  for (int i = 1; i < argc; i++)
  {
    std::string inputFileName(argv[i]);

    std::cout << "Testing " << inputFileName << std::endl;

    // 1. Read the .sbi file into a new attribute resource

    smtk::attribute::ResourcePtr resource;
    {
      auto importer = smtk::attribute::Import::create();
      importer->parameters()->findFile("filename")->setValue(inputFileName);

      auto result = importer->operate();
      if (result->findInt("outcome")->value() !=
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
      {
        std::cerr << "Import operator failed\n";
        std::cerr << importer->log().convertToString(true) << "\n";
        return -2;
      }

      resource = std::dynamic_pointer_cast<smtk::attribute::Resource>(
        result->findResource("resource")->value());
    }

    // 2. Write the resource out to a new .sbi file (to ensure it is of the
    //    latest format)

    std::string sbi1FileName;
    {
      std::stringstream s;
      s << write_root << "/"
        << "originalResource"
        << ".sbi";
      sbi1FileName = s.str();

      smtk::attribute::Export::Ptr exporter = smtk::attribute::Export::create();
      exporter->parameters()->associate(resource);
      exporter->parameters()->findFile("filename")->setValue(sbi1FileName);

      auto result = exporter->operate();
      if (result->findInt("outcome")->value() !=
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
      {
        std::cerr << "Export operator failed\n";
        std::cerr << exporter->log().convertToString(true) << "\n";
        return -2;
      }
    }

    // 3. Write the resource out to a .smtk file

    std::string smtkFileName;
    {
      std::stringstream s;
      s << write_root << "/"
        << "jsonResource"
        << ".smtk";
      smtkFileName = s.str();
      resource->setLocation(smtkFileName);

      smtk::attribute::Write::Ptr writeOp = smtk::attribute::Write::create();
      writeOp->parameters()->associate(resource);
      auto result = writeOp->operate();

      if (result->findInt("outcome")->value() !=
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
      {
        std::cerr << "Write operation failed\n";
        std::cerr << writeOp->log().convertToString();
        return -2;
      }
    }

    // 4. Read the .smtk file into a new resource

    smtk::attribute::Resource::Ptr copiedResource;
    {
      smtk::attribute::Read::Ptr readOp = smtk::attribute::Read::create();
      readOp->parameters()->findFile("filename")->setValue(smtkFileName);
      auto result = readOp->operate();

      if (result->findInt("outcome")->value() !=
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
      {
        std::cerr << "Read operation failed\n";
        std::cerr << readOp->log().convertToString();
        cleanup(smtkFileName);
        return -2;
      }

      copiedResource = std::dynamic_pointer_cast<smtk::attribute::Resource>(
        result->findResource("resource")->objectValue());
    }

    // 5. Write the new resource to a .sbi file

    std::string sbi2FileName;
    {
      std::stringstream s;
      s << write_root << "/"
        << "jsonResource"
        << ".sbi";
      sbi2FileName = s.str();

      smtk::io::Logger logger;
      smtk::io::AttributeWriter writer;
      if (writer.write(copiedResource, sbi2FileName, logger))
      {
        std::cerr << "Encountered Errors while writing " << sbi2FileName << "\n";
        std::cerr << logger.convertToString();
        return -2;
      }
    }

    // 6. Compare the .sbi files generated in steps 2 and 5

    {
      std::ifstream f1(sbi1FileName, std::ifstream::binary | std::ifstream::ate);
      std::ifstream f2(sbi2FileName, std::ifstream::binary | std::ifstream::ate);

      if (f1.fail() || f2.fail())
      {
        std::cerr << "Could not read in original or generated file\n\n";
        std::cerr << "generated smtk file: " << smtkFileName << "\n";
        std::cerr << "generated sbi1 file: " << sbi1FileName << "\n";
        std::cerr << "generated sbi2 file: " << sbi2FileName << "\n";
        assert(false);
      }

      if (f1.tellg() != f2.tellg())
      {
        std::cerr << "Original and generated file sizes are different\n\n";
        std::cerr << "generated smtk file: " << smtkFileName << "\n";
        std::cerr << "generated sbi1 file: " << sbi1FileName << "\n";
        std::cerr << "generated sbi2 file: " << sbi2FileName << "\n";
        assert(false);
      }

      f1.seekg(0, std::ifstream::beg);
      f2.seekg(0, std::ifstream::beg);
      if (!std::equal(std::istreambuf_iterator<char>(f1.rdbuf()), std::istreambuf_iterator<char>(),
            std::istreambuf_iterator<char>(f2.rdbuf())))
      {
        std::cerr << "Original and generated files are different\n\n";
        std::cerr << "generated smtk file: " << smtkFileName << "\n";
        std::cerr << "generated sbi1 file: " << sbi1FileName << "\n";
        std::cerr << "generated sbi2 file: " << sbi2FileName << "\n";
        assert(false);
      }
    }

    // Clean up all generated files

    cleanup(smtkFileName);
    cleanup(sbi1FileName);
    cleanup(sbi2FileName);
  }
  std::cout << "Pass all file tests" << std::endl;

  return 0;
}
