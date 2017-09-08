//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/CompilerInformation.h"


SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <utility>

#include "smtk/extension/delaunay/worker/DelaunayMeshWorker.h"

#include "remus/common/LocateFile.h"
#include "remus/proto/JobRequirements.h"
#include "remus/common/MeshIOType.h"
#include "remus/common/ContentTypes.h"

#include "smtk/attribute/Collection.h"
#include "smtk/common/Paths.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

namespace py = pybind11;

namespace
{
  const std::vector<std::string>& relative_search_paths()
  {
    static std::vector<std::string> rel_search_paths = { "." };
    return rel_search_paths;
  }

  const std::vector<std::string> absolute_search_paths()
  {
    static smtk::common::Paths paths;
    return paths.workerSearchPaths();
  }
}

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindMeshWorkerDelaunay, delaunay)
{
  delaunay.doc() = "<description>";

  delaunay.def("start_worker", [](const std::string& dest){
      remus::worker::ServerConnection connection =
        remus::worker::make_ServerConnection(dest);
      remus::common::FileHandle rfile(
        remus::common::findFile("DelaunayMeshingDefs", "sbt",
                                relative_search_paths(),
                                absolute_search_paths()) );

      DelaunayMeshWorker worker( connection, rfile );
      worker.meshJob();
      return 0;
    });

  delaunay.def("job_requirements", [](){
      remus::proto::JobRequirements reqs =
        remus::proto::make_JobRequirements(
          remus::common::make_MeshIOType(remus::meshtypes::Model(),
                                         remus::meshtypes::Model()),
          std::string("DelaunayMeshWorker"),
          remus::common::findFile("DelaunayMeshingDefs", "sbt",
                                  relative_search_paths(),
                                  absolute_search_paths()),
          remus::common::ContentFormat::XML);
      std::stringstream s;
      s << reqs;
      return s.str();
    });

  delaunay.def("meshing_attributes", [](){
  smtk::attribute::CollectionPtr sysptr = smtk::attribute::Collection::create();
  smtk::io::Logger logger;
  smtk::io::AttributeReader reader;
  reader.read(sysptr,
              remus::common::findFile("DelaunayMeshingDefs", "sbt",
                                      relative_search_paths(),
                                      absolute_search_paths()).path(),
              logger);
  return sysptr;
    });
}
