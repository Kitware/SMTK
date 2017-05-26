//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __AbstractDataModel_h
#define __AbstractDataModel_h

#include <QAbstractItemModel>

class QTreeWidgetItem;

/**
 * @brief Abstract class implementing a tree model with basic functionality.
 *
 * It uses QTreeWidgetItem as its item class. Supports a bare minimum
 * implementation of the QAbstractItemModel API (Qt::DisplayRole, etc.). Any
 * more advanced features should be implemented in a subclass (row/column
 * insertion, make items checkable with Qt::CheckedRole/Qt::ItemIsUserCheckable,
 * etc.).
 *
 * To use, derive and implement initializeRootItem and populate its nodes.
 */
class AbstractDataModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  /**
   * A default model index might be useful to initialize selection in a view.
   */
  const QModelIndex getDefaultIndex();

protected:
  AbstractDataModel(QObject* parent_ = NULL);
  virtual ~AbstractDataModel();

  /**
  * @{
  * QAbstractItemModel implementation
  */
  int rowCount(const QModelIndex& parent_ = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent_ = QModelIndex()) const override;

  QModelIndex index(int row, int column, const QModelIndex& parent_ = QModelIndex()) const override;

  QModelIndex parent(const QModelIndex& index_) const override;
  QVariant data(const QModelIndex& index_, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index_, const QVariant& value, int role) override;

  QVariant headerData(
    int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  Qt::ItemFlags flags(const QModelIndex& index_) const override;
  /**
  * @}
  */

  /**
   * Convenience function to query the internal item of an index. Checks the
   * validity of index and returns RootItem if invalid (QAbstractModelItem
   * expects invalid QModelIndex() instances to refer to the RootItem).
   */
  QTreeWidgetItem* getItem(const QModelIndex& index) const;

  /**
  * Construct the root element. This is the element holding the header tags
  * so these should be initialized here. Concrete classes should implement this
  * method as it is up to them to decide the concrete time of element
  * (QTreeWidgetItem subclasses) to use.
  */
  virtual void initializeRootItem() = 0;

  /**
  * Helper for a more comprehensive validation of indices.
  */
  bool isIndexValid(const QModelIndex& index_) const;

  QTreeWidgetItem* RootItem = nullptr;

private:
  void operator=(const AbstractDataModel&) = delete;
  AbstractDataModel(const AbstractDataModel&) = delete;
};

#endif //__AbstractDataModel_h
