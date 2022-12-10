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
#include "smtk/model/Edge.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/EntityTypeBits.h"
#include "smtk/model/Face.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Vertex.h"
#include "smtk/operation/Operation.h"
#include "smtk/resource/Component.h"
#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/internal/Model.h"
#include "smtk/session/polygon/internal/Model.txx"
#include "smtk/session/polygon/operators/CreateEdgeFromVertices.h"
#include "smtk/session/polygon/operators/CreateFacesFromEdges.h"
#include "smtk/session/polygon/operators/CreateModel.h"
#include "smtk/session/polygon/operators/Delete.h"

#include "smtk/session/polygon/operators/ImportPPG_xml.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace
{
constexpr std::size_t INVALID_INDEX = std::numeric_limits<std::size_t>::max();
constexpr int OP_SUCCEEDED = static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED);

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

enum class LoopType
{
  FACE = 0, // outer loop
  EMBEDDED, // inner loop as embedded face
  HOLE      // inner loop as hole
};

struct PPGVertex
{
  std::size_t userId;
  double x;
  double y;

  PPGVertex(std::size_t inputId, double xcoord, double ycoord)
    : userId(inputId)
    , x(xcoord)
    , y(ycoord)
  {
  }

  void dump() const
  {
    std::cout << "PPGVertex userId: " << userId << ", x: " << x << ", y: " << y << std::endl;
  }
};

struct PPGFace
{
  std::size_t userId;
  std::vector<std::size_t> vertexIds; // from input
  LoopType loopType;
  std::size_t innerLoopCount;
  std::vector<smtk::model::EntityPtr> edges;
  double center[2];

  PPGFace(
    std::size_t inputId,
    const std::vector<std::size_t>& inputVertexIds,
    LoopType lt,
    double centerCoords[2])
    : userId(inputId)
    , loopType(lt)
    , innerLoopCount(0)
  {
    vertexIds = std::move(inputVertexIds);
    center[0] = centerCoords[0];
    center[1] = centerCoords[1];
  }

  void dump() const
  {
    std::cout << "PPGFace userId: " << userId << ", loop type: " << static_cast<int>(loopType)
              << ", inner loops: " << innerLoopCount << ", center: " << center[0] << ", "
              << center[1] << '\n'
              << "vertex ids:";
    for (auto vid : vertexIds)
    {
      std::cout << " " << vid;
    }
    std::cout << std::endl;
  }
};

// Returns the number of digits needed to represent a give number, e.g. for numbers < 10 only
// one digit is required. For 100 <= number < 999, three digits are needed.
unsigned int numDigitsFor(std::size_t number)
{
  int digits = 0;
  while (number)
  {
    number /= 10;
    digits++;
  }
  return digits;
}

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
    , m_holeCount(0)
    , m_outerLoopIndex(INVALID_INDEX)
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
  bool createModel();
  bool createVertices();
  bool createEdges();
  bool createFaces();

  // Finds PPGFace index matching input model face
  std::size_t findInnerLoop(smtk::model::EntityRef& faceRef, std::size_t outerIndex) const;

  // Parse input line
  bool parseInputLine(const std::string& line, unsigned lineNum);

  void print() const
  {
    std::cout << "PPG input vertex count: " << m_ppgVertexList.size() << std::endl;
    std::cout << "PPG input face count: " << m_ppgFaceList.size() - m_holeCount << std::endl;
    std::cout << "PPG input hole count: " << m_holeCount << std::endl;

    for (const auto& ppgFace : m_ppgFaceList)
    {
      ppgFace.dump();
    }
  }

  smtk::model::ResourcePtr resource() const { return m_resource; }

protected:
  bool parsePPGVertex(const std::vector<std::string>& symbols, unsigned lineNum);
  bool parsePPGFace(const std::vector<std::string>& symbols, unsigned lineNum);

  smtk::io::Logger& m_log;
  std::vector<PPGFace> m_ppgFaceList;
  std::vector<PPGVertex> m_ppgVertexList;
  unsigned int m_holeCount;
  std::size_t m_outerLoopIndex; // used parsing input to store current outer edge loop

  // List of smtk vertices
  std::vector<smtk::model::EntityRef> m_smtkVertexList;

  // Track vertex-pairs used to make edges, to avoid duplication
  std::set<std::pair<std::size_t, std::size_t>> m_ppgVertexPairSet;

  smtk::model::ResourcePtr m_resource;
  std::shared_ptr<smtk::resource::PersistentObject> m_modelEntity;
};

bool ImportPPG::Internal::createModel()
{
#ifndef NDEBUG
  std::cout << "Creating Model..." << std::endl;
#endif

  // Initialize and run CreateModel operation.
  auto createOp = smtk::session::polygon::CreateModel::create();
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

  // Set name
  smtk::model::Model modelRef(m_resource, m_modelEntity->id());
  modelRef.setName("model 1");
  return true;
}

bool ImportPPG::Internal::createVertices()
{
  if (m_ppgVertexList.empty())
  {
    smtkWarningMacro(m_log, "No input vertices specified.");
    return true;
  }

#ifndef NDEBUG
  std::cout << "Creating Vertices..." << std::endl;
#endif

  // Initialize new vertex list with one no-op vertex, because indices start with 1
  m_smtkVertexList.clear();
  m_smtkVertexList.emplace(m_smtkVertexList.end());

  auto polyResource = std::dynamic_pointer_cast<smtk::session::polygon::Resource>(m_resource);
  internal::pmodel::Ptr storage = polyResource->findStorage<internal::pmodel>(m_modelEntity->id());

  std::stringstream ss;
  std::vector<double> coords(2);
  unsigned digits = numDigitsFor(m_ppgVertexList.size());
  for (const auto& v : m_ppgVertexList)
  {
    coords[0] = v.x;
    coords[1] = v.y;
    smtk::session::polygon::internal::Point projected =
      storage->projectPoint(coords.begin(), coords.begin() + 2);
    smtk::model::Vertex newVertex = storage->addModelVertex(m_resource, projected);

    // Set the vertex name
    ss.str(std::string());
    ss.clear();
    ss << "vertex " << std::setfill('0') << std::setw(digits) << v.userId;
    newVertex.setName(ss.str());

    // Update member data
    m_smtkVertexList.push_back(newVertex);
  }
  return true;
}

bool ImportPPG::Internal::createEdges()
{
#ifndef NDEBUG
  std::cout << "Creating Edges..." << std::endl;
#endif

  std::size_t edgeId = 0;
  std::stringstream ss;

  // Get upper limit on number of edges
  std::size_t numEdges = 0;
  for (auto& ppgFace : m_ppgFaceList)
  {
    numEdges += ppgFace.vertexIds.size();
  }
  unsigned digits = numDigitsFor(numEdges);

  // Initialize and run CreateEdgeFromVertices operation.
  auto createOp = smtk::session::polygon::CreateEdgeFromVertices::create();
  for (auto& ppgFace : m_ppgFaceList)
  {
    std::vector<std::size_t>& vertexIds(ppgFace.vertexIds); // shorthand
    // Temporarily add first vertex to the end of the list
    vertexIds.push_back(vertexIds.front());
    for (std::size_t i = 1; i < vertexIds.size(); ++i)
    {
      createOp->parameters()->removeAllAssociations();
      std::size_t id1 = vertexIds[i - 1];
      std::size_t id2 = vertexIds[i];

      // Check for duplicate
      auto pair1 = std::make_pair(id1, id2);
      if (m_ppgVertexPairSet.count(pair1))
      {
        continue;
      }
      m_ppgVertexPairSet.insert(pair1);
      m_ppgVertexPairSet.insert(std::make_pair(id2, id1));

      auto v1 = m_smtkVertexList[id1];
      auto v2 = m_smtkVertexList[id2];

      createOp->parameters()->associateEntity(v1);
      createOp->parameters()->associateEntity(v2);
      auto result = createOp->operate();
      int outcome = result->findInt("outcome")->value();
      if (outcome != OP_SUCCEEDED)
      {
        errorMacroFalse("Failed to create edge from vertex " << (i - 1) << " to " << i);
      }

      // Get the new edge and assign name
      auto createdItem = result->findComponent("created");
      smtk::resource::ComponentPtr pcomp = createdItem->value();
      smtk::model::EntityRef edgeRef(m_resource, pcomp->id());

      ++edgeId;
      ss.str(std::string());
      ss.clear();
      ss << "edge " << std::setfill('0') << std::setw(digits) << edgeId;
      edgeRef.setName(ss.str());

      ppgFace.edges.push_back(edgeRef.entityRecord());
    } // for (i)

    // Remove temp vertex added to end of list
    vertexIds.pop_back();
  } // for (ppgFace)

  return true;
}

bool ImportPPG::Internal::createFaces()
{
#ifndef NDEBUG
  std::cout << "Creating Faces..." << std::endl;
#endif

  // Initialize and run CreateFaces operation.
  auto createOp = smtk::session::polygon::CreateFacesFromEdges::create();
  auto deleteOp = smtk::session::polygon::Delete::create();
  std::stringstream ss;
  for (std::size_t i = 0; i < m_ppgFaceList.size(); ++i)
  {
    createOp->parameters()->removeAllAssociations();

    // Add the next face (outer loop)
    auto& outerPPGFace = m_ppgFaceList[i];
    // outerPPGFace.dump();

    std::size_t outerPPGIndex = i;
    for (auto& edge : outerPPGFace.edges)
    {
      createOp->parameters()->associate(edge);
    }

    // Add inner edge loops
    for (std::size_t n = 0; n < outerPPGFace.innerLoopCount; ++n)
    {
      ++i;
      const auto& innerPPGFace = m_ppgFaceList[i];
      for (const auto& edge : innerPPGFace.edges)
      {
        createOp->parameters()->associate(edge);
      }
    }

    auto result = createOp->operate();
    int outcome = result->findInt("outcome")->value();
    if (outcome != OP_SUCCEEDED)
    {
      errorMacroFalse("Failed to create model faces.");
    }

    // Deal with inner edge loops
    auto createdItem = result->findComponent("created");
    unsigned digits = numDigitsFor(m_ppgFaceList.size() - m_holeCount);
    for (std::size_t n = 0; n < createdItem->numberOfValues(); ++n)
    {
      smtk::resource::ComponentPtr pcomp = createdItem->value(n);
      smtk::model::EntityRef faceRef(m_resource, pcomp->id());
      if (n == 0)
      {
        // First face is the outer loop
        ss.str(std::string());
        ss.clear();
        ss << "face " << std::setfill('0') << std::setw(digits) << outerPPGFace.userId;
        faceRef.setName(ss.str());

        continue;
      }

      // Find matching inner edge loop
      std::size_t index = this->findInnerLoop(faceRef, outerPPGIndex);
      if (index == INVALID_INDEX)
      {
        errorMacroFalse("Error: unabled to find inner face");
      }

      const auto& innerPPGFace = m_ppgFaceList[index];
      if (innerPPGFace.loopType == LoopType::HOLE)
      {
        deleteOp->parameters()->associate(faceRef.entityRecord());
      }
      else
      {
        // First face is the outer loop
        ss.str(std::string());
        ss.clear();
        ss << "face " << std::setfill('0') << std::setw(digits) << innerPPGFace.userId;
        faceRef.setName(ss.str());
      }
    } // for (n)
  }   // for (ppg faces)

  if (!deleteOp->ableToOperate())
  {
    return true; // no faces to delete
  }

  auto deleteResult = deleteOp->operate();
  int deleteOutcome = deleteResult->findInt("outcome")->value();
  if (deleteOutcome != OP_SUCCEEDED)
  {
    errorMacroFalse("Failed to create model faces.");
  }

  return true;
}

std::size_t ImportPPG::Internal::findInnerLoop(
  smtk::model::EntityRef& faceRef,
  std::size_t outerIndex) const
{
  const PPGFace& outerPPGFace = m_ppgFaceList[outerIndex];
  if (outerPPGFace.innerLoopCount < 1)
  {
    smtkErrorMacro(m_log, "Internal Error - PPGFace has no inner loops");
    return INVALID_INDEX;
  }

  // Find which inner loop corresponds to this model face
  // Uses the fact that faces are convex and don't intersect
  std::vector<double> bbox = faceRef.boundingBox();
  std::size_t index = outerIndex + 1;
  for (int i = 0; i < static_cast<int>(outerPPGFace.innerLoopCount); ++i, ++index)
  {
    // Get xmean, ymean
    const PPGFace& innerFace = m_ppgFaceList[index];
    double x = innerFace.center[0];
    double y = innerFace.center[1];
    if ((x >= bbox[0]) && (x <= bbox[1]) && (y >= bbox[2]) && (y <= bbox[3]))
    {
      return index;
    }
  }
  return INVALID_INDEX; // not found
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
  else if (("f" == symbols[0]) || ("e" == symbols[0]) || ("h" == symbols[0]))
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
    errorMacroFalse("Error parsing vertex on line " << lineNum << ": " << e.what());
  }

  auto userId = 1 + m_ppgVertexList.size();
  m_ppgVertexList.emplace(m_ppgVertexList.end(), userId, x, y);
  return true;
} // parsePPGVertex()

bool ImportPPG::Internal::parsePPGFace(const std::vector<std::string>& symbols, unsigned lineNum)
{
  if (symbols.size() < 2)
  {
    errorMacroFalse("Face on line " << lineNum << " not specified with any vertex indices.");
  }

  std::vector<std::size_t> vertexIds;
  std::size_t vertexId;
  double xsum = 0.0;
  double ysum = 0.0;
  for (auto it = symbols.begin() + 1; it != symbols.end(); ++it)
  {
    std::string s = *it;
    if (s.at(0) == '#')
    {
      break; // comment at end of line
    }

    try
    {
      vertexId = std::stoi(s);
    }
    catch (const std::exception& e)
    {
      errorMacroFalse(
        "Error parsing vertex id " << s << " in line " << lineNum << ": " << e.what());
    }

    if (vertexId == 0)
    {
      errorMacroFalse("Invalid vertex id 0 on line " << lineNum << ". (Vertex ids start at 1.)");
    }
    else if (vertexId > m_ppgVertexList.size())
    {
      errorMacroFalse(
        "Invalid vertexId " << vertexId << " on line " << lineNum
                            << ". Greater than number of vertices specified ("
                            << m_ppgVertexList.size() << ").");
    }

    vertexIds.push_back(vertexId);

    xsum += m_ppgVertexList[vertexId - 1].x;
    ysum += m_ppgVertexList[vertexId - 1].y;
  } // for (it)

  double npoints = static_cast<double>(vertexIds.size());
  double center[2] = { xsum / npoints, ysum / npoints };

  LoopType lt = LoopType::FACE;
  lt = symbols[0] == "e" ? LoopType::EMBEDDED : lt;
  lt = symbols[0] == "h" ? LoopType::HOLE : lt;
  bool isHole = lt == LoopType::HOLE;

  // Sanity check
  if (m_ppgFaceList.empty() && lt != LoopType::FACE)
  {
    errorMacroFalse("First model face is not type 'f', on line " << lineNum);
  }

  std::size_t inputId = isHole ? 0 : 1 + m_ppgFaceList.size() - m_holeCount;
  m_ppgFaceList.emplace(m_ppgFaceList.end(), inputId, vertexIds, lt, center);
  m_holeCount += isHole ? 1 : 0;

  // Handle outer and inner edge loops differently
  if (lt == LoopType::FACE)
  {
    // Set the index for the current outer loop
    m_outerLoopIndex = m_ppgFaceList.size() - 1;
  }
  else
  {
    // Increment inner loop count for the outer loop
    auto& outerLoop = m_ppgFaceList[m_outerLoopIndex];
    outerLoop.innerLoopCount += 1;
    m_ppgFaceList[m_outerLoopIndex] = outerLoop;
  }

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
  if (!m_internal->createModel())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  this->log().reset();
  if (!m_internal->createVertices())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  this->log().reset();
  if (!m_internal->createEdges())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  this->log().reset();
  if (!m_internal->createFaces())
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
