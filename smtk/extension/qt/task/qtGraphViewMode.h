//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtGraphViewMode_h
#define smtk_extension_qtGraphViewMode_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/PublicPointerDefs.h"

#include <QAction>
#include <QActionGroup>
#include <QObject>
#include <QToolBar>

class QAction;

namespace smtk
{
namespace extension
{

class qtTaskEditor;

/**\brief An object to handle modal interaction in a graph view.
  *
  * Subclasses of this object control how objects in the QSceneGraph respond
  * to user input by installing event filters and setting item flags.
  *
  * This base class provides the functionality to create a QAction for a
  * QActionGroup that is inserted into the view's QToolBar to switch modes.
  * When the action is invoked, a mode change is requested (and the mode
  * becomes active at some later point after any prior mode has an opportunity
  * to finalize itself).
  */
class SMTKQTEXT_EXPORT qtGraphViewMode : public QObject
{
  Q_OBJECT

public:
  using Superclass = QObject;

  /// Construct a mode for \a editor and add it to \a toolbar and \a modeGroup.
  qtGraphViewMode(
    smtk::string::Token modeName,
    qtTaskEditor* editor,
    QToolBar* toolbar,
    QActionGroup* modeGroup);
  ~qtGraphViewMode() override;

  QAction* modeAction() const { return m_modeAction; }

  bool isModeActive() const;

protected:
  friend class qtTaskEditor;

  /// This method is called by the editor as the scene is cleared.
  ///
  /// Subclasses which add items to the scene need to override this method
  /// to recreate their internal items.
  virtual void sceneCleared() {}

  /// This method is called by the editor as the mode is being changed *away from* this class.
  ///
  /// Subclasses should override this method to prepare the scene
  /// (such as enabling/disabling items in the scene, removing items drawn
  /// while the mode was active, etc.)
  virtual void exitMode() {}

  /// This method is called by the editor as the mode is being changed *to* this class.
  ///
  /// Subclasses should override this method to prepare the scene
  /// (such as enabling/disabling items in the scene, adding items to be
  /// drawn while the mode is active, etc.)
  virtual void enterMode() {}

  smtk::string::Token m_modeName;
  qtTaskEditor* m_editor{ nullptr };
  QAction* m_modeAction{ nullptr };
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtGraphViewMode_h
