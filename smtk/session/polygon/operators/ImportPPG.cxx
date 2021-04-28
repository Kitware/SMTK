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
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/common/StringUtil.h"
#include "smtk/common/UUID.h"
#include "smtk/io/Logger.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/EntityTypeBits.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Vertex.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/resource/Component.h"
#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/operators/CreateEdgeFromVertices.h"
#include "smtk/session/polygon/operators/CreateFacesFromEdges.h"
#include "smtk/session/polygon/operators/CreateModel.h"
#include "smtk/session/polygon/operators/CreateVertices.h"
#include "smtk/session/polygon/operators/Delete.h"

#include "smtk/session/polygon/ImportPPG_xml.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
const int OP_SUCCEEDED = static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED);

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

  void clear()
  {
    m_ppgFaceList.clear();
    m_ppgVertexList.clear();

    m_resource.reset();
    m_modelEntity.reset();
  }

  // Build model entities
  bool createModel(smtk::operation::ManagerPtr opManager);
  bool createVertices(smtk::operation::ManagerPtr opManager);
  bool createEdges(smtk::operation::ManagerPtr opManager);
  bool createFaces(smtk::operation::ManagerPtr opManager);

  // Parse input line
  bool parseInputLine(const std::string& line, unsigned lineNum);

  void print() const
  {
    std::cout << "PPG input vertex count: " << m_ppgVertexList.size() << std::endl;
    std::cout << "PPG input face count: " << m_ppgFaceList.size() << std::endl;
  }

  smtk::model::ResourcePtr resource() const { return m_resource; }

protected:
  bool parsePPGVertex(const std::vector<std::string>& symbols, unsigned lineNum);
  bool parsePPGFace(const std::vector<std::string>& symbols, unsigned lineNum);

  smtk::io::Logger& m_log;
  std::vector<PPGFace> m_ppgFaceList;
  std::vector<PPGVertex> m_ppgVertexList;

  std::vector<smtk::model::EntityRef> m_newVertexList;

  smtk::model::ResourcePtr m_resource;
  std::shared_ptr<smtk::resource::PersistentObject> m_modelEntity;
};

bool ImportPPG::Internal::createModel(smtk::operation::ManagerPtr opManager)
{
  auto createOp = opManager->create<smtk::session::polygon::CreateModel>();
  auto result = createOp->operate();
  int outcome = result->findInt("outcome")->value();
  if (outcome != OP_SUCCEEDED)
  {
    errorMacroFalse(
      "CreateModel operation failed with outcome " << outcome << ". "
                                                   << createOp->log().convertToString());
  }

  auto resourceItem = result->findResource("resource");
  auto resource = resourceItem->value();
  m_resource = std::dynamic_pointer_cast<smtk::model::Resource>(resource);

  smtk::attribute::ComponentItemPtr compItem = result->findComponent("model");
  m_modelEntity = compItem->value();
  return true;
}

bool ImportPPG::Internal::createVertices(smtk::operation::ManagerPtr opManager)
{
  if (m_ppgVertexList.size() < 2)
  {
    return true;
  }
  auto vertsOp = opManager->create<smtk::session::polygon::CreateVertices>();
  bool associated = vertsOp->parameters()->associate(m_modelEntity);

  vertsOp->parameters()->findInt("point dimension")->setValue(2);

  auto pointsGroup = vertsOp->parameters()->findGroup("2d points");
  pointsGroup->setNumberOfGroups(m_ppgVertexList.size());

  // Scan vertex list at 1
  for (std::size_t i = 0; i < m_ppgVertexList.size(); ++i)
  {
    auto v = m_ppgVertexList[i];
    auto pointItem = pointsGroup->findAs<smtk::attribute::DoubleItem>(i, "points");
    pointItem->setValue(0, v.x);
    pointItem->setValue(1, v.y);
  }

  auto result = vertsOp->operate();
  int outcome = result->findInt("outcome")->value();
  if (outcome != OP_SUCCEEDED)
  {
    errorMacroFalse(
      "CreateVertices operation failed with outcome " << outcome << ". "
                                                      << vertsOp->log().convertToString());
  }

  // Now gotta sort created vertices to match input order (never easy)
  // First just copy the vertices into a std::set
  std::set<smtk::model::Vertex> vertexSet;
  std::vector<smtk::model::Vertex> vertexList;
  auto createdItem = result->findComponent("created");
  std::stringstream ss;
  for (std::size_t i = 0; i < createdItem->numberOfValues(); ++i)
  {
    // smtk::resource::PersistentObject pobj = createdItem->value();
    smtk::resource::ComponentPtr pcomp = createdItem->value(i);
    // smtk::model::EntityRef ref(m_resource, pcomp->id());
    smtk::model::Vertex vref(m_resource, pcomp->id());
    vertexSet.insert(vref);
    std::cout << __FILE__ << ":" << __LINE__ << " " << vertexSet.size() << std::endl;
    vertexList.push_back(vref);
    std::cout << __FILE__ << ":" << __LINE__ << " " << vertexList.size() << std::endl;
  }

  // And the sorting by bruce force
  // To reduce computation, the logic uses dx + dy in lieu of actual distance
  // This requires that all points are at least 2*tol apart.
  double tol = 1.0e-6;
  m_newVertexList.clear();
  for (std::size_t i = 0; i < m_ppgVertexList.size(); ++i)
  {
    double x = m_ppgVertexList[i].x;
    double y = m_ppgVertexList[i].y;
    //std::shared_ptr<smtk::model::Vertex> matchVertex;
    smtk::model::Vertex matchRef;
    // matchRef.setEntity(smtk::common::UUID::null());

    std::cout << __FILE__ << ":" << __LINE__ << " " << vertexSet.size() << std::endl;
    for (const auto ref : vertexSet)
    {
      std::cout << __FILE__ << ":" << __LINE__ << " dim: " << ref.dimension()
                << " flags: " << ref.flagSummary() << " is vertex: " << ref.isVertex()
                << " coords: " << ref.coordinates()[0] << ", " << ref.coordinates()[1] << std::endl;
#if 0
      smtk::model::EntityPtr ent = ref.entityRecord();
      std::cout << __FILE__ << ":" << __LINE__ << " " << std::hex << ent.get() << std::dec << std::endl;
      //auto vertex = std::dynamic_pointer_cast<smtk::model::Vertex>(ent);
      const smtk::model::EntityRef* pref = &ref;
      // smtk::model::EntityRef dref = const_cast<smtk::model::EntityRef>(ref);
      const smtk::model::Vertex* vertex = dynamic_cast<const smtk::model::Vertex*>(pref);
      std::cout << __FILE__ << ":" << __LINE__ << " " << std::hex << vertex << std::dec << std::endl;
#else

#endif
      double dx2 = std::fabs(x - ref.coordinates()[0]);
      std::cout << __FILE__ << ":" << __LINE__ << " "
                << "dx2: " << dx2 << std::endl;
      if (dx2 > tol)
      {
        continue;
      }
      double dy2 = std::fabs(y - ref.coordinates()[1]);
      std::cout << __FILE__ << ":" << __LINE__ << " "
                << "dy2: " << dy2 << std::endl;
      if (dy2 < tol)
      {
        matchRef = ref;
        std::cout << __FILE__ << ":" << __LINE__ << " " << ref.isValid() << std::endl;
        std::cout << __FILE__ << ":" << __LINE__ << " " << matchRef.isValid() << std::endl;
        break;
      }
    } // for (vertex)

    if (!matchRef.isValid())
    {
      errorMacroFalse(
        "CreateVertices unable to match input vertex " << (i + 1) << " at coords " << x << ", "
                                                       << y);
    }

    // Now set name starting with vertex 1 (not 0)
    // smtk::model::Vertex pv = matchRef.entityRecord();
    // smtk::model::EntityRef ref(*pv);
    ss.str(std::string());
    ss.clear();
    ss << "vertex " << (i + 1);
    matchRef.setName(ss.str());
    m_newVertexList.push_back(matchRef);
  }

  return true;
}

bool ImportPPG::Internal::createEdges(smtk::operation::ManagerPtr opManager)
{
  return true;
}

bool ImportPPG::Internal::createFaces(smtk::operation::ManagerPtr opManager)
{
  return true;
}

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
} // parseInputLine()

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
} // parsePPGVertex()

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

  return true;
} // parsePPGFace()

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
  m_internal->clear();

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
  m_internal->print();
#endif

  this->log().reset();
  if (!m_internal->createModel(this->manager()))
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  this->log().reset();
  if (!m_internal->createVertices(this->manager()))
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  this->log().reset();
  if (!m_internal->createEdges(this->manager()))
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  this->log().reset();
  if (!m_internal->createFaces(this->manager()))
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  result->findResource("resource")->setValue(m_internal->resource());
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
