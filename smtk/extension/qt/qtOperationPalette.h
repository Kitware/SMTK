//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtOperationPalette_h
#define smtk_extension_qtOperationPalette_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/qtBaseView.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtWidgets/QLayout>

class QLineEdit;
class QSortFilterProxyModel;
class QVBoxLayout;
class QWidget;
class QScrollArea;
class qtOperationTypeModel;
class qtOperationTypeView;

namespace smtk
{
namespace view
{
class OperationDecorator;
}
namespace extension
{

/**\brief An SMTK view that lists types of operations.
  *
  * This view configures a qtOperationTypeModel as the source of
  * operation types to be presented.
  * It also passes the model through two QSortFilterProxyModel
  * instances: one which subsets operations using an OperationDecorator
  * provided by the application and one which filters operations
  * to match a search expression provided by the user in the optional
  * search bar (configured with the "SearchBar" attribute).
  * Both of these proxy models can be configured to perform a stable-sort
  * of their output operations. Use the "SubsetSort" and "FilterSort"
  * attributes to specify columns of the qtOperationTypeModel on which
  * to sort.
  * The default SubsetSort is "TypeName" while the default FilterSort
  * is "Associability," so that operations appear first by their relevance
  * to the current selection and then, within groups of the same relevance,
  * sorted by the label text.
  *
  * If the "AlwaysLimit" configuration option is false (or absent), then
  * a checkbox labeled "All" will be shown. The checkbox allows users to
  * display all of the operations without any decorations.
  *
  * Each operation has a corresponding qtOperationAction that
  * creates QPushButton instances for display in a dynamic grid.
  * The push buttons cause the action to emit either an "acceptDefaults"
  * or "editParameters" signal, depending on the whether the operation
  * can be configured to run with defaults and whether the user has
  * clicked or double-clicked on the button.
  * A search bar may also be configured.
  *
  * The view configuration of the operation palette may include a
  * model configuration to direct how the qtOperationTypeModel is
  * configured and may also include an "OperationDecorator"
  * configuration that provides overrides for the presentation
  * style of operations.
  * Note that if an `smtk::common::Managers::Ptr` is included in
  * the constructor's `smtk::view::Information` and it contains
  * a `QSharedPointer<qtOperationTypeModel>`, that object will be
  * used instead of constructing a model solely for the view.
  * This allows applications to provide a single model for many
  * purposes (toolbox, toolbars, menu actions, etc.).
  *
  * <View Type="qtOperationPalette"
  *   SearchBar="true"
  *   FilterSort="Precendence"
  *   AlwaysLimit="false"
  *   SubsetSort="Associability" SubsetAssociability="[4-9]"
  *   >
  *   <Model ... see qtOperationModel for configuration details ... />
  * </View>
  */
class SMTKQTEXT_EXPORT qtOperationPalette : public qtBaseView
{
  Q_OBJECT

public:
  // Views do not follow RAII. They require a call to the protected virtual
  // method buildUI() to be initialized. Since objects that inherit from
  // qtOperationPalette are only ever called by qtUIManager, we can code around this
  // issue by making qtOperationPalette a friend of qtUIManager and by calling buildUI()
  // immediately upon construction in qtUIManager.
  friend class qtUIManager;

  smtkTypenameMacro(qtOperationPalette);
  smtkSuperclassMacro(qtBaseView);

  qtOperationPalette(const smtk::view::Information& info);
  ~qtOperationPalette() override;

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

  // Validates the view information to see if it is suitable for creating a qtOperationPalette instance
  static bool validateInformation(const smtk::view::Information& info);

  /// Return the model displayed in the view.
  QSortFilterProxyModel* model() const;

  /**\brief Return the model holding all of the operations.
    *
    * It is this model which emits "runOperation" and "editOperationParameters" signals.
    */
  qtOperationTypeModel* operationModel() const;

  /// Return the view widget holding operation buttons.
  qtOperationTypeView* operationView() const;

  /// Return the search-bar widget (if configured) or null (otherwise).
  QLineEdit* searchTextWidget() const;

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

  virtual void editTopOperation();
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

  /// A QAbstractListModel that lists all registered operations.
  QPointer<qtOperationTypeModel> m_model;
  /// A proxy model that chooses a subset of operations relevant to users.
  QPointer<QSortFilterProxyModel> m_subset;
  /// A proxy model for users to search by operation name.
  QPointer<QSortFilterProxyModel> m_filter;

  /// The central widget's layout.
  QPointer<QVBoxLayout> m_layout;
  /// An optional search bar for users.
  QPointer<QLineEdit> m_search;
  /// The list-view of operations.
  QPointer<qtOperationTypeView> m_list;
  /// The default operation decorator owned by m_model.
  ///
  /// Toggling the "All" checkbox to unlimit operations unsets or restores
  /// the decorator on m_model.
  std::shared_ptr<smtk::view::OperationDecorator> m_decorator;
};

} // namespace extension
} // namespace smtk

#endif
