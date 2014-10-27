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

#include "smtk/common/Environment.h"

#include "smtk/model/Manager.h"

#include "smtk/io/ExportJSON.h"

#include "remus/worker/Worker.h"

using namespace smtk::model;
using namespace smtk::common;
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

#if !defined(_WIN32) || defined(__CYGWIN__)
#  include <unistd.h>
int smtkChDir(const std::string& path) { return chdir(path.c_str()); }
#else
#  include <direct.h>
int smtkChDir(const std::string& path) { return _chdir(path.c_str()); }
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

#ifdef SMTK_BUILD_DISCRETE_BRIDGE
smtkComponentInitMacro(smtk_discrete_bridge);
#endif // SMTK_BUILD_DISCRETE_BRIDGE

std::ofstream logr("/tmp/wlog.txt", std::ios::app);

int usage(
  int ecode = 0, const std::string& msg = std::string())
{
  // I. Basic usage info.
  std::cout
    << "Usage:\n"
    << "\tsmtk-model-worker <url> [options]      to accept jobs\n"
    << "\tsmtk-model-worker -generate [options]  to generate a worker description file\n"
    << "where\n"
    << "  -generate          Create a Remus worker description(s) and exits.\n"
    << "  -server=<url>      Specify the remus server to user. By default,\n"
    << "                     this is tcp://localhost:50510. If this is the\n"
    << "                     first argument, you may specify just <url>\n"
    << "                     without \"-server=\" in front.\n"
    << "  -rwfile=<file>     Specify the base filename storing the generated\n"
    << "                     worker description. With -generate, <file> also\n"
    << "                     serves as the base for the requirements filename.\n"
    << "  -kernel=<kern>     Specify the name an SMTK modeling kernel.\n"
    << "  -engine=<engine>   Specify an engine the SMTK modeling kernel should use.\n"
    << "  -root=<dir>        Specify the directory the worker should make\n"
    << "                     available for reading and writing model files.\n"
    << "  -site=<site>       Specify the filesystem/host site name.\n"
    << "  -help              Print this message and exit.\n"
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
    << "  smtk-model-worker tcp://localhost:50505 \\\n"
    << "    -rwfile=/var/smtk/workers/foo.RW\n"
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
    {
    std::cout << msg << "\n";
    logr << "  Usage: " << msg << "\n";
    }

  return ecode;
}

// A struct to hold options passed to the model worker.
struct WkOpts
{
  WkOpts()
    : m_gen(false), m_printhelp(false)
    {
    }

  void setPrintHelp() { this->m_printhelp = true; }
  void setServer(const std::string& url) { this->m_url = url; }
  void setKernel(const std::string& kern) { this->m_kern = kern; }
  void setEngine(const std::string& engine) { this->m_engine = engine; }
  void setSite(const std::string& site) { this->m_site = site; }
  void setRoot(const std::string& root) { this->m_root = root; }
  void setRWFile(const std::string& rwfile) { this->m_rwfile = rwfile; }
  void setGenerate() { this->m_gen = true; }
  void setWorkerPath(const std::string& wpath) { this->m_wpath = wpath; }

  bool printHelp() const { return this->m_printhelp; }
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
  bool m_printhelp;
};

int main(int argc, char* argv[])
{
  logr << "Starting model worker:";
  for (int i = 1; i < argc; ++i)
    logr << " " << argv[i];
  logr << "\n";

  using namespace smtk::bridge;
  WkOpts wkOpts;
  wkOpts.setWorkerPath(argv[0]);
  clpp::command_line_parameters_parser args;
  try
    {
    args.add_parameter("-server",   &wkOpts, &WkOpts::setServer).default_value("tcp://localhost:50510").order(1);
    args.add_parameter("-rwfile",   &wkOpts, &WkOpts::setRWFile);
    args.add_parameter("-root",     &wkOpts, &WkOpts::setRoot);
    args.add_parameter("-site",     &wkOpts, &WkOpts::setSite);
    args.add_parameter("-kernel",   &wkOpts, &WkOpts::setKernel);
    args.add_parameter("-engine",   &wkOpts, &WkOpts::setEngine);
    args.add_parameter("-generate", &wkOpts, &WkOpts::setGenerate);
    args.add_parameter("-help",     &wkOpts, &WkOpts::setPrintHelp);
    args.parse(argc, argv);
    }
  catch (std::exception& e)
    {
    logr << "  Exception " << e.what() << "\n";
    return usage(1, e.what());
    }
  if (wkOpts.printHelp())
    {
    logr << "  Help\n";
    return usage(0);
    }

  logr << "  Chroot\n";
  if (!wkOpts.root().empty())
    {
    if (smtkChDir(wkOpts.root()))
      {
      return usage(1,
        "Unable to change to directory \"" + wkOpts.root() + "\"");
      }
    }

  logr << "  About to connect to " << wkOpts.serverURL() << "\n";
  remus::worker::ServerConnection connection =
    remus::worker::make_ServerConnection(wkOpts.serverURL());
  logr << "  Server " << wkOpts.serverURL() << "\n";

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
  logr << "  Worker name: " << wkOpts.workerName() << "\n";
  std::cout << "Worker iotype " << io_type.inputType() << "->" << io_type.outputType() << "\n";
  if (wkOpts.generate())
    {
    if (wkOpts.rwfile().empty())
      {
      return usage(1, "Remus worker filename not specifed or invalid.");
      }
    // Create bridge session and serialize operators.
    smtk::model::Manager::Ptr mgr = smtk::model::Manager::create();
    smtk::model::Bridge::Ptr bridge = mgr->createAndRegisterBridge(wkOpts.kernel());
    if (!bridge)
      return usage(1, "Could not create bridge \"" + wkOpts.kernel() + "\"");
    smtk::attribute::System* opsys = bridge->operatorSystem();
    cJSON* spec = cJSON_CreateObject();
    std::string opspec;
    if (smtk::io::ExportJSON::forOperatorDefinitions(opsys, spec))
      opspec = cJSON_GetObjectItem(spec, "ops")->valuestring;
    cJSON_Delete(spec);
    requirements = make_JobRequirements(
      io_type, wkOpts.workerName(), opspec.c_str(), remus::common::ContentFormat::XML);

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
        bridge, wkOpts.engine(),
        wkOpts.site(), wkOpts.root(),
        wkOpts.workerPath(), reqFileName))
      {
      // Now handle platform-specific environment settings that we may
      // need to preserve.
      const std::string fallbacklibsearchpath_name = "DYLD_FALLBACK_LIBRARY_PATH";
      const std::string pythonpath_name = "PYTHONPATH";
      std::string fallbacklibsearchpath;
      std::string libsearchpath_name;
      std::string libsearchpath;
      std::string pythonpath;
      cJSON* descEnv = cJSON_CreateObject();
#ifndef _WIN32
#  ifdef __APPLE__
      libsearchpath_name = "DYLD_LIBRARY_PATH";
      fallbacklibsearchpath =
        Environment::getVariable(
          fallbacklibsearchpath_name);
#  else
      libsearchpath_name = "LD_LIBRARY_PATH";
#  endif
#else
      libsearchpath_name = "PATH";
#endif
      libsearchpath = Environment::getVariable(libsearchpath_name);
      pythonpath = Environment::getVariable(pythonpath_name);
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

    logr << "Wrote " << wkOpts.rwfile() << "\n      " << reqFileName << "\n";
    std::cout << "\n\nWrote " << wkOpts.rwfile() << "\n      " << reqFileName << "\n\n";
    return 0; // Do not wait for jobs.
    }

  remote::RemusRPCWorker::Ptr smtkWorker = remote::RemusRPCWorker::create();
  if (!wkOpts.rwfile().empty())
    { // Configure the smtkWorker
    std::ifstream rwFile(wkOpts.rwfile());
    std::string rwData(
      (std::istreambuf_iterator<char>(rwFile)),
      (std::istreambuf_iterator<char>()));
    cJSON* config = cJSON_Parse(rwData.c_str());
    if (!config)
      {
      std::cerr << "\n\nUnable to parse RemusWorker file " << wkOpts.rwfile() << "\n";
      return 1;
      }
    cJSON* tag = cJSON_GetObjectItem(config, "tag");
    if (!tag)
      {
      std::cerr << "\n\nUnable to find RemusWorker tag data\n";
      return 1;
      }
    const char* known_tag_options[] = {
      "default_kernel",
      "default_engine",
      "exclude_kernels",
      "exclude_engines",
      "site",
      NULL
    };
    const char* known_top_options[] = {
      "Root",
      NULL
    };
    cJSON* opt;
    const char** oname;
    for (oname = known_top_options; *oname; ++oname)
      {
      opt = cJSON_GetObjectItem(config, *oname);
      if (opt && opt->valuestring && opt->valuestring[0])
        smtkWorker->setOption(*oname, opt->valuestring);
      }
    for (oname = known_tag_options; *oname; ++oname)
      {
      opt = cJSON_GetObjectItem(tag, *oname);
      if (opt && opt->valuestring && opt->valuestring[0])
        smtkWorker->setOption(*oname, opt->valuestring);
      }
    char* tagchar = cJSON_PrintUnformatted(tag);
    requirements.tag(tagchar);
    free(tagchar);
    }
  logr << "Requirements tag is \"" << requirements.tag() << "\"\n";

  remus::Worker* w = new remus::Worker(requirements,connection);
  while (true)
    {
    logr << "Waiting for job\n"; logr.flush();
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
    logr << "  Got job\n"; logr.flush();
    std::cout << "  Got job\n";

    smtkWorker->processJob(w, jobdesc, requirements);

    logr << "  Job complete\n"; logr.flush();
    std::cout << "  Job complete\n";
    }

  delete w;
  return 0;
}
