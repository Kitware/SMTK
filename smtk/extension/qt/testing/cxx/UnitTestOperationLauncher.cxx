//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/mesh/Resource.h"
#include "smtk/session/mesh/operators/Import.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/extension/qt/qtOperationLauncher.h"

#include "smtk/extension/qt/testing/cxx/UnitTestOperationLauncher.h"

#include "smtk/io/ExportMesh.h"

#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Tessellation.h"

#include "smtk/operation/Manager.h"

#include "QCoreApplication"
#include <QObject>
#include <QString>
#include <QThread>
#include <QTime>
#include <QTimer>

#include <future>

namespace
{
std::string dataRoot = SMTK_DATA_DIR;

void flushEventQueue()
{
  QTime dieTime = QTime::currentTime().addMSecs(500);
  while (QTime::currentTime() < dieTime)
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 500);
  }
}
} // namespace

// Get the qt Launcher
smtk::extension::qtOperationLauncher* TestOperationLauncher::GetLauncher()
{
  smtk::extension::qt::Launcher* launcher =
    m_operationManager->launchers()["myLauncher"].target<smtk::extension::qt::Launcher>();

  if (launcher == nullptr)
  {
    m_operationManager->launchers()["myLauncher"] = smtk::extension::qt::Launcher();
    launcher =
      m_operationManager->launchers()["myLauncher"].target<smtk::extension::qt::Launcher>();
  }

  assert(launcher != nullptr);
  return launcher->get();
}

// Open operation
std::vector<std::future<smtk::operation::Operation::Result>> TestOperationLauncher::OpenFiles(
  const std::vector<std::string>& files)
{
  std::vector<std::future<smtk::operation::Operation::Result>> futures;
  for (auto& file : files)
  {
    smtk::session::mesh::Import::Ptr importOp =
      m_operationManager->create<smtk::session::mesh::Import>();

    smtk::attribute::FileItem::Ptr fileItem = importOp->parameters()->findFile("filename");
    importOp->parameters()->findFile("filename")->setValue(file);

    // Get a launcher
    smtk::extension::qtOperationLauncher* launcher = GetLauncher();

    // Since this connection is made inside the loop, it needs to be a one-shot.
    // Remove the connection once the lambda expression is called.
    QMetaObject::Connection* connection = new QMetaObject::Connection;
    *connection = QObject::connect(
      launcher,
      &smtk::extension::qtOperationLauncher::resultReady,
      [this, file, connection](smtk::operation::Operation::Result result) {
        if (
          result->findInt("outcome")->value() !=
          static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
        {
          std::cerr << "Could not import resource at " << file << "\n";
        }
        else
        {
          ++m_nReadFiles;
        }
        // Remove this connection.
        QObject::disconnect(*connection);
        delete connection;
      });

    // Execute operation after the connection has been made and return future
    futures.push_back((*launcher)(importOp));
  }
  return futures;
}

void TestOperationLauncher::run()
{
  std::vector<std::string> testFiles = {
    { dataRoot + "/model/3d/exodus/SimpleReactorCore/SimpleReactorCore.exo",
      dataRoot + "/model/3d/genesis/gun-1fourth.gen",
      dataRoot + "/mesh/3d/cube.exo",
      dataRoot + "/mesh/3d/cube_with_hole.exo",
      dataRoot + "/model/3d/exodus/sx_bar_hex.exo" }
  };

  std::vector<std::future<smtk::operation::Operation::Result>> results = OpenFiles(testFiles);

  // Because we are driving this test programatically, we need to give Qt's
  // event queue time to flush. Qt's queued connections post their actions on
  // the context thread's event queue, and qtOperationLauncher relays the
  // completed status of an operation via a queued connection across threads.
  // If this were an actual application, I think the event queue would get
  // flushed whenever the application went idle.
  while (this->numberOfReadFiles() != testFiles.size())
  {
    flushEventQueue();
  }

  // Ensure that the compute threads have finished executing their operations.
  for (auto& result : results)
  {
    result.wait();
  }

  Q_EMIT finished();
}

int UnitTestOperationLauncher(int argc, char* argv[])
{
  QCoreApplication application(argc, argv);

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Register the mesh resource to the resource manager
  {
    resourceManager->registerResource<smtk::session::mesh::Resource>();
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register import operator to the operation manager
  {
    operationManager->registerOperation<smtk::session::mesh::Import>("smtk::session::mesh::Import");
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  // Create an instance of our test harness
  TestOperationLauncher* testOperationLauncher =
    new TestOperationLauncher(resourceManager, operationManager, &application);

  // Exit when the task signals finished
  QObject::connect(testOperationLauncher, &TestOperationLauncher::finished, [&application]() {
    QTimer::singleShot(0, &application, &QCoreApplication::quit);
  });

  // Run the task from the application event loop
  QTimer::singleShot(0, testOperationLauncher, SLOT(run()));

  return application.exec();
}
