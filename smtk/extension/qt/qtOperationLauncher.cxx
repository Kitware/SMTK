//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtOperationLauncher.h"

namespace smtk
{
namespace extension
{
std::shared_ptr<ResultHandler> qtOperationLauncher::operator()(
  const smtk::operation::Operation::Ptr& op)
{
  // Create Result Handler
  std::shared_ptr<ResultHandler> handler = std::make_shared<ResultHandler>();

// To enable SINGLE_THREAD, set CMake variable SMTK_ENABLE_OPERATION_THREADS to OFF.
#ifdef SINGLE_THREAD

  // Construct a promise to pass to the subthread. Its associated future is the
  // output of this method.
  std::promise<smtk::operation::Operation::Result> promise;

  // Access the promise's associated future before moving the promise onto the
  // subthread.
  std::shared_future<smtk::operation::Operation::Result> future = promise.get_future();

  // Set the result handler's future
  handler->m_future = future;

  // Execute the operation in the subthread.
  auto result = op->operate();

  // Set the promise to the output result.
  promise.set_value(result);

  /// Signal that the operation's result is ready for parsing.
  /// The emit should happen later because you'll miss the signal
  /// by the time this function returns. Passing a lamda to invokeMethod and
  /// using QueuedConnection will allow the emit to happen after the current
  /// path of execution.
  QMetaObject::invokeMethod(
    this, [handler, result]() { emit handler->resultReady(result); }, Qt::QueuedConnection);

#else

  // Construct a copy of the operation shared pointer to pass to the subsequent
  // lambda to prevent the underlying operation from being changed before the
  // operation completes.
  const smtk::operation::Operation::Ptr& operation = op;

  // When the subthread completes the operation, it emits a private signal
  // containing the name of the result. We pass the result name rather than the
  // result itself because QStrings are designed to pass from signals to slots
  // across threads without registration with qMetaData. Given this class
  // instance as a context, we associate the subthread's signal with a lambda
  // that accesses the result of the operation and emits it on the thread
  // associated with this object. Each connection we make is a one-shot
  // associated to the input operation, so we delete the connection upon firing.
  QMetaObject::Connection* connection = new QMetaObject::Connection;
  *connection = QObject::connect(
    this,
    &qtOperationLauncher::operationHasResult,
    this,
    [&, connection, operation, handler](QString parametersName, QString resultName) {
      if (parametersName.toStdString() == operation->parameters()->name())
      {
        auto result = operation->specification()->findAttribute(resultName.toStdString());
        emit handler->resultReady(result);

        // Remove this connection.
        QObject::disconnect(*connection);
        delete connection;
      }
    });

  handler->m_future = m_threadPool(&qtOperationLauncher::run, this, op);

#endif

  // Return the future associated with the promise created above.
  return handler;
}

smtk::operation::Operation::Result qtOperationLauncher::run(
  smtk::operation::Operation::Ptr operation)
{
  // Execute the operation in the subthread.
  auto result = operation->operate();

  // Privately emit the name of the output result so the contents of this class
  // that reside on the original thread can access it.
  emit operationHasResult(
    QString::fromStdString(operation->parameters()->name()),
    QString::fromStdString(result->name()),
    QPrivateSignal());

  return result;
}

ResultHandler::ResultHandler()
  : QObject(nullptr)
{
}

smtk::operation::Operation::Result ResultHandler::waitForResult()
{
  return m_future.get();
}
} // namespace extension
} // namespace smtk
