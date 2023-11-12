//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKWorkletToolboxPanel_h
#define smtk_extension_paraview_appcomponents_pqSMTKWorkletToolboxPanel_h
#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtWorkletPalette.h"

#include "smtk/operation/Operation.h" // for Index

#include "smtk/resource/Observer.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

#include <QPointer>
#include <QWidget>

class pqModalShortcut;
class pqPipelineSource;
class pqServer;

class pqSMTKWrapper;

class QListWidgetItem;

namespace smtk
{
namespace view
{
class OperationDecorator;
}
} // namespace smtk

/**\brief A panel that displays available operations in a "toolbox".
  *
  * The panel emits signals when users request an operation be
  * (a) immediately run or (b) run after editing parameters.
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKWorkletToolboxPanel : public QWidget
{
  Q_OBJECT
  typedef QWidget Superclass;

public:
  pqSMTKWorkletToolboxPanel(QWidget* parent = nullptr);
  ~pqSMTKWorkletToolboxPanel() override;

  /// Provide a default set of operations that should be presented in the toolbox.
  static void setDefaultOperations(
    const std::shared_ptr<smtk::view::OperationDecorator>& decorator);

  /// Return the toolbox view this dock-widget displays:
  QPointer<smtk::extension::qtWorkletPalette> toolbox() const { return m_view; }

Q_SIGNALS:
  void titleChanged(QString title);

public Q_SLOTS:
  /// Called when a new client-server connection is added.
  virtual void observeWrapper(pqSMTKWrapper*, pqServer*);

  /// Called when a client-server connection is removed.
  virtual void unobserveWrapper(pqSMTKWrapper*, pqServer*);

  /**\brief Called when the user presses Ctrl+Space.
    *
    * This method will raise the panel and apply focus to
    * the search bar.
    */
  virtual void searchFocus();

  /**\brief Available for task-based workflows to reconfigure available operations.
    *
    * A default configuration is provided the enables the search bar in
    * the qtWorkletPalette and the auto-run option in the qtWorkletModel.
    * The default allows all operations to be presented.
    */
  virtual bool setConfiguration(const smtk::view::Information& config);

Q_SIGNALS:
  /**\brief Populate the operation editor panel with the given operation \a index.
    *
    * If the operation editor is hidden and this method returns true,
    * it will be shown.
    */
  bool editOperation(smtk::operation::Operation::Index index);

  /// Queue the (potentially asynchronous) operation to be run immediately with default parameters.
  void runOperation(smtk::operation::Operation::Index index);

protected:
  /// Called by both setConfiguration and observeWrapper.
  void reconfigure();

  pqSMTKWrapper* m_wrapper{ nullptr }; // NB: This ties us to a single pqServer (the active one).
  QPointer<smtk::extension::qtWorkletPalette> m_view;
  QPointer<smtk::extension::qtUIManager> m_uiMgr;
#ifndef PARAVIEW_VERSION_59
  QPointer<pqModalShortcut> m_findOperationShortcut;
#endif
  std::shared_ptr<smtk::view::Configuration> m_configuration;
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKWorkletToolboxPanel_h
