//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// Implement an SMTK remote model worker.
// Steal code from CMB's vtkModelManagerWrapper.
#include "smtk/bridge/remote/RemusRemoteBridge.h"
#include "smtk/bridge/remote/RemusRPCWorker.h"

#include "smtk/model/Manager.h"

#include "smtk/io/ExportJSON.h"

#include "remus/worker/Worker.h"

using namespace smtk::model;
using namespace remus::meshtypes;
using namespace remus::proto;

#include "smtk/AutoInit.h"
#include "smtk/Options.h"

#include "cJSON.h"

#include "clpp/parser.hpp"

#include <fstream>

#ifdef __APPLE__
#  include "TargetConditionals.h"
#endif

#ifdef SMTK_BUILD_CGM
smtkComponentInitMacro(smtk_cgm_bridge);
smtkComponentInitMacro(smtk_cgm_read_operator);
// FIXME: Use conditionals to enable these only when the underlying CGM library supports them:
//        At worst, this source could depend on list-cgm-engines, which provides a list.
//        At best, FindCGM could provide CMake variables to be included in smtk/options.h.in.
#include "smtk/bridge/cgm/Engines.h"
smtkRegisterBridgeWithRemus("cgm", smtk::bridge::cgm::Engines::setDefault("ACIS"),  "smtk::model[cgm{ACIS}]", CGM_ACIS);
smtkRegisterBridgeWithRemus("cgm", smtk::bridge::cgm::Engines::setDefault("cubit"), "smtk::model[cgm{Cubit}]", CGM_Cubit);
smtkRegisterBridgeWithRemus("cgm", smtk::bridge::cgm::Engines::setDefault("OCC"),   "smtk::model[cgm{OpenCascade}]", CGM_OpenCascade);
smtkRegisterBridgeWithRemus("cgm", smtk::bridge::cgm::Engines::setDefault("facet"), "smtk::model[cgm{Cholla}]", CGM_Cholla);
#endif // SMTK_BUILD_CGM

int usage(
  int ecode = 0, const std::string& msg = std::string())
{
  // I. Basic usage info.
  std::cout
    << "Usage:\n"
    << "\tsmtk-model-worker <url> [options]      to accept jobs\n"
    << "\tsmtk-model-worker -generate [options]  to generate a worker description file\n"
    << "where\n"
    << "  -generate          creates a Remus worker description(s) and exits.\n"
    << "  -server=<url>      specifies the remus server to user, tcp://localhost:50510 by default.\n"
    << "                     If this is the first argument, you may specify just <url> without -server=.\n"
    << "  -rwfile=<file>     specifies the base filename storing the generated worker description.\n"
    << "                     With -generate, <file> also serves as the base for the requirements filename.\n"
    << "  -kernel=<kern>     specifies the name an SMTK modeling kernel.\n"
    << "  -engine=<engine>   specifies an engine the SMTK modeling kernel should use.\n"
    << "  -root=<dir>        specifies the directory the worker should make available for I/O.\n"
    << "  -site=<site>       specifies the filesystem/host site name.\n"
    << "\n"
    << "Examples:\n"
    << "  smtk-model-worker -generate \\\n"
    << "    -kernel=cgm -engine=OpenCascade \\\n"
    << "    -root=/var/smtk/data -site=foo \\\n"
    << "    -rwfile=/var/smtk/workers/foo.RW\n"
    << "will create 2 files named /var/smtk/workers/foo.RW (for the Remus\n"
    << "Worker description) and /var/smtk/workers/foo.RW.requirements (for\n"
    << "the worker requirements). The RW file specifies how to start a worker\n"
    << "to run a CGM modeling kernel with OpenCascade as the default engine.\n"
    << "The worker will be able to read and write files to /var/smtk/data\n"
    << "and will advertise its mesh type as \"smtk::model[cgm{OpenCascade}@foo]\"\n"
    << "\n"
    << "  smtk-model-worker tcp://localhost:50505 -rwfile=/var/smtk/workers/foo.RW\n"
    << "will start serving model requests using the description generated above.\n"
    << "\n"
    ;

  // II. List available modeling kernels.
  std::cout << "Valid <kern> values are:\n";
  StringList allKernels =
    smtk::bridge::remote::RemusRemoteBridge::availableTypeNames();
  for (StringList::iterator kit = allKernels.begin(); kit != allKernels.end(); ++kit)
    if (*kit != "smtk::model[native]") // Do not allow "native" unbacked models, for now.
      std::cout << "  " << *kit << "\n";
  std::cout << "\n";

  // III. Print user-specified message and return exit code.
  if (!msg.empty())
    std::cout << msg << "\n";

  return ecode;
}

// A struct to hold options passed to the model worker.
struct WkOpts
{
  WkOpts()
    : m_gen(false)
    {
    }

  void setServer(const std::string& url) { this->m_url = url; }
  void setKernel(const std::string& kern) { this->m_kern = kern; }
  void setEngine(const std::string& engine) { this->m_engine = engine; }
  void setSite(const std::string& site) { this->m_site = site; }
  void setRoot(const std::string& root) { this->m_root = root; }
  void setRWFile(const std::string& rwfile) { this->m_rwfile = rwfile; }
  void setGenerate() { this->m_gen = true; }
  void setWorkerPath(const std::string& wpath) { this->m_wpath = wpath; }

  std::string serverURL() const { return this->m_url; }
  std::string root() const { return this->m_root; }
  std::string rwfile() const { return this->m_rwfile; }
  std::string site() const { return this->m_site; }
  std::string kernel() const { return this->m_kern; }
  std::string engine() const { return this->m_engine; }
  std::string workerPath() const { return this->m_wpath; }
  std::string meshType() const
    {
    std::ostringstream mt;
    mt << "smtk::model[" << this->kernel();
    if (!this->engine().empty())
      mt << "{" << this->engine() << "}";
    mt << "]";
    return mt.str();
    }
  std::string workerName() const
    {
    std::ostringstream mt;
    mt << "smtk::model[" << this->kernel();
    if (!this->engine().empty())
      mt << "{" << this->engine() << "}";
    if (!this->site().empty())
      mt << "@" <<  this->site();
    mt << "]";
    return mt.str();
    }
  bool generate() const { return this->m_gen; }

  std::string m_url;
  std::string m_site;
  std::string m_kern;
  std::string m_engine;
  std::string m_root;
  std::string m_rwfile;
  std::string m_wpath;
  bool m_gen;
};

int main(int argc, char* argv[])
{
  using namespace smtk::bridge;
  WkOpts wkOpts;
  wkOpts.setWorkerPath(argv[0]);
  clpp::command_line_parameters_parser args;
  try
    {
    args.add_parameter("-server",   &wkOpts, &WkOpts::setServer).default_value("tcp://localhost:50510").order(1);
    args.add_parameter("-rwfile",   &wkOpts, &WkOpts::setRWFile).default_value("smtk-model-worker.RW");
    args.add_parameter("-root",     &wkOpts, &WkOpts::setRoot);
    args.add_parameter("-site",     &wkOpts, &WkOpts::setSite);
    args.add_parameter("-kernel",   &wkOpts, &WkOpts::setKernel);
    args.add_parameter("-engine",   &wkOpts, &WkOpts::setEngine);
    args.add_parameter("-generate", &wkOpts, &WkOpts::setGenerate);
    args.parse(argc, argv);
    }
  catch (std::exception& e)
    {
    return usage(1, e.what());
    }

  remus::worker::ServerConnection connection =
    remus::worker::make_ServerConnection(wkOpts.serverURL());

  // I. Advertise a "handshake" worker for the type of kernel requested.
  //    The RemusRPCWorker instance will swap it out for one with a
  //    session tag once a session is started.

  remote::RemusModelBridgeType other =
    remote::RemusRemoteBridge::findAvailableType(wkOpts.meshType());
  if (!other)
    {
    return usage(1, "kernel \"" + wkOpts.meshType() + "\" was unknown.");
    }

  // Perform any prep work required so that new bridge sessions create
  // kernel sessions of the proper type (e.g., set the default CGM engine).
  other->bridgePrep();

  // Every worker shares the same output type ("smtk::model[native]")
  // since that is the wire format SMTK uses:
  remus::common::MeshIOType io_type(other->name(), "smtk::model[native]");

  // TODO: requirements should list available "storage" UUIDs.
  //       Tag should exist and be a JSON string with hostname and a host UUID
  //       (but not a session ID yet).
  JobRequirements requirements = make_JobRequirements(io_type, wkOpts.workerName(), "");
  if (wkOpts.generate())
    {
    // Create bridge session and serialize operators.
    smtk::model::Manager::Ptr mgr = smtk::model::Manager::create();
    smtk::model::Bridge::Ptr bridge = mgr->createAndRegisterBridge(wkOpts.kernel());
    smtk::attribute::System* opsys = bridge->operatorSystem();
    cJSON* spec = cJSON_CreateObject();
    std::string opspec;
    if (smtk::io::ExportJSON::forOperatorDefinitions(opsys, spec))
      opspec = cJSON_GetObjectItem(spec, "ops")->valuestring;
    cJSON_Delete(spec);
    requirements = make_JobRequirements(
      io_type, wkOpts.workerName(), opspec.c_str(), remus::common::ContentFormat::JSON);

    // Write the model operator attribute system as the job requirements:
    std::string reqFileName = wkOpts.rwfile() + ".requirements";
    std::ofstream reqFile(reqFileName);
    reqFile << opspec;
    reqFile.close();

    // Generate the Remus Worker (.RW) file.
    cJSON* desc = cJSON_CreateObject();
    // FIXME: workerPath() should return path of worker RELATIVE TO RWFile!
    if (smtk::io::ExportJSON::forModelWorker(
        desc, io_type.inputType(), io_type.outputType(),
        wkOpts.kernel(), wkOpts.engine(),
        wkOpts.site(), wkOpts.root(),
        wkOpts.workerPath(), reqFileName))
      {
      // Now handle platform-specific environment settings that we may
      // need to preserve.
      std::string libsearchpath_name;
      std::string fallbacklibsearchpath_name;
      std::string pythonpath_name = "PYTHONPATH";
      std::string libsearchpath;
      std::string fallbacklibsearchpath;
      std::string pythonpath;
      cJSON* descEnv = cJSON_CreateObject();
      char* buf;
#ifndef _WIN32
#  ifdef __APPLE__
      libsearchpath_name = "DYLD_LIBRARY_PATH";
      fallbacklibsearchpath_name = "DYLD_FALLBACK_LIBRARY_PATH";
      buf = getenv(fallbacklibsearchpath_name.c_str());
      if (buf && buf[0])
        fallbacklibsearchpath = buf;
#  else
      libsearchpath_name = "LD_LIBRARY_PATH";
#  endif
      buf = getenv(libsearchpath_name.c_str());
      if (buf && buf[0])
        libsearchpath = buf;
      buf = getenv(pythonpath_name.c_str());
      if (buf && buf[0])
        pythonpath = buf;
#else
#  ifdef __CYGWIN__
#  else
      const bool valid;

      libsearchpath_name = "PATH";
      valid = (_dupenv_s(&buf, NULL, libsearchpath_name.c_str()) == 0) && (buf != NULL);
      if (valid)
        libsearchpath = buf;
      free(buf); //perfectly valid to free a NULL pointer

      valid = (_dupenv_s(&buf, NULL, pythonpath_name.c_str()) == 0) && (buf != NULL);
      if (valid)
        pythonpath = buf;
      free(buf); //perfectly valid to free a NULL pointer
#  endif
#endif
      bool anyEnv = false;
      if (!libsearchpath.empty())
        {
        anyEnv = true;
        cJSON_AddItemToObject(
          descEnv,
          libsearchpath_name.c_str(),
          cJSON_CreateString(libsearchpath.c_str()));
        }
      if (!fallbacklibsearchpath.empty())
        {
        anyEnv = true;
        cJSON_AddItemToObject(
          descEnv,
          fallbacklibsearchpath_name.c_str(),
          cJSON_CreateString(fallbacklibsearchpath.c_str()));
        }
      if (!pythonpath.empty())
        {
        anyEnv = true;
        cJSON_AddItemToObject(
          descEnv,
          pythonpath_name.c_str(),
          cJSON_CreateString(pythonpath.c_str()));
        }
      if (anyEnv)
        cJSON_AddItemToObject(desc, "Environment", descEnv);
      else
        cJSON_Delete(descEnv);

      std::ofstream workerFile(wkOpts.rwfile());
      char* descStr = cJSON_Print(desc);
      workerFile << descStr << "\n";
      workerFile.close();
      free(descStr);
      }
    cJSON_Delete(desc);

    std::cout << "\n\nWrote " << wkOpts.rwfile() << "\n\n";
    return 0; // Do not wait for jobs.
    }

  remus::Worker* w = new remus::Worker(requirements,connection);

  remote::RemusRPCWorker::Ptr smtkWorker = remote::RemusRPCWorker::create();
  while (true)
    {
    std::cerr << "Waiting for job\n";
    remus::worker::Job jobdesc = w->getJob();
    switch (jobdesc.validityReason())
      {
    case remus::worker::Job::TERMINATE_WORKER:
      std::cerr << "Told to exit. Exiting.\n";
      return 0;
    case remus::worker::Job::INVALID:
      std::cerr << "  Skipping invalid job \"" << jobdesc.id() << "\"\n";
      continue; // skip to the next job.
      break;
    case remus::worker::Job::VALID_JOB:
    default:
      break;
      }
    std::cout << "  Got job\n";

    smtkWorker->processJob(w, jobdesc, requirements);

    std::cout << "  Job complete\n";
    }

  delete w;
  return 0;
}
