//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extension_qtOperationLauncher_h
#define __smtk_extension_qtOperationLauncher_h

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

  /// The primary execution method of this functor.
  std::future<smtk::operation::Operation::Result> operator()(
    const smtk::operation::Operation::Ptr& operation);

signals:
  /// Internal signal from the executing subthread to the primary thread
  /// indicating the completion of the operation.
  void operationHasResult(QString resultName, QPrivateSignal);

  /// Externally accessible signal on the primary thread containing the
  /// operation results.
  void resultReady(smtk::operation::Operation::Result result);

private:
  /// Internal method run on a subthread to invoke the operation.
  smtk::operation::Operation::Result run(smtk::operation::Operation::Ptr operation);

  smtk::common::ThreadPool<smtk::operation::Operation::Result> m_threadPool;
};

namespace qt
{
/// qtOperationLauncher has all of the Qt-integrated functionality we require;
/// smtk's operation launchers are stored in std::maps, however, requiring that
/// launchers support copy construction. We therefore wrap our Qt-enabled
/// launcher in a more stl-complient wrapper.
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

  Launcher(Launcher&& other)
    : m_launcher{ other.m_launcher }
  {
    other.m_launcher = nullptr;
  }

  Launcher& operator=(Launcher&& other)
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

  std::future<smtk::operation::Operation::Result> operator()(
    const smtk::operation::Operation::Ptr& operation)
  {
    return (*m_launcher)(operation);
  }

  qtOperationLauncher* get() const { return m_launcher; }

private:
  qtOperationLauncher* m_launcher;
};
}
}
}

#endif
