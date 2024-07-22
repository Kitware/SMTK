//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::attribute;
using namespace smtk::common;
using namespace smtk;

namespace
{
const double double_epsilon = 1.e-10;
}

int unitXmlReaderProperties(int /*unused*/, char* /*unused*/[])
{
  // Read in the test template
  std::string attFile;
  attFile = SMTK_DATA_DIR;
  attFile += "/attribute/attribute_collection/propertiesExample.sbt";
  io::AttributeReader reader;
  io::Logger logger;
  auto attRes = attribute::Resource::create();
  reader.read(attRes, attFile, logger);
  if (logger.hasErrors())
  {
    std::cerr << "Errors Generated when reading SBT file :\n" << logger.convertToString();
    return -1;
  }
  std::cerr << std::boolalpha; // To print out booleans

  //Lets validate that the resource has the correct properties for the resource
  smtkTest(attRes->properties().contains<int>("pi"), "Resource does not contains Int Property pi");
  smtkTest(
    attRes->properties().contains<double>("pd"), "Resource does not contains Double Property pd");
  smtkTest(
    attRes->properties().contains<std::string>("ps"),
    "Resource does not contains String Property ps");
  smtkTest(
    attRes->properties().contains<bool>("pb"), "Resource does not contains Bool Property pb");

  smtkTest(
    attRes->properties().at<int>("pi") == 42,
    "Resource Int Property pi is " << attRes->properties().at<int>("pi") << " but should be 42.");
  smtkTest(
    (attRes->properties().at<double>("pd") - 3.141) < double_epsilon,
    "Resource Double Property pd is " << attRes->properties().at<double>("pd")
                                      << " but should be 3.141");
  smtkTest(
    attRes->properties().at<std::string>("ps") == "Test string",
    "Resource String Property ps is " << attRes->properties().at<std::string>("ps")
                                      << " but should be Test string");
  smtkTest(
    attRes->properties().at<bool>("pb"), "Resource Bool Property pb was false but should be true");

  smtkTest(
    attRes->properties().contains<std::vector<int>>("pvi"),
    "Resource does not contains vector<int> Property pvi");
  smtkTest(
    attRes->properties().at<std::vector<int>>("pvi")[0] == 10,
    "Resource's vector<int> Property pvi [0] = "
      << attRes->properties().at<std::vector<int>>("pvi")[0] << " but should be 10");
  smtkTest(
    attRes->properties().contains<std::vector<long>>("pvl"),
    "Resource does not contains vector<long> Property pvl");
  smtkTest(
    attRes->properties().at<std::vector<long>>("pvl")[0] == 1000,
    "Resource's vector<long> Property pvi [0] = "
      << attRes->properties().at<std::vector<long>>("pvl")[0] << " but should be 1000");
  smtkTest(
    attRes->properties().contains<std::vector<std::string>>("animals"),
    "Resource does not contains vector<string> Property animals");
  smtkTest(
    attRes->properties().at<std::vector<std::string>>("animals")[0] == "the dog",
    "Resource's vector<string> Property animals [0] = \""
      << attRes->properties().at<std::vector<std::string>>("animals")[0]
      << "\" but should be \"the dog\"");
  // Now lets test the properties on the attribute and definition
  auto att = attRes->findAttribute("foo");
  smtkTest(att != nullptr, "Could not find attribute foo");
  auto def = att->definition();
  smtkTest(def != nullptr, "Could not find attribute foo's definition");
  smtkTest(
    def->properties().contains<int>("alpha"), "Definition does not contains Int Property alpha");
  smtkTest(
    def->properties().at<int>("alpha") == 100,
    "Attribute Definition Int Property alpha is " << def->properties().at<int>("alpha")
                                                  << " but should be 100.");
  smtkTest(att->properties().contains<int>("pi"), "Attribute does not contains Int Property pi");
  smtkTest(
    att->properties().contains<double>("pd"), "Attribute does not contains Double Property pd");
  smtkTest(
    att->properties().contains<std::string>("ps"),
    "Attribute does not contains String Property ps");
  smtkTest(att->properties().contains<bool>("pb"), "Attribute does not contains Bool Property pb");

  smtkTest(
    att->properties().at<int>("pi") == 69,
    "Attribute Int Property pi is " << att->properties().at<int>("pi") << " but should be 69.");
  smtkTest(
    (att->properties().at<double>("pd") - 3.141) < double_epsilon,
    "Attribute Double Property pd is " << att->properties().at<double>("pd")
                                       << " but should be 3.141");
  smtkTest(
    att->properties().at<std::string>("ps").empty(),
    "Attribute String Property ps is " << att->properties().at<std::string>("ps")
                                       << " but should be the empty string");
  smtkTest(
    att->properties().at<bool>("pb"), "Attribute Bool Property pb was false but should be true");

  return 0;
}
