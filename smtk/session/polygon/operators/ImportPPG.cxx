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
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/resource/Component.h"
#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/internal/Model.h"
#include "smtk/session/polygon/internal/Model.txx"
#include "smtk/session/polygon/operators/CreateEdgeFromVertices.h"
#include "smtk/session/polygon/operators/CreateFacesFromEdges.h"
#include "smtk/session/polygon/operators/CreateModel.h"
#include "smtk/session/polygon/operators/Delete.h"

#include "smtk/session/polygon/ImportPPG_xml.h"

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
  std::vector<unsigned int> vertexIds; // from input
  std::vector<unsigned int> edgeIds;   // generated internally
  bool isHole;

  PPGFace(const std::vector<unsigned int>& inputVertexIds, bool hole = false)
    : isHole(hole)
  {
    vertexIds = std::move(inputVertexIds);
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
  bool createVertices();
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
  std::vector<smtk::model::EntityRef> m_newEdgeList;
  std::vector<smtk::model::EntityRef> m_newFaceList;

  smtk::model::ResourcePtr m_resource;
  std::shared_ptr<smtk::resource::PersistentObject> m_modelEntity;
};

bool ImportPPG::Internal::createModel(smtk::operation::ManagerPtr opManager)
{
#ifndef NDEBUG
  std::cout << "Creating Model..." << std::endl;
#endif

  // Initialize and run CreateModel operation.
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
  m_newVertexList.clear();
  m_newVertexList.emplace(m_newVertexList.end());

  auto polyResource = std::dynamic_pointer_cast<smtk::session::polygon::Resource>(m_resource);
  internal::pmodel::Ptr storage = polyResource->findStorage<internal::pmodel>(m_modelEntity->id());

  std::stringstream ss;
  std::vector<double> coords(2);
  for (std::size_t i = 0; i < m_ppgVertexList.size(); ++i)
  {
    auto v = m_ppgVertexList[i];
    coords[0] = v.x;
    coords[1] = v.y;
    smtk::session::polygon::internal::Point projected =
      storage->projectPoint(coords.begin(), coords.begin() + 2);
    smtk::model::Vertex newVertex = storage->addModelVertex(m_resource, projected);

    // Set the name starting with vertex 1 (not 0)
    ss.str(std::string());
    ss.clear();
    ss << "vertex " << (i + 1);
    newVertex.setName(ss.str());
    m_newVertexList.push_back(newVertex);
  }
  return true;
}

bool ImportPPG::Internal::createEdges(smtk::operation::ManagerPtr opManager)
{
#ifndef NDEBUG
  std::cout << "Creating Edges..." << std::endl;
#endif

  // Initialize new vertex list with one no-op vertex, because indices start with 1
  m_newEdgeList.clear();
  m_newEdgeList.emplace(m_newEdgeList.end());
  std::stringstream ss;

  // Initialize and run CreateEdgeFromVertices operation.
  auto createOp = opManager->create<smtk::session::polygon::CreateEdgeFromVertices>();
  for (auto& ppgFace : m_ppgFaceList)
  {
    ppgFace.edgeIds.clear();
    std::vector<unsigned int>& vertexIds(ppgFace.vertexIds); // shorthand
    // Temporarily add first vertex to the end of the list
    vertexIds.push_back(vertexIds.front());
    for (std::size_t i = 1; i < vertexIds.size(); ++i)
    {
      createOp->parameters()->removeAllAssociations();
      unsigned id1 = vertexIds[i - 1];
      unsigned id2 = vertexIds[i];

      auto v1 = m_newVertexList[id1];
      auto v2 = m_newVertexList[id2];

      createOp->parameters()->associateEntity(v1);
      createOp->parameters()->associateEntity(v2);
      auto result = createOp->operate();
      int outcome = result->findInt("outcome")->value();
      if (outcome != OP_SUCCEEDED)
      {
        errorMacroFalse("Failed to create edge from vertex " << (i - 1) << " to " << i);
      }

      // Assign name
      auto createdItem = result->findComponent("created");
      smtk::resource::ComponentPtr pcomp = createdItem->value();
      smtk::model::Edge edgeRef(m_resource, pcomp->id());
      std::size_t edgeId = m_newEdgeList.size();
      ppgFace.edgeIds.push_back(edgeId);

      ss.str(std::string());
      ss.clear();
      ss << "edge " << edgeId;
      edgeRef.setName(ss.str());

      m_newEdgeList.push_back(edgeRef);
    } // for (i)

    // Remove temp vertex added to end of list
    vertexIds.pop_back();
  } // for (ppgFace)

  return true;
}

bool ImportPPG::Internal::createFaces(smtk::operation::ManagerPtr opManager)
{
#ifndef NDEBUG
  std::cout << "Creating Faces..." << std::endl;
#endif

  m_newFaceList.clear();
  m_newFaceList.emplace(m_newFaceList.end());
  std::vector<smtk::model::Face> facesToDelete;
  std::stringstream ss;

  // Initialize and run CreateFacesFromEdges operation.
  auto createOp = opManager->create<smtk::session::polygon::CreateFacesFromEdges>();
  for (auto& ppgFace : m_ppgFaceList)
  {
    createOp->parameters()->removeAllAssociations();
    for (unsigned int id : ppgFace.edgeIds)
    {
      auto edgeRef = m_newEdgeList[id];
      createOp->parameters()->associateEntity(edgeRef);
    }

    auto result = createOp->operate();
    int outcome = result->findInt("outcome")->value();
    if (outcome != OP_SUCCEEDED)
    {
      errorMacroFalse("Failed to create face " << m_newFaceList.size());
    }

    // Assign name
    auto createdItem = result->findComponent("created");
    smtk::resource::ComponentPtr pcomp = createdItem->value();
    smtk::model::Face faceRef(m_resource, pcomp->id());
    std::size_t faceId = m_newFaceList.size();

    ss.str(std::string());
    ss.clear();
    ss << "face " << faceId;
    faceRef.setName(ss.str());

    m_newFaceList.push_back(faceRef);

    if (ppgFace.isHole)
    {
      facesToDelete.push_back(faceRef);
    }
  } // for (ppgFace)

  if (!facesToDelete.empty())
  {
#ifndef NDEBUG
    std::cout << "Deleting Faces (" << facesToDelete.size() << ")..." << std::endl;
#endif
    // Initialize and run Delete operation.
    auto deleteOp = opManager->create<smtk::session::polygon::Delete>();
    for (auto& faceRef : facesToDelete)
    {
      deleteOp->parameters()->associateEntity(faceRef);
    }
    auto deleteResult = deleteOp->operate();
    int deleteOutcome = deleteResult->findInt("outcome")->value();
    if (deleteOutcome != OP_SUCCEEDED)
    {
      errorMacroFalse("Failed to delete face(s) " << m_newFaceList.size());
    }
  } // if (facesToDelete)

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

  std::vector<unsigned int> vertexIds;
  unsigned int vertexId;
  for (auto it = symbols.begin() + 1; it != symbols.end(); ++it)
  {
    std::string s = *it;
    try
    {
      vertexId = std::stoi(s);
    }
    catch (const std::exception& e)
    {
      errorMacroFalse("Error parsing vertex id " << s << " in line " << lineNum);
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
  } // for (it)

  bool isHole = symbols[0] == "h";
  m_ppgFaceList.emplace(m_ppgFaceList.end(), vertexIds, isHole);

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
  if (!m_internal->createVertices())
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
