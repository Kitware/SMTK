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
#include "smtk/common/StringUtil.h"
#include "smtk/io/Logger.h"
#include "smtk/session/polygon/Resource.h"

#include "smtk/session/polygon/ImportPPG_xml.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
// Macro for returning on error
#define errorMacro(msg)                                                                            \
  do                                                                                               \
  {                                                                                                \
    smtkErrorMacro(this->log(), msg);                                                              \
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);                        \
  } while (0)

// Macro for returning error from Internal
#define errorMacroFalse(msg)                                                                       \
  do                                                                                               \
  {                                                                                                \
    smtkErrorMacro(m_log, msg);                                                                    \
    return false;                                                                                  \
  } while (0)

struct PPGVertex
{
  double x;
  double y;

  PPGVertex(double xcoord, double ycoord)
    : x(xcoord)
    , y(ycoord)
  {
  }
};

struct PPGFace
{
  std::vector<unsigned int> vertices;
  bool isHole;

  PPGFace(const std::vector<unsigned int>& inputVerts, bool hole = false)
    : isHole(hole)
  {
    vertices = std::move(inputVerts);
  }
};
} // namespace

namespace smtk
{
namespace session
{
namespace polygon
{
class ImportPPG::Internal
{
public:
  Internal(smtk::io::Logger& logger)
    : m_log(logger)
  {
  }
  ~Internal() = default;

  // Parse input line
  bool parseInputLine(const std::string& line, unsigned lineNum);
  bool parsePPGVertex(const std::vector<std::string>& symbols, unsigned lineNum);
  bool parsePPGFace(const std::vector<std::string>& symbols, unsigned lineNum);

  smtk::io::Logger& m_log;
  std::vector<PPGVertex> m_ppgVertexList;
  std::vector<PPGFace> m_ppgFaceList;
};

bool ImportPPG::Internal::parseInputLine(const std::string& line, unsigned lineNum)
{
  // std::cout << line << std::endl;
  std::vector<std::string> symbols = smtk::common::StringUtil::split(line, " ", true, true);
  if (symbols.empty() || "#" == symbols[0])
  {
    return true;
  }

  else if ("v" == symbols[0])
  {
    return this->parsePPGVertex(symbols, lineNum);
  }
  else if (("f" == symbols[0]) || ("h") == symbols[0])
  {
    return this->parsePPGFace(symbols, lineNum);
  }
  else
  {
    errorMacroFalse("Unrecognized input line " << lineNum << ": " << line);
    return false;
  }

  return true;
}

bool ImportPPG::Internal::parsePPGVertex(const std::vector<std::string>& symbols, unsigned lineNum)
{
  if (symbols.size() < 3)
  {
    errorMacroFalse("Vertex on line " << lineNum << " not specified with x and y coords.");
  }

  double x;
  double y;
  try
  {
    x = std::stod(symbols[1]);
    y = std::stod(symbols[2]);
  }
  catch (const std::exception& e)
  {
    errorMacroFalse("Error parsing vertex on line " << lineNum);
  }

  m_ppgVertexList.emplace(m_ppgVertexList.end(), x, y);
  return true;
}

bool ImportPPG::Internal::parsePPGFace(const std::vector<std::string>& symbols, unsigned lineNum)
{
  if (symbols.size() < 2)
  {
    errorMacroFalse("Face on line " << lineNum << " not specified with any vertex indices.");
  }

  std::vector<unsigned int> vertices;
  unsigned int index;
  for (auto it = symbols.begin() + 1; it != symbols.end(); ++it)
  {
    std::string s = *it;
    try
    {
      index = std::stoi(s);
    }
    catch (const std::exception& e)
    {
      errorMacroFalse("Error parsing index " << s << " in line " << lineNum);
    }

    if (index == 0)
    {
      errorMacroFalse("Invalid index 0 on line " << lineNum << ". (Vertex indices starts at 1.)");
    }
    else if (index > m_ppgVertexList.size())
    {
      errorMacroFalse(
        "Invalid index " << index << " on line " << lineNum
                         << ". Greater than number of vertices specified ("
                         << m_ppgVertexList.size() << ").");
    }
  } // for (it)

  bool isHole = symbols[0] == "h";
  m_ppgFaceList.emplace(m_ppgFaceList.end(), vertices, isHole);

  // m_ppgVertexList.emplace(m_ppgVertexList.end(), x, y);
  return true;
}

ImportPPG::ImportPPG()
{
  m_internal = new Internal(this->log());
}

ImportPPG::~ImportPPG()
{
  delete m_internal;
}

smtk::operation::Operation::Result ImportPPG::operateInternal()
{
  m_internal->m_ppgVertexList.clear();
  m_internal->m_ppgFaceList.clear();

  std::stringstream ss;

  // Check for string input first
  auto stringItem = this->parameters()->findString("string");
  if (stringItem->isEnabled())
  {
    ss.str(stringItem->value());
  }
  else
  {
    // Otherwise check for file input
    std::string filename = this->parameters()->findFile("filename")->value();
    std::ifstream infile(filename);
    if (!infile.good())
    {
      errorMacro("Unable to find or read input file: " << filename);
    }

    ss << infile.rdbuf();
    infile.close();
  }

  // Traverse the input
  if (ss.rdbuf()->in_avail() > 0)
  {
    std::string line;
    unsigned int lineNum = 0;
    while (std::getline(ss, line, '\n'))
    {
      ++lineNum;
      if (!m_internal->parseInputLine(line, lineNum))
      {
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
    } // while
  }   // if

#ifndef NDEBUG
  std::cout << "PPG input vertex count: " << m_internal->m_ppgVertexList.size() << std::endl;
  std::cout << "PPG input face count: " << m_internal->m_ppgFaceList.size() << std::endl;
#endif

  // Create the resource (note that polygon session is also required)
  auto polySession = smtk::session::polygon::Session::create();
  auto resource = smtk::session::polygon::Resource::create();
  resource->setSession(polySession);

  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
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
