//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtAttributeTableDelegate.h"

#include "smtk/extension/qt/qtAttributeTableModel.h"

#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/IntItemDefinition.h"

#include <QApplication>
#include <QComboBox>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStyle>
#include <QStyleOptionComboBox>

#include <algorithm>
#include <cmath>
#include <limits>

namespace smtk
{
namespace extension
{

// Add validators that will return empty values
class qtUnsettableDoubleValidator : public QDoubleValidator
{
public:
  using QDoubleValidator::QDoubleValidator;

  State validate(QString& input, int& position) const override
  {
    if (input.trimmed().isEmpty())
    {
      return Acceptable;
    }

    return QDoubleValidator::validate(input, position);
  }
};

class qtUnsettableIntValidator : public QIntValidator
{
public:
  using QIntValidator::QIntValidator;

  State validate(QString& input, int& position) const override
  {
    if (input.trimmed().isEmpty())
    {
      return Acceptable;
    }

    return QIntValidator::validate(input, position);
  }
};

qtAttributeTableDelegate::qtAttributeTableDelegate(qtAttributeTableModel* model, QObject* parent)
  : QStyledItemDelegate(parent)
  , m_model(model)
{
}

QModelIndex qtAttributeTableDelegate::sourceIndex(const QModelIndex& index) const
{
  if (!index.isValid())
  {
    return {};
  }

  /*
   * The delegate normally receives an index from the QSortFilterProxyModel
   * installed on the QTableView. Map that index back to the source model before
   * querying SMTK-specific information.
   */
  const auto* proxy = qobject_cast<const QSortFilterProxyModel*>(index.model());

  if (proxy && proxy->sourceModel() == m_model)
  {
    return proxy->mapToSource(index);
  }

  return index;
}

QWidget* qtAttributeTableDelegate::createEditor(
  QWidget* parent,
  const QStyleOptionViewItem& option,
  const QModelIndex& index) const
{
  if (!m_model)
  {
    return QStyledItemDelegate::createEditor(parent, option, index);
  }

  const QModelIndex srcIndex = this->sourceIndex(index);

  if (!srcIndex.isValid())
  {
    return QStyledItemDelegate::createEditor(parent, option, index);
  }

  /*
   * Discrete IntItem, DoubleItem, and StringItem values are all represented
   * using the same QComboBox editor. This test must occur before checking the
   * concrete item definition type, otherwise a discrete IntItem would receive
   * a QIntValidator editor.
   */
  if (m_model->isDiscrete(srcIndex))
  {
    auto* comboBox = new QComboBox(parent);

    QObject::connect(comboBox, qOverload<int>(&QComboBox::activated), this, [this, comboBox](int) {
      /*
         * Commit the new enum selection immediately. Using activated() instead
         * of currentIndexChanged() avoids committing while setEditorData() is
         * initially populating the combo box.
         */
      Q_EMIT const_cast<qtAttributeTableDelegate*>(this)->commitData(comboBox);
    });

    return comboBox;
  }
  const auto definition = m_model->itemDefinitionForIndex(srcIndex);

  if (!definition)
  {
    return QStyledItemDelegate::createEditor(parent, option, index);
  }

  /*
   * Integer value items use a line editor with an integer validator whose
   * limits are derived from the SMTK IntItemDefinition.
   */
  if (
    auto intDefinition = std::dynamic_pointer_cast<smtk::attribute::IntItemDefinition>(definition))
  {
    auto* editor = new QLineEdit(parent);

    this->configureIntValidator(editor, intDefinition);

    return editor;
  }

  /*
   * Double value items use a line editor with a floating-point validator whose
   * limits are derived from the SMTK DoubleItemDefinition.
   */
  if (
    auto doubleDefinition =
      std::dynamic_pointer_cast<smtk::attribute::DoubleItemDefinition>(definition))
  {
    auto* editor = new QLineEdit(parent);

    this->configureDoubleValidator(editor, doubleDefinition);

    return editor;
  }

  /*
   * String items and other editable item types use Qt's default editor.
   */
  return QStyledItemDelegate::createEditor(parent, option, index);
}

QSize qtAttributeTableDelegate::sizeHint(
  const QStyleOptionViewItem& option,
  const QModelIndex& index) const
{
  QSize result = QStyledItemDelegate::sizeHint(option, index);

  if (!m_model)
  {
    return result;
  }

  const QModelIndex srcIndex = this->sourceIndex(index);
  if (!srcIndex.isValid() || !m_model->isDiscrete(srcIndex))
  {
    return result;
  }

  /*
   * resizeColumnsToContents() normally considers only the value currently
   * displayed in each cell. Account for every value that the combo-box editor
   * can display, including its unset entry and the style's arrow and padding.
   */
  QString widestValue = tr("<unset>");
  int widestText = option.fontMetrics.horizontalAdvance(widestValue);

  for (const QString& value : m_model->discreteValues(srcIndex))
  {
    const int textWidth = option.fontMetrics.horizontalAdvance(value);
    if (textWidth > widestText)
    {
      widestValue = value;
      widestText = textWidth;
    }
  }

  QStyleOptionComboBox comboOption;
  comboOption.currentText = widestValue;
  comboOption.fontMetrics = option.fontMetrics;

  QStyle* style = option.widget ? option.widget->style() : QApplication::style();
  const QSize comboSize = style->sizeFromContents(
    QStyle::CT_ComboBox,
    &comboOption,
    QSize(widestText, option.fontMetrics.height()),
    option.widget);

  result.setWidth(std::max(result.width(), comboSize.width()));
  result.setHeight(std::max(result.height(), comboSize.height()));
  return result;
}

void qtAttributeTableDelegate::configureIntValidator(
  QLineEdit* editor,
  const smtk::attribute::IntItemDefinitionPtr& definition) const
{
  if (!editor || !definition)
  {
    return;
  }

  int minimum = std::numeric_limits<int>::min();
  int maximum = std::numeric_limits<int>::max();

  /*
   * QIntValidator uses inclusive limits. Convert SMTK exclusive limits to the
   * nearest legal integer boundary.
   */
  if (definition->hasMinRange())
  {
    minimum = definition->minRange();

    if (!definition->minRangeInclusive() && minimum < std::numeric_limits<int>::max())
    {
      ++minimum;
    }
  }

  if (definition->hasMaxRange())
  {
    maximum = definition->maxRange();

    if (!definition->maxRangeInclusive() && maximum > std::numeric_limits<int>::min())
    {
      --maximum;
    }
  }

  editor->setValidator(new qtUnsettableIntValidator(minimum, maximum, editor));
}

void qtAttributeTableDelegate::configureDoubleValidator(
  QLineEdit* editor,
  const smtk::attribute::DoubleItemDefinitionPtr& definition) const
{
  if (!editor || !definition)
  {
    return;
  }

  double minimum = -std::numeric_limits<double>::max();

  double maximum = std::numeric_limits<double>::max();

  if (definition->hasMinRange())
  {
    minimum = definition->minRange();

    /*
     * QDoubleValidator only supports inclusive bounds. For an exclusive SMTK
     * limit, use the next representable floating-point value.
     */
    if (!definition->minRangeInclusive())
    {
      minimum = std::nextafter(minimum, std::numeric_limits<double>::infinity());
    }
  }

  if (definition->hasMaxRange())
  {
    maximum = definition->maxRange();

    if (!definition->maxRangeInclusive())
    {
      maximum = std::nextafter(maximum, -std::numeric_limits<double>::infinity());
    }
  }

  auto* validator = new qtUnsettableDoubleValidator(minimum, maximum, 16, editor);

  /*
   * Permit both ordinary decimal notation and scientific notation.
   */
  validator->setNotation(QDoubleValidator::ScientificNotation);

  editor->setValidator(validator);
}

void qtAttributeTableDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  auto* comboBox = qobject_cast<QComboBox*>(editor);

  if (!comboBox)
  {
    QStyledItemDelegate::setEditorData(editor, index);

    return;
  }

  if (!m_model)
  {
    return;
  }

  const QModelIndex srcIndex = this->sourceIndex(index);

  if (!srcIndex.isValid())
  {
    return;
  }

  const QStringList values = m_model->discreteValues(srcIndex);

  comboBox->clear();

  /*
   * The invalid QVariant associated with this entry tells setModelData()
   * that the value should be unset.
   */
  comboBox->addItem(tr("<unset>"), QVariant());

  for (const QString& value : values)
  {
    comboBox->addItem(value, value);
  }

  if (!m_model->isValueSet(srcIndex))
  {
    comboBox->setCurrentIndex(0);
    return;
  }

  const QString currentValue = m_model->currentDiscreteValue(srcIndex);

  const int currentIndex = comboBox->findData(currentValue);

  comboBox->setCurrentIndex(currentIndex >= 0 ? currentIndex : 0);
}

void qtAttributeTableDelegate::setModelData(
  QWidget* editor,
  QAbstractItemModel* model,
  const QModelIndex& index) const
{
  /*
   * Handle discrete-value editors.
   */
  if (auto* comboBox = qobject_cast<QComboBox*>(editor))
  {
    const QVariant value = comboBox->currentData();

    if (!value.isValid())
    {
      /*
       * The <unset> entry has an invalid QVariant as its user data.
       */
      model->setData(index, QVariant(), Qt::EditRole);
    }
    else
    {
      model->setData(index, value, Qt::EditRole);
    }

    return;
  }

  /*
   * Handle numeric line editors.
   */
  if (auto* lineEdit = qobject_cast<QLineEdit*>(editor))
  {
    const QString text = lineEdit->text().trimmed();

    if (text.isEmpty())
    {
      /*
       * Clearing the editor explicitly unsets the SMTK ValueItem element.
       */
      model->setData(index, QVariant(), Qt::EditRole);

      return;
    }

    /*
     * Let the normal Qt delegate logic convert the editor text into the
     * QVariant expected by the model.
     */
    QStyledItemDelegate::setModelData(editor, model, index);

    return;
  }

  QStyledItemDelegate::setModelData(editor, model, index);
}

} // namespace extension
} // namespace smtk
