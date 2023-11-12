//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtWorkletPalette_h
#define smtk_extension_qtWorkletPalette_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/qtBaseView.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtWidgets/QLayout>

class QLineEdit;
class QTableView;
class QAbstractItemModel;
class QSortFilterProxyModel;
class QVBoxLayout;
class QWidget;
class QScrollArea;
class qtWorkletModel;

namespace smtk
{
namespace extension
{

/**\brief An SMTK view that lists worklets which can be added to a project's workflow.
  *
  * This view configures a qtWorkletModel as the source of worklets to be presented.
  * It also passes the model through a QSortFilterProxyModel which filters (and
  * sorts worklets to match a search expression provided by the user in the optional
  * search bar (configured with the "SearchBar" attribute).
  * This proxy model can be configured to perform a stable-sort of its output worklets.
  * Use the "FilterSort" attributes to specify columns of the qtWorkletModel on which to sort.
  * The default FilterSort is "Label," so that worklets appear sorted by their name.
  *
  * The view configuration of the worklet palette may include a
  * model configuration to direct how the qtWorkletModel is configured.
  *
  * <View Type="qtWorkletPalette"
  *   SearchBar="true"
  *   FilterSort="Label"
  *   >
  *   <Model ... see qtWorkletModel for configuration details ... />
  * </View>
  */
class SMTKQTEXT_EXPORT qtWorkletPalette : public qtBaseView
{
  Q_OBJECT

public:
  // Views do not follow RAII. They require a call to the protected virtual
  // method buildUI() to be initialized. Since objects that inherit from
  // qtWorkletPalette are only ever called by qtUIManager, we can code around this
  // issue by making qtWorkletPalette a friend of qtUIManager and by calling buildUI()
  // immediately upon construction in qtUIManager.
  friend class qtUIManager;

  smtkTypenameMacro(qtWorkletPalette);
  smtkSuperclassMacro(qtBaseView);

  qtWorkletPalette(const smtk::view::Information& info);
  ~qtWorkletPalette() override;

  // virtual int advanceLevel() const { return 0; }
  // virtual bool categoryEnabled() const { return false; }
  // virtual std::string currentCategory() const { return ""; }

  /// When category filtering is requested to be either on by default or is requested to be
  /// on permanently, we need to have a mechanism to force the views to display info based on
  /// the initial category.  This method will check to see if this is the case and call the
  /// onShowCategory method
  // virtual void setInitialCategory() {}

  /// Return true if the view does not contain any information to display.
  /// Subclasses should override this method and return false by default
  /// unless they have specific knowledge that the view is vacuous.
  bool isEmpty() const override;

  // Validates the view information to see if it is suitable for creating a qtWorkletPalette instance
  static bool validateInformation(const smtk::view::Information& info);

  /// Return the model displayed in the view.
  QSortFilterProxyModel* model() const;

  /**\brief Return the model holding all of the worklets.
    */
  qtWorkletModel* workletModel() const;

  /// Return the view widget holding worklet buttons.
  QTableView* workletView() const;

  /// Return the search-bar widget (if configured) or null (otherwise).
  QLineEdit* searchTextWidget() const;

Q_SIGNALS:
  void emplaceWorklet(const std::string& workletName, const std::array<double, 2>& location);

public Q_SLOTS:
  ///\brief Have the view update its contents
  ///
  /// The public method slot is used to update the view's GUI based on its  contents
  /// and the state of it's UIManager (category and advance level state, etc..)
  void updateUI() override {}
  void childrenResized() override {}
  ///\brief Have the view update its contents based on a new advance level
  void showAdvanceLevel(int /* level */) override {}
  void enableShowBy(int /* enable */) override {}
  /// Display view information (i.e., help text)
  void onInfo() override;

  void onShowCategory() override {}

  virtual void instantiateTopWorklet();
  virtual void toggleFiltering(int filterState);

protected:
  ///\brief Creates the UI related to the view and properly assigns it
  /// to the parent widget.
  void buildUI() override;
  ///\brief Creates the main QT Widget that is associated with a View.  Typically this
  /// is the only method a derived View needs to override.
  void createWidget() override;

  ///\brief Set the information to be displayed in the ViewInfoDialog
  void setInfoToBeDisplayed() override;

  /// A QAbstractListModel that lists all registered worklets.
  QPointer<qtWorkletModel> m_model;
  /// A proxy model for users to search by worklet name.
  QPointer<QSortFilterProxyModel> m_filter;

  /// The central widget's layout.
  QPointer<QVBoxLayout> m_layout;
  /// An optional search bar for users.
  QPointer<QLineEdit> m_search;
  /// The list-view of worklets.
  QPointer<QTableView> m_list;
};

} // namespace extension
} // namespace smtk

#endif
