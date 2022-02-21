//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtOperationTypeView_h
#define smtk_extension_qt_qtOperationTypeView_h

#include "smtk/SharedFromThis.h"
#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtToolPaletteLayout.h"

#include <QAbstractItemView>
#include <QPointer>
#include <QScrollArea>

class QAbstractItemModel;

/**\brief A view that displays a list-model as toolbar buttons.
  *
  * The model must be a qtOperationTypeModel or a proxy-model with
  * an underlying qtOperationTypeModel because this view expects
  * the second column to contain a qtOperationAction object.
  */
class SMTKQTEXT_EXPORT qtOperationTypeView : public QScrollArea
{
  Q_OBJECT
  Q_PROPERTY(QAbstractItemModel* model READ model WRITE setModel);

public:
  qtOperationTypeView(QWidget* parent = nullptr);
  ~qtOperationTypeView() override;

  smtkTypeMacroBase(qtOperationTypeView);
  smtkSuperclassMacro(QScrollArea);

  ///@{
  /**\brief Methods that mimic QAbstractItemView.
    *
    * This class does not inherit QAbstractItemView because
    * that makes handling scrolling and resizing overly complex,
    * but it presents much of the same API.
    */
  QAbstractItemModel* model() const;
  void setModel(QAbstractItemModel* model);
  QModelIndex indexAt(const QPoint& point) const;
  void scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint);
  QRect visualRect(const QModelIndex& index) const;
  ///@}

protected Q_SLOTS:
  void operationsAdded(const QModelIndex& parent, int first, int last);
  void operationsReordered(
    const QModelIndex& sourceParent,
    int sourceStart,
    int sourceEnd,
    const QModelIndex& destinationParent,
    int destinationRow);
  void operationsRemoved(const QModelIndex& parent, int first, int last);
  void operationsUpdated(
    const QModelIndex& topLeft,
    const QModelIndex& bottomRight,
    const QVector<int>& roles = QVector<int>());
  void operationsLayoutChanging(
    const QList<QPersistentModelIndex>& parents,
    QAbstractItemModel::LayoutChangeHint hint);
  void operationsLayoutChanged(
    const QList<QPersistentModelIndex>& parents,
    QAbstractItemModel::LayoutChangeHint hint);
  void operationsResetting();

protected:
#if 0
  // TODO: Implement analogues to these QAbstractItemView methods.
  int horizontalOffset() const;
  int verticalOffset() const;
  bool isIndexHidden(const QModelIndex& index) const;
  QModelIndex moveCursor(
    QAbstractItemView::CursorAction cursorAction,
    Qt::KeyboardModifiers modifiers);
  void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags);
  QRegion visualRegionForSelection(const QItemSelection& selection) const;
  bool eventFilter(QObject* object, QEvent* event);
#endif

  QPointer<qtToolPaletteLayout> m_flow;
  QPointer<QWidget> m_palette;
  QPointer<QAbstractItemModel> m_model;

private:
  Q_DISABLE_COPY(qtOperationTypeView);
};

#endif // smtk_extension_qt_qtOperationTypeView_h
