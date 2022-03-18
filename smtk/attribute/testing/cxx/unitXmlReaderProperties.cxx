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

  // Now lets test the properties on the attribute
  auto att = attRes->findAttribute("foo");
  smtkTest(att != nullptr, "Could not find attribute foo");
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
