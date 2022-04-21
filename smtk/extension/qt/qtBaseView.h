//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtBaseView_h
#define smtk_extension_qtBaseView_h

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtWidgets/QLayout>

#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/common/Deprecation.h"
#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtViewInfoDialog.h"
#include "smtk/view/BaseView.h"
#include "smtk/view/Information.h"

class QScrollArea;

// VTK's wrapper parser does not properly handle Qt macros on macos.
#if defined(__VTK_WRAP__) && !defined(Q_SLOTS)
#define Q_DISABLE_COPY(x)
#define Q_SLOTS
#define Q_SIGNALS protected
#define Q_OBJECT
#endif

namespace smtk
{
namespace extension
{
class qtUIManager;
class qtItem;
class qtViewInfoDialog;

///\brief A base class for all view types implemented using Qt
class SMTKQTEXT_EXPORT qtBaseView
  : public QObject
  , public smtk::view::BaseView
{
  Q_OBJECT

public:
  // Views do not follow RAII. They require a call to the protected virtual
  // method buildUI() to be initialized. Since objects that inherit from
  // qtBaseView are only ever called by qtUIManager, we can code around this
  // issue by making qtBaseView a friend of qtUIManager and by calling buildUI()
  // immediately upon construction in qtUIManager.
  friend class qtUIManager;

  smtkTypenameMacro(qtBaseView);

  qtBaseView(const smtk::view::Information& info);

  ~qtBaseView() override;

  const smtk::view::ConfigurationPtr& configuration() const
  {
    return m_viewInfo.get<smtk::view::ConfigurationPtr>();
  }

  QWidget* widget() const { return this->Widget; }

  QWidget* parentWidget() const { return m_viewInfo.get<QWidget*>(); }

  qtUIManager* uiManager() const { return m_viewInfo.get<qtUIManager*>(); }

  virtual int advanceLevel() const { return 0; }
  virtual bool categoryEnabled() const { return false; }
  virtual std::string currentCategory() const { return ""; }
  /// When category filtering is requested to be either on by default or is requested to be
  /// on permanently, we need to have a mechanism to force the views to display info based on
  /// the initial category.  This method will check to see if this is the case and call the
  /// onShowCategory method
  virtual void setInitialCategory() { ; }

  bool isTopLevel() const { return m_isTopLevel; }

  /// Return true if the view does not contain any information to display.
  /// Subclasses should override this method and return false by default
  /// unless they have specific knowledge that the view is vacuous.
  virtual bool isEmpty() const { return true; }

  ///\brief Return true if the view's contents are valid.
  virtual bool isValid() const { return true; }

  // Validates the view information to see if it is suitable for creating a qtBaseView instance
  static bool validateInformation(const smtk::view::Information& info);

Q_SIGNALS:
  void aboutToDestroy();
  void modified();

public Q_SLOTS:
  ///\brief Have the view update its contents
  ///
  /// The public method slot is used to update the view's GUI based on its  contents
  /// and the state of it's UIManager (category and advance level state, etc..)
  virtual void updateUI() { ; }
  virtual void childrenResized() { ; }
  virtual void showAdvanceLevelOverlay(bool val) { m_advOverlayVisible = val; }
  ///\brief Have the view update its contents based on a new advance level
  virtual void showAdvanceLevel(int /* level */) { ; }
  virtual void enableShowBy(int /* enable */) { ; }
  /// Display view information (i.e., help text)
  virtual void onInfo();

  virtual void onShowCategory() {}

protected:
  ///\brief Creates the UI related to the view and properly assigns it
  /// to the parent widget.
  virtual void buildUI() { ; }
  ///\brief Creates the main QT Widget that is associated with a View.  Typically this
  /// is the only method a derived View needs to override.
  virtual void createWidget() { ; }

  ///\brief Adds properties associated with respects to a top level view
  virtual void makeTopLevel();

  ///\brief Set the information to be displayed in the ViewInfoDialog
  virtual void setInfoToBeDisplayed();

  QWidget* Widget;
  bool m_isTopLevel;
  bool m_useSelectionManager;
  smtk::view::Information m_viewInfo;
  QPointer<qtViewInfoDialog> m_infoDialog;
  bool m_advOverlayVisible;
}; // class

} // namespace extension
} // namespace smtk

#endif
