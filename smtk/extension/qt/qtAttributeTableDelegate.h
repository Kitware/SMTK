//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtAttributeTableDelegate_h
#define smtk_extension_qtAttributeTableDelegate_h

#include <QStyledItemDelegate>

#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/IntItemDefinition.h"

class QComboBox;
class QLineEdit;

namespace smtk
{
namespace extension
{

class qtAttributeTableModel;

/**
 * @brief Provides editors for cells in qtAttributeTableModel.
 *
 * Integer and double value items are edited using QLineEdit widgets with
 * validators configured from the corresponding SMTK item definitions.
 *
 * Other item types use QStyledItemDelegate's default editor behavior.
 */
class qtAttributeTableDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  qtAttributeTableDelegate(qtAttributeTableModel* model, QObject* parent = nullptr);

  ~qtAttributeTableDelegate() override = default;

  QWidget* createEditor(
    QWidget* parent,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const override;

  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index)
    const override;

protected:
  /**
   * @brief Convert an index from the table's proxy model to the source model.
   *
   * The QTableView displays a QSortFilterProxyModel, but item definitions are
   * stored by qtAttributeTableModel. Therefore, the index must be mapped before
   * querying the source model.
   */
  QModelIndex sourceIndex(const QModelIndex& index) const;

  /**
   * @brief Configure an integer editor from an IntItemDefinition.
   */
  void configureIntValidator(
    QLineEdit* editor,
    const smtk::attribute::IntItemDefinitionPtr& definition) const;

  /**
   * @brief Configure a floating-point editor from a DoubleItemDefinition.
   */
  void configureDoubleValidator(
    QLineEdit* editor,
    const smtk::attribute::DoubleItemDefinitionPtr& definition) const;

private:
  qtAttributeTableModel* m_model{ nullptr };
};

} // namespace extension
} // namespace smtk

#endif
