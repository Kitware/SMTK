//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/job/UpdateContainerQueueMachine.h"
#include "smtk/extension/qt/job/UpdateContainerQueueMachine_xml.h"

#include "smtk/extension/qt/job/ContainerQueue.h"

#include "smtk/job/Job.h"
#include "smtk/job/Queue.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/io/Logger.h"

#include <QProcess>
#include <QString>

namespace smtk
{
namespace qt
{
namespace job
{

UpdateContainerQueueMachine::Result UpdateContainerQueueMachine::operateInternal()
{
  auto params = this->parameters();
  auto queue = params->associations()->valueAs<smtk::qt::job::ContainerQueue>();
  if (!queue)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  bool ok = true;
  bool machineExists = false;
  bool needToRemove = false;
  bool isRunning = false;
  {
    // 0. List machines and see whether machine exists for this queue (and if it is running).
    QProcess proc;
    QStringList processArguments;
    proc.setProgram(QString::fromStdString(queue->engineExecutable().string()));
    processArguments << "machine"
                     << "inspect" << QString::fromStdString(queue->name());
    proc.setArguments(processArguments);
    proc.start();
    proc.waitForFinished(-1);
    ok = (proc.exitStatus() == QProcess::ExitStatus::NormalExit && proc.exitCode() == 0);
    if (ok)
    {
      auto machineInfo = nlohmann::json::parse(proc.readAllStandardOutput().toStdString());
      // std::cout << "inspected: " << machineInfo.dump(2) << "\n";
      // Check whether we need to recreate or start.
      if (machineInfo.empty())
      {
        machineExists = false;
        isRunning = false;
      }
      else
      {
        machineExists = true;
        isRunning = (machineInfo[0]["State"] == "Running");
      }
    }
    else
    {
      // Machine doesn't exist or podman command not found.
      // Assume podman was found but machine doesn't exist.
      machineExists = false;
      ok = true;
    }
  }

  if (!ok)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // If the root job directory doesn't match the existing machine, we need to remove
  // and re-create the machine.
  needToRemove = !queue->checkQueueRoot(queue->rootJobDirectory());

  // 1. Run "podman machine stop {queue->name()}" allowing failure
  // 2. Run "podman machine rm -f {queue->name()}" allowing failure
  // 3. Run "podman machine init -v {queue->rootJobDirectory()}:{queue->rootJobDirectory()} {queue->name()}" prohibiting failure
  // 4. Run "podman machine start {queue->name()}" prohibiting failure
  if (machineExists && needToRemove && isRunning)
  { // 1. Stop machine
    QProcess proc;
    QStringList processArguments;
    proc.setProgram(QString::fromStdString(queue->engineExecutable().string()));
    processArguments << "machine"
                     << "stop" << QString::fromStdString(queue->name());
    proc.setArguments(processArguments);
    proc.start();
    proc.waitForFinished(-1);
    // Ignore exit status and code.
    auto startLog = proc.readAllStandardOutput().toStdString();
    auto startErr = proc.readAllStandardError().toStdString();
    smtkWarningMacro(
      this->log(),
      "Stop machine\n\n"
        << startLog << "\nok=" << (ok ? "Y" : "N") << "\n\n"
        << startErr << "\n");
  }
  if (machineExists && needToRemove)
  { // 2. Remove machine
    machineExists = false;
    QProcess proc;
    QStringList processArguments;
    proc.setProgram(QString::fromStdString(queue->engineExecutable().string()));
    processArguments << "machine"
                     << "rm"
                     << "-f" << QString::fromStdString(queue->name());
    proc.setArguments(processArguments);
    proc.start();
    proc.waitForFinished(-1);
    // Ignore exit status and code.
    auto startLog = proc.readAllStandardOutput().toStdString();
    auto startErr = proc.readAllStandardError().toStdString();
    smtkWarningMacro(
      this->log(),
      "Remove machine\n"
        << startLog << "\nok=" << (ok ? "Y" : "N") << "\n\n"
        << startErr << "\n");
  }
  if (!machineExists)
  { // 3. Create machine
    isRunning = false;
    QProcess proc;
    QStringList processArguments;
    proc.setProgram(QString::fromStdString(queue->engineExecutable().string()));
    QString mp = queue->rootJobDirectoryAsString();
    QString mountSpec = mp + ":" + mp;
    std::cerr << "Creating maching with mount -v " << mountSpec.toStdString() << "\n";
    processArguments << "machine"
                     << "init"
                     << "-v" << mountSpec << QString::fromStdString(queue->name());
    proc.setArguments(processArguments);
    proc.start();
    proc.waitForFinished(-1);
    // If this fails, the job has failed and the queue is in a bad state.
    ok = (proc.exitStatus() == QProcess::ExitStatus::NormalExit && proc.exitCode() == 0);
    // std::cerr << proc.readAllStdout().toStdString() << "\n";
    if (ok)
    {
      queue->setMetadata("root_job_directory", queue->rootJobDirectory().string());
    }
    auto startLog = proc.readAllStandardOutput().toStdString();
    auto startErr = proc.readAllStandardError().toStdString();
    smtkWarningMacro(
      this->log(),
      "Create machine\n"
        << startLog << "\nok=" << (ok ? "Y" : "N") << "\n"
        << startErr << "\n");
  }
  if (ok && !isRunning)
  { // 4. Start machine
    QProcess proc;
    QStringList processArguments;
    proc.setProgram(QString::fromStdString(queue->engineExecutable().string()));
    processArguments << "machine"
                     << "start" << QString::fromStdString(queue->name());
    proc.setArguments(processArguments);
    proc.start();
    proc.waitForFinished(-1);
    // If this fails, the job has failed and the queue is in a bad state.
    ok = (proc.exitStatus() == QProcess::ExitStatus::NormalExit && proc.exitCode() == 0);
    auto startLog = proc.readAllStandardOutput().toStdString();
    auto startErr = proc.readAllStandardError().toStdString();
    if (!ok && startErr.find("already running") != std::string::npos)
    {
      ok = true;
    }
    smtkWarningMacro(
      this->log(),
      "Machine start\n"
        << startLog << "\nok=" << (ok ? "Y" : "N") << "\n\n"
        << startErr << "\n");
#if 0
    { // Debug printout of machine state
      QStringList processArguments2;
      processArguments2 << "machine" << "inspect" << QString::fromStdString(queue->name());
      proc.setArguments(processArguments2);
      proc.start();
      proc.waitForFinished(-1);
      startLog = proc.readAllStandardOutput().toStdString();
      std::cerr << "\n\n" << startLog << "\n\n\n";
    }
#endif // 0
  }

  // Set the queue as online (true) or offline (false):
  queue->setQueueOnline(ok);

  auto result = this->createResult(
    ok ? smtk::operation::Operation::Outcome::SUCCEEDED
       : smtk::operation::Operation::Outcome::FAILED);
  return result;
}

const char* UpdateContainerQueueMachine::xmlDescription() const
{
  return UpdateContainerQueueMachine_xml;
}
} // namespace job
} // namespace qt
} // namespace smtk
