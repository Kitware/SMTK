//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "ImportPPG.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/session/polygon/Resource.h"

#include "smtk/session/polygon/ImportPPG_xml.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
struct PPGVertex
{
  double x;
  double y;

  PPGVertex(double xval, double yval)
    : x(xval)
    , y(yval)
  {
  }
};

struct PPGFace
{
  std::vector<unsigned int> vertices;
  bool isHole;

  PPGFace(bool hole = false)
    : isHole(hole)
  {
  }
};

} // namespace

namespace smtk
{
namespace session
{
namespace polygon
{

smtk::operation::Operation::Result ImportPPG::operateInternal()
{
  std::stringstream ss;

  // Check for string input first
  auto stringItem = this->parameters()->findString("string");
  if (stringItem->isEnabled())
  {
    ss.str(stringItem->value());
  }
  else
  {
    std::string filename = this->parameters()->findFile("filename")->value();
    std::ifstream infile(filename);
    if (!infile.good())
    {
      smtkErrorMacro(this->log(), "Unable to find or read input file: " << filename);
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    ss << infile.rdbuf();
    infile.close();
  }

  auto polySession = smtk::session::polygon::Session::create();
  auto resource = smtk::session::polygon::Resource::create();
  resource->setSession(polySession);
  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  if (ss.rdbuf()->in_avail() > 0)
  {
    // Create list of vertex and face instances
    std::vector<PPGVertex> vertexList;
    std::vector<PPGFace> faceList;
  }

  result->findResource("resource")->setValue(resource);

  return result;
}

bool ImportPPG::ableToOperate()
{
  // Check if filename is set
  if (this->parameters()->findFile("filename")->isValid())
  {
    return true;
  }

  // If no filename, check for string item (advanced level)
  auto stringItem = this->parameters()->findString("string");
  return stringItem->isEnabled() && stringItem->isSet();
}

const char* ImportPPG::xmlDescription() const
{
  return ImportPPG_xml;
}

} // namespace polygon
} // namespace session
} // namespace smtk
