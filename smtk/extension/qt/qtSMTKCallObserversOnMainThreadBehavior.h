//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtSMTKCallObserversOnMainThreadBehavior_h
#define smtk_extension_qt_qtSMTKCallObserversOnMainThreadBehavior_h

#include "smtk/extension/qt/Exports.h"

#include "smtk/common/Managers.h"
#include "smtk/common/UUID.h"
#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"
#include "smtk/view/Selection.h"

#include <QObject>

#include <mutex>

/** \brief Add logic to managers to ensure that operation and resource
  *        observers are called on the main thread.
  *
  * Qt requires that all methods that affect the GUI be performed on the
  * application's main thread. Many of the registered Observer functions for
  * both operations and resources are designed to affect the GUI. Rather than
  * connect GUI-modifying logic to a signal triggered by an observer, we mutate
  * the behavior of the operation and resource Observer calling logic to ensure
  * that all Observer functors are called on the main thread, regardless of
  * which thread performs the observation.
  *
  * The logic that allows us to ensure that observations are called on the main
  * thread uses Qt's signals and slots to relay the observation request between
  * threads. Because SMTK's Operation and Resource classes are not wrapped for
  * Qt's signal/slot relay, we instead hold a map of calling Operations and
  * Resources and pass an associated unique id between threads.
  *
  * \sa pqSMTKCallObserversOnMainThreadBehavior
  */
class SMTKQTEXT_EXPORT qtSMTKCallObserversOnMainThreadBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  static qtSMTKCallObserversOnMainThreadBehavior* instance(QObject* parent = nullptr);
  ~qtSMTKCallObserversOnMainThreadBehavior() override;

  /**\brief Mutate the logic of the manager's operation and resource Observers to
   *        be called on the main thread, rather than the thread from which the
   *        observation was requested.
   */
  void forceObserversToBeCalledOnMainThread(const smtk::common::Managers::Ptr& mgrs);

Q_SIGNALS:
  /**\brief Signal that a resource \a rsrc has been added or removed from the
   *        manager.
   */
  void resourceEvent(QString resourceId, int event, QPrivateSignal);

  /**\brief Signal that an operator \a op has been created, is about to run,
    *       or has run with the included \a result.
    */
  void operationEvent(QString opId, int event, QString resultName, QPrivateSignal);

  /**\brief Signal that a selection has been made.
   */
  void selectionEvent(QString selectionId, QString str, QPrivateSignal);

protected:
  qtSMTKCallObserversOnMainThreadBehavior(QObject* parent = nullptr);

private:
  std::map<smtk::common::UUID, std::shared_ptr<smtk::resource::Resource>> m_activeResources;
  std::map<smtk::common::UUID, std::shared_ptr<smtk::operation::Operation>> m_activeOperations;
  std::map<smtk::common::UUID, std::shared_ptr<smtk::view::Selection>> m_activeSelection;

  // A mutex to make access to m_activeOperations thread-safe
  std::mutex m_activeOperationMutex;

  Q_DISABLE_COPY(qtSMTKCallObserversOnMainThreadBehavior);
};

#endif
