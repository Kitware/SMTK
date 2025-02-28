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
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/session/polygon/Operation.h"
#include "smtk/session/polygon/Session.h"

#include "smtk/common/PythonInterpreter.h"

#include "smtk/io/ExportMesh.h"

#include "smtk/model/EntityIterator.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/SimpleModelSubphrases.h"
#include "smtk/model/Tessellation.h"

#include "smtk/mesh/core/ForEachTypes.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/testing/cxx/helpers.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

namespace
{
//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

void removeRefsWithoutTess(smtk::model::EntityRefs& ents)
{
  smtk::model::EntityIterator it;
  it.traverse(ents.begin(), ents.end(), smtk::model::ITERATE_BARE);
  std::vector<smtk::model::EntityRef> withoutTess;
  for (it.begin(); !it.isAtEnd(); ++it)
  {
    if (!it->hasTessellation())
    {
      withoutTess.push_back(it.current());
    }
  }

  typedef std::vector<smtk::model::EntityRef>::const_iterator c_it;
  for (c_it i = withoutTess.begin(); i < withoutTess.end(); ++i)
  {
    ents.erase(*i);
  }
}

class ValidatePoints : public smtk::mesh::PointForEach
{
public:
  ValidatePoints(std::size_t nBins, double min, double max)
    : m_min(min)
    , m_max(max)
  {
    m_hist.resize(nBins, 0);
  }

  void forPoints(const smtk::mesh::HandleRange& pointIds, std::vector<double>& xyz, bool&) override
  {
    typedef smtk::mesh::HandleRange::const_iterator c_it;
    int counter = 0;
    for (c_it i = pointIds.begin(); i != pointIds.end(); ++i, counter += 3)
    {
      std::size_t bin = xyz[counter + 2] < m_min ? 0
        : xyz[counter + 2] > m_max
        ? m_hist.size() - 1
        : static_cast<std::size_t>((xyz[counter + 2] - m_min) / (m_max - m_min) * m_hist.size());
      ++m_hist[bin];
    }
  }

  const std::vector<std::size_t>& histogram() const { return m_hist; }

private:
  std::vector<std::size_t> m_hist;
  double m_min;
  double m_max;
};
} // namespace

int main(int argc, char* argv[])
{
  if (argc == 1)
  {
    std::cout << "TestRuntimePyOperation <path/to/file.py>" << std::endl;
    return 1;
  }

  ::boost::filesystem::path path(argv[1]);
  if (!::boost::filesystem::is_regular_file(path))
  {
    std::cout << "Cannot find " << argv[1] << std::endl;
    return 1;
  }

  smtk::model::ResourcePtr resource = smtk::model::Resource::create();
  smtk::mesh::ManagerPtr meshManager = resource->meshes();

  std::cout << "Available sessions\n";
  smtk::model::StringList sessions = resource->sessionTypeNames();
  for (smtk::model::StringList::iterator it = sessions.begin(); it != sessions.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::session::polygon::Session::Ptr session = smtk::session::polygon::Session::create();
  resource->registerSession(session);

  std::cout << "Available operators\n";
  smtk::model::StringList opnames = session->operatorNames();
  for (smtk::model::StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  {
    smtk::operation::Operation::Ptr op = session->op("import python operator");

    op->findFile("filename")->setValue(::boost::filesystem::absolute(path).string().c_str());

    smtk::operation::OperationResult result = op->operate();
    if (result->findInt("outcome")->value() != smtk::operation::Operation::OPERATION_SUCCEEDED)
    {
      std::cerr << "Could not load smtk model!\n";
      return 1;
    }
  }

  smtk::model::Model model;
  {
    std::string file_path(data_root);
    file_path += "/mesh/2d/boxWithHole.smtk";

    std::ifstream file(file_path.c_str());
    if (file.good())
    { //just make sure the file exists
      file.close();

      smtk::operation::Operation::Ptr op = session->op("load smtk model");

      op->findFile("filename")->setValue(file_path.c_str());
      smtk::operation::OperationResult result = op->operate();
      if (result->findInt("outcome")->value() != smtk::operation::Operation::OPERATION_SUCCEEDED)
      {
        std::cerr << "Could not load smtk model!\n";
        return 1;
      }
      model = result->findModelEntity("mesh_created")->value();
    }
  }

  smtk::mesh::ResourcePtr meshResource;
  {
    smtk::model::EntityRefs currentEnts =
      resource->entitiesMatchingFlagsAs<smtk::model::EntityRefs>(smtk::model::FACE);
    removeRefsWithoutTess(currentEnts);
    // We only extract the first face
    smtk::model::EntityRef eRef = *currentEnts.begin();

    const smtk::model::Face& face = eRef.as<smtk::model::Face>();

    if (!face.isValid())
    {
      std::cerr << "Face is invald\n";
      return 1;
    }

    smtk::operation::OperationPtr triangulateFace = session->op("triangulate faces");
    if (!triangulateFace)
    {
      std::cerr << "No triangulate face operator\n";
      return 1;
    }
    triangulateFace->specification()->associateEntity(face);
    smtk::operation::OperationResult result = triangulateFace->operate();
    auto associatedResources = meshManager->associatedCollections(face);
    meshResource = associatedResources[0];
  }

  // histogram non-elevated coordinate values
  {
    ValidatePoints validatePoints(5, -1., 1.);
    smtk::mesh::for_each(meshResource->meshes().points(), validatePoints);
    std::size_t valid[5] = { 0, 0, 8, 0, 0 };

    for (std::size_t bin = 0; bin < 5; bin++)
    {
      test(validatePoints.histogram()[bin] == valid[bin]);
    }
  }

  {
    smtk::operation::Operation::Ptr op = session->op("my elevate mesh");

    op->findMesh("mesh")->setValue(meshResource->meshes());
    smtk::operation::OperationResult result = op->operate();
    if (result->findInt("outcome")->value() != smtk::operation::Operation::OPERATION_SUCCEEDED)
    {
      std::cerr << "Could not run \"my elevate mesh\"!\n";
      return 1;
    }
  }

  // histogram elevated coordinate values
  {
    ValidatePoints validatePoints(5, -1., 1.);
    smtk::mesh::for_each(meshResource->meshes().points(), validatePoints);
    std::size_t valid[5] = { 2, 0, 4, 0, 2 };

    for (std::size_t bin = 0; bin < 5; bin++)
    {
      test(validatePoints.histogram()[bin] == valid[bin]);
    }
  }

  return 0;
}

smtkComponentInitMacro(smtk_delaunay_triangulate_faces_operator);
