//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __AttDefInformation_h
#define __AttDefInformation_h
#include <memory>

#include <QWidget>

namespace Ui
{
class AttDefInformation;
}

class QModelIndex;
class ItemDefDataModel;

/**
 * \brief Widget displaying attribute definition's properties
 *
 * Displays <AttDef> elements defined in a (*.sbt) file and the ItemDefinitions
 * it contains.
 *
 */
class AttDefInformation : public QWidget
{
  Q_OBJECT

public:
  AttDefInformation(QWidget* parent = nullptr);
  ~AttDefInformation() override;

  AttDefInformation(const AttDefInformation&) = delete;
  AttDefInformation& operator=(const AttDefInformation&) = delete;

public Q_SLOTS:
  /**
   * Handles signals from the view (QSelectionModel) displaying the attribute
   * definitions.
   */
  void onAttDefChanged(const QModelIndex& currentDef, const QModelIndex& previousDef);

  /**
   * Updates the current user input in the smtk::attribute::Definition.
   */
  void onSaveAttDef();

Q_SIGNALS:
  /**
   * Indicates to the parent widget that a change has been made in the attribute
   * collection (or any of its entities) so that the user is informed about it.
   */
  void collectionChanged(bool needsSaving);

private Q_SLOTS:
  void showInheritedItemDetails(const QModelIndex& index);
  void showOwnedItemDetails(const QModelIndex& index);

  void onAddItemDef();
  void onRemoveItemDef();

private:
  void updateAttDefData(const QModelIndex& currentDef);

  void updateInheritedItemDef();

  void updateOwnedItemDef();

  std::unique_ptr<Ui::AttDefInformation> Ui;
  ItemDefDataModel* InheritedItemDefModel = nullptr;
  ItemDefDataModel* OwnedItemDefModel = nullptr;

  smtk::attribute::DefinitionPtr CurrentAttDef;
};
#endif //__AttDefInformation_h
