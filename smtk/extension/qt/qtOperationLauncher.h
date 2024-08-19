//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtOperationLauncher_h
#define smtk_extension_qtOperationLauncher_h

#include "smtk/extension/qt/Exports.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"

#include "smtk/common/ThreadPool.h"

#include "smtk/operation/Operation.h"

#include <QObject>
#include <QString>

#include <future>

namespace smtk
{
namespace extension
{

class Launcher;

/// This class will be responsible for notifying the original caller of an
/// operation that the result from said operation is ready.
class SMTKQTEXT_EXPORT ResultHandler : public QObject
{
  Q_OBJECT

public:
  explicit ResultHandler();
  smtk::operation::Operation::Result waitForResult() const;
  std::shared_future<smtk::operation::Operation::Result>& future() { return m_future; }

Q_SIGNALS:
  /// Externally accessible signal on the primary thread containing the
  /// operation results.
  void resultReady(smtk::operation::Operation::Result result);

private:
  friend class qtOperationLauncher;
  friend class Launcher;
  std::shared_future<smtk::operation::Operation::Result> m_future;
};

/// An operation launcher that emits a signal containing the operation's result
/// after the operation has successfully executed.
///
/// qtOperationLauncher is a functor that executes an operation in a separate
/// thread and emits a signal with the operation's result on the thread
/// containing the launcher.
class SMTKQTEXT_EXPORT qtOperationLauncher : public QObject
{
  Q_OBJECT

public:
  static constexpr const char* const type_name = "smtk::extension::qtOperationLauncher";

  /**\brief The primary execution method of this functor. This function returns a new ResultHandler
    *       for each operation.
    */
  std::shared_ptr<ResultHandler> operator()(const smtk::operation::Operation::Ptr& operation);

  /// Return the number of operations launched whose results have not been processed yet.
  int activeOperations() const { return m_operationCount; }

  /**\brief Enable or disable switching to a busy-cursor when operations are running.
    *
    * The busy-cursor behavior is enabled (true) by default.
    *
    * The active-operation counter is always running, but whether it causes the
    * application cursor to change is controlled by this method.
    * The return value indicates whether the setting was changed.
    */
  static bool setBusyCursorBehavior(bool enabled);

Q_SIGNALS:
  /// Internal signal from the executing subthread to the primary thread
  /// indicating the completion of the operation.
  void operationHasResult(QString parametersName, QString resultName, QPrivateSignal);

private:
  /// Internal method run on a subthread to invoke the operation.
  smtk::operation::Operation::Result run(smtk::operation::Operation::Ptr operation);

  /// Internal method to update active operation count and busy-cursor (if enabled).
  void decrementCount(smtk::operation::Operation::Result result);

  smtk::common::ThreadPool<smtk::operation::Operation::Result> m_threadPool;

  /**\brief A count of the number of operations that have been launched but not produced results.
    *
    * This is used to change the default application cursor to a "spinner"
    * when the count is non-zero.
    */
  std::atomic<int> m_operationCount{ 0 };
  /// Whether the busy-cursor behavior is enabled (the default) or not.
  static bool s_busyCursorEnabled;
};

namespace qt
{
/// qtOperationLauncher has all of the Qt-integrated functionality we require;
/// smtk's operation launchers are stored in std::maps, however, requiring that
/// launchers support copy construction. We therefore wrap our Qt-enabled
/// launcher in a more stl-compliant wrapper.
class Launcher
{
public:
  Launcher()
    : m_launcher(new qtOperationLauncher)
  {
  }

  Launcher(const Launcher&)
    : m_launcher(new qtOperationLauncher)
  {
  }

  Launcher(Launcher&& other) noexcept
    : m_launcher{ other.m_launcher }
  {
    other.m_launcher = nullptr;
  }

  Launcher& operator=(Launcher&& other) noexcept
  {
    if (&other != this)
    {
      delete m_launcher;
      m_launcher = other.m_launcher;
      other.m_launcher = nullptr;
    }
    return *this;
  }

  ~Launcher() { delete m_launcher; }

  std::shared_future<smtk::operation::Operation::Result> operator()(
    const smtk::operation::Operation::Ptr& operation)
  {
    auto resHandler = (*m_launcher)(operation);
    return resHandler->future();
  }

  qtOperationLauncher* get() const { return m_launcher; }

private:
  qtOperationLauncher* m_launcher;
};
} // namespace qt
} // namespace extension
} // namespace smtk

#endif
