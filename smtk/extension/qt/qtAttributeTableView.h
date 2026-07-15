//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtAttributeTableView_h
#define smtk_extension_qtAttributeTableView_h

#include "smtk/extension/qt/qtBaseAttributeView.h"

#include <QPointer>

class QPushButton;
class QTableView;
class QWidget;

namespace smtk
{
namespace attribute
{
class Attribute;
}

namespace view
{
class Information;
}

namespace extension
{

class qtAttributeTableModel;

/**
 * @brief Displays and manages attributes based on a common definition.
 *
 * Each table row represents one SMTK attribute. Columns are generated from
 * the item definitions associated with the configured attribute definition.
 *
 * The view also provides controls for:
 *
 * - creating a new attribute based on the configured definition;
 * - deleting one or more selected attributes.
 *
 * The SMTK attribute resource remains the authoritative source of data.
 */
class qtAttributeTableView : public qtBaseAttributeView
{
  Q_OBJECT

public:
  smtkTypenameMacro(qtAttributeTableView);

  static qtBaseView* createViewWidget(const smtk::view::Information& info);

  qtAttributeTableView(const smtk::view::Information& info);

  ~qtAttributeTableView() override;

  /// Return the source model used by the table.
  qtAttributeTableModel* tableModel() const;

  /// Return the table widget.
  QTableView* tableView() const;

public Q_SLOTS:

  /**
   * @brief Refresh the table from the underlying attribute resource.
   */
  void updateUI() override;

protected Q_SLOTS:

  /**
   * @brief Create an attribute based on the configured definition.
   *
   * The resource assigns a unique attribute name when no explicit name is
   * supplied. After creation, the model is refreshed and the new row is
   * selected.
   */
  void createAttribute();

  /**
   * @brief Delete the attributes represented by the selected table rows.
   *
   * Proxy-model row indices are mapped to source-model indices before the
   * associated attributes are retrieved.
   */
  void deleteSelectedAttributes();

  /**
   * @brief Update the enabled state of the delete button.
   *
   * Deletion is enabled only when at least one table row is selected.
   */
  void updateButtonState();

protected:
  /**
   * @brief Construct this view's Qt widgets.
   */
  void createWidget() override;

  /**
   * @brief Return the definition type specified by the view configuration.
   */
  std::string definitionType() const;

  /**
   * @brief Rebuild the list of attributes displayed by the model.
   */
  void rebuildAttributeList();

  /**
   * @brief Notify the SMTK UI that an attribute was modified.
   */
  void attributeModified(const smtk::attribute::AttributePtr& attribute);

  /**
   * @brief Update conditional-child column visibility for the current row.
   *
   * When the view's HideInactiveChildren option is enabled, columns for
   * conditional children that are inactive in the current row are hidden.
   * All columns are shown when there is no current row.
   */
  void updateColumnVisibility();

  /**
   * @brief Select an attribute in the table.
   *
   * This method is used after attribute creation so the newly created
   * attribute becomes the current table selection.
   */
  void selectAttribute(const smtk::attribute::AttributePtr& attribute);

private:
  class Internal;
  Internal* m_internals{ nullptr };
};

} // namespace extension
} // namespace smtk

#endif
