// Implement an SMTK remote model worker.
// Steal code from CMB's vtkModelManagerWrapper.
#include "smtk/bridge/remote/RemusRemoteBridge.h"
#include "smtk/bridge/remote/RemusRPCWorker.h"

#include "remus/worker/Worker.h"

using namespace smtk::model;
using namespace remus::meshtypes;
using namespace remus::proto;

#include "smtk/AutoInit.h"
#include "smtk/options.h"

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
  int argc, char* argv[],
  const std::string& msg = std::string(), int ecode = 0)
{
  // I. Basic usage info.
  std::cout
    << "Usage:\n"
    << "\t" << argv[0] << " serverURL modelingKernel\n"
    << "where\n"
    << "  serverURL      is the URL of a remus server.\n"
    << "  modelingKernel is the name an SMTK modeling kernel.\n"
    << "\n";

  // II. List available modeling kernels.
  std::cout << "Valid modelKernel values are:\n";
  StringList allKernels =
    smtk::bridge::remote::RemusRemoteBridge::availableTypeNames();
  for (StringList::iterator kit = allKernels.begin(); kit != allKernels.end(); ++kit)
    if (*kit != "smtk::model[native]") // Do not allow "native" unbacked models, for now.
      std::cout << "  " << *kit << "\n";
  std::cout << "\n";

  // III. Print user-specified message and return exit code.
  if (!msg.empty())
    std::cout << msg << "\n";
  return argc < 3 ? -1 : ecode;
}

int main(int argc, char* argv[])
{
  using namespace smtk::bridge;
  if (argc < 3)
    return usage(argc, argv);

  std::string serverURL(argv[1]);
  std::string kernel(argv[2]);

  remus::worker::ServerConnection connection =
    remus::worker::make_ServerConnection(serverURL);

  // I. Advertise a "handshake" worker for the type of kernel requested.
  //    The RemusRPCWorker instance will swap it out for one with a
  //    session tag once a session is started.

  // Every worker shares the same output type ("smtk::model[native]")
  // since that is the wire format SMTK uses:
  remote::RemusModelBridgeType native =
    remote::RemusRemoteBridge::findAvailableType("smtk::model[native]");

  remote::RemusModelBridgeType other =
    remote::RemusRemoteBridge::findAvailableType(kernel);
  if (!other)
    {
    return usage(argc, argv, "modelingKernel \"" + kernel + "\" was unknown.", 1);
    }

  // Perform any prep work required so that new bridge sessions create
  // kernel sessions of the proper type (e.g., set the default CGM engine).
  other->bridgePrep();

  remus::common::MeshIOType io_type =
    remus::common::make_MeshIOType(*other.get(), *native.get());
  // TODO: requirements should list available "storage" UUIDs.
  //       Tag should exist and be a JSON string with hostname and a host UUID
  //       (but not a session ID yet).
  JobRequirements requirements = make_JobRequirements(io_type, "smtk::model", "");
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
