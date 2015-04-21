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
#ifndef SHIBOKEN_SKIP
#include "smtk/bridge/remote/Session.h"
#include "smtk/bridge/remote/RemusRPCWorker.h"

#include "smtk/common/Environment.h"

#include "smtk/model/Manager.h"

#include "smtk/io/ExportJSON.h"
#include "smtk/io/Logger.h"

#include "remus/worker/Worker.h"

using namespace smtk::model;
using namespace smtk::common;
using namespace remus::meshtypes;
using namespace remus::proto;

#include "smtk/Options.h"

#include "cJSON.h"

#ifndef _MSC_VER
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored"-Wshadow"
#endif
#include "clpp/parser.hpp"
#ifndef _MSC_VER
#  pragma GCC diagnostic pop
#endif

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

// ++ UserGuide/Model/1 ++
#include "smtk/AutoInit.h"

#ifdef SMTK_BUILD_CGM
smtkComponentInitMacro(smtk_cgm_session);
#include "smtk/bridge/cgm/Engines.h"
#endif // SMTK_BUILD_CGM

#ifdef SMTK_BUILD_DISCRETE_SESSION
smtkComponentInitMacro(smtk_discrete_session);
#endif // SMTK_BUILD_DISCRETE_SESSION
// -- UserGuide/Model/1 --

int usage(
  smtk::io::Logger& logr,
  int ecode = 0,
  const std::string& msg = std::string())
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
  StringList allKernels = smtk::model::SessionRegistrar::sessionTypeNames();
  for (StringList::iterator kit = allKernels.begin(); kit != allKernels.end(); ++kit)
    if (*kit != "native") // Do not allow "native" unbacked models, for now.
      std::cout << "  " << *kit << "\n";
  std::cout << "\n";

  // III. Print user-specified message and return exit code.
  if (!msg.empty())
    {
    std::cout << msg << "\n";
    smtkInfoMacro(logr, "Usage: " << msg);
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
  smtk::model::Manager::Ptr mgr = smtk::model::Manager::create();
  smtk::io::Logger& logr(mgr->log());

  logr.setFlushToFile("wlog.txt", true);
  smtkInfoMacro(logr, "Starting model worker with args:");
  for (int i = 1; i < argc; ++i)
    smtkInfoMacro(logr, "   " << i << ": " << argv[i]);

  using namespace smtk::bridge;
  WkOpts wkOpts;
  wkOpts.setWorkerPath(argv[0]);
  clpp::command_line_parameters_parser args;
  try
    {
    args
      .add_parameter("-server",     &wkOpts, &WkOpts::setServer)
      .default_value(std::string("tcp://localhost:50510"))
      .order(1);
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
    smtkInfoMacro(logr, "  Exception " << e.what());
    return usage(logr, 1, e.what());
    }
  if (wkOpts.printHelp())
    {
    smtkInfoMacro(logr, "  Help");
    return usage(logr, 0);
    }

  smtkInfoMacro(logr, "  Chdir");
  if (!wkOpts.root().empty())
    {
    if (smtkChDir(wkOpts.root()))
      {
      return usage(logr, 1,
        "Unable to change to directory \"" + wkOpts.root() + "\"");
      }
    }

  smtkInfoMacro(logr, "  About to connect to " << wkOpts.serverURL());
  remus::worker::ServerConnection connection =
    remus::worker::make_ServerConnection(wkOpts.serverURL());
  smtkInfoMacro(logr, "  Server " << wkOpts.serverURL());

  // I. Advertise a "handshake" worker for the type of kernel requested.
  //    The RemusRPCWorker instance will swap it out for one with a
  //    session tag once a session is started.

  // Every worker shares the same output type ("smtk::model[native]")
  // since that is the wire format SMTK uses:
  remus::common::MeshIOType io_type(wkOpts.meshType(), "smtk::model[native]");

  // TODO: requirements should list available "storage" UUIDs.
  //       Tag should exist and be a JSON string with hostname and a host UUID
  //       (but not a session ID yet).
  JobRequirements requirements = make_JobRequirements(io_type, wkOpts.workerName(), "");
  smtkInfoMacro(logr, "  Worker name: " << wkOpts.workerName());
  std::cout << "Worker iotype " << io_type.inputType() << "->" << io_type.outputType() << "\n";
  if (wkOpts.generate())
    {
    if (wkOpts.rwfile().empty())
      {
      return usage(logr, 1, "Remus worker filename not specifed or invalid.");
      }
    // Create session and serialize operators.
    smtk::model::Session::Ptr session = mgr->createSession(wkOpts.kernel()).session();
    if (!session)
      return usage(logr, 1, "Could not create session \"" + wkOpts.kernel() + "\"");
    smtk::attribute::System* opsys = session->operatorSystem();
    cJSON* spec = cJSON_CreateObject();
    std::string opspec;
    if (smtk::io::ExportJSON::forOperatorDefinitions(opsys, spec))
      opspec = cJSON_GetObjectItem(spec, "ops")->valuestring;
    cJSON_Delete(spec);
    requirements = make_JobRequirements(
      io_type, wkOpts.workerName(), opspec.c_str(), remus::common::ContentFormat::XML);

    // Write the model operator attribute system as the job requirements:
    std::string reqFileName = wkOpts.rwfile() + ".requirements";
    std::ofstream reqFile(reqFileName.c_str());
    reqFile << opspec;
    reqFile.close();

    // Generate the Remus Worker (.RW) file.
    cJSON* desc = cJSON_CreateObject();
    // FIXME: workerPath() should return path of worker RELATIVE TO RWFile!
    if (smtk::io::ExportJSON::forModelWorker(
        desc, io_type.inputType(), io_type.outputType(),
        session, wkOpts.engine(),
        wkOpts.site(), wkOpts.root(),
        wkOpts.workerPath(), reqFileName))
      {
      // Set the worker name to match the one we'll compute when given
      // the site specified on the command line. This may change in the future.
      cJSON_AddItemToObject(desc,
        "WorkerName", cJSON_CreateString(wkOpts.workerName().c_str()));
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

      std::ofstream workerFile(wkOpts.rwfile().c_str());
      char* descStr = cJSON_Print(desc);
      workerFile << descStr << "\n";
      workerFile.close();
      free(descStr);
      }
    cJSON_Delete(desc);

    smtkInfoMacro(logr, "Wrote " << wkOpts.rwfile() << "\n      " << reqFileName);
    std::cout << "\n\nWrote " << wkOpts.rwfile() << "\n      " << reqFileName << "\n\n";
    return 0; // Do not wait for jobs.
    }

  remote::RemusRPCWorker::Ptr smtkWorker = remote::RemusRPCWorker::create();
  smtkWorker->setManager(mgr);
  if (!wkOpts.rwfile().empty())
    { // Configure the smtkWorker
    std::ifstream rwFile(wkOpts.rwfile().c_str());
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
    // If requirements were specified in a file change our requirements to match.
    opt = cJSON_GetObjectItem(config, "File");
    if (opt && opt->valuestring && opt->valuestring[0])
      requirements = make_JobRequirements(
        io_type, wkOpts.workerName(),
        remus::common::FileHandle(opt->valuestring),
        remus::common::ContentFormat::XML); // FIXME: Read FileFormat from RW file and use it.

    char* tagchar = cJSON_PrintUnformatted(tag);
    requirements.tag(tagchar);
    free(tagchar);
    }
  smtkInfoMacro(logr, "Requirements tag is \"" << requirements.tag() << "\"");

  // Register the requirements mesh type as the special session name advertised via Remus.
  SessionStaticSetup bsetup = SessionRegistrar::sessionStaticSetup(wkOpts.kernel());
  SessionConstructor bctor = SessionRegistrar::sessionConstructor(wkOpts.kernel());
  if (!bctor)
    {
    return usage(logr, 1, "Unable to obtain constructor for kernel \"" + wkOpts.kernel() + "\"");
    }
  smtk::model::SessionRegistrar::registerSession(
    wkOpts.workerName(), requirements.tag(), bsetup, bctor);

  remus::Worker* w = new remus::Worker(requirements,connection);
  while (true)
    {
    smtkInfoMacro(logr, "Waiting for job");
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
    smtkInfoMacro(logr, "Got job");
    std::cout << "  Got job\n";

    smtkWorker->processJob(w, jobdesc, requirements);

    smtkInfoMacro(logr, "Job complete");
    std::cout << "  Job complete\n";
    }

  delete w;
  return 0;
}
#endif // SHIBOKEN_SKIP
