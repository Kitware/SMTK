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

// #define SINGLE_THREAD

namespace smtk
{
namespace extension
{
std::future<smtk::operation::Operation::Result> qtOperationLauncher::operator()(
  const smtk::operation::Operation::Ptr& op)
{
  // Construct a promise to pass to the subthread. Its associated future is the
  // output of this method.
  std::promise<smtk::operation::Operation::Result> promise;

  // Access the promise's associated future before moving the promise onto the
  // subthread.
  std::future<smtk::operation::Operation::Result> future = promise.get_future();

#ifdef SINGLE_THREAD

  // Execute the operation in the subthread.
  auto result = op->operate();

  // Set the promise to the output result.
  promise.set_value(result);

#else

  // Construct a thread to execute the operation. It takes:
  //
  // 1. a pointer to this class so its emitted signal can be accessed by our
  //    associated slot,
  // 2. a shared pointer to the operation by value to prevent the underlying
  //    operation from being changed before the operation completes, and
  // 3. the associated promise to this method's returned future.
  std::thread thread(&qtOperationLauncher::run, this, op, std::move(promise));

  // Construct a copy of the operation shared pointer to pass to the subsequent
  // lambda to prevent the underlying operation from being changed before the
  // operation completes.
  smtk::operation::Operation::Ptr operation = op;

  // When the subthread completes the operation, it emits a private signal
  // containing the name of the result. We pass the result name rather than the
  // result itself because QStrings are designed to pass from signals to slots
  // across threads without registration with qMetaData. Given this class
  // instance as a context, we associate the subthread's signal with a lambda
  // that accesses the result of the operation and emits it on the thread
  // associated with this object. Each connection we make is a one-shot
  // associated to the input operation, so we delete the connection upon firing.
  QMetaObject::Connection* connection = new QMetaObject::Connection;
  *connection = QObject::connect(this, &qtOperationLauncher::operationHasResult, this,
    [&, connection, operation](QString resultName) {
      auto result = operation->specification()->findAttribute(resultName.toStdString());
      this->resultReady(result);

      // Remove this connection.
      QObject::disconnect(*connection);
      delete connection;
    });

  // We have the future result and we have made the one-shot connection to the
  // output. We no longer need to track this thread, so we detach it.
  thread.detach();

#endif

  // Return the future associated with the promise created above.
  return future;
}

void qtOperationLauncher::run(smtk::operation::Operation::Ptr operation,
  std::promise<smtk::operation::Operation::Result>&& promise)
{
  // Execute the operation in the subthread.
  auto result = operation->operate();

  // Set the promise to the output result.
  promise.set_value(result);

  // Privately emit the name of the output result so the contents of this class
  // that reside on the original thread can access it.
  emit operationHasResult(QString::fromStdString(result->name()), QPrivateSignal());
}
}
}
