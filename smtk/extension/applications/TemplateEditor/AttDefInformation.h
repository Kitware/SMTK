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
class ItemDefinitionsDataModel;

/**
 * \brief Widget which displays an attribute definition.
 *
 * Displays <AttDef> elements defined in a (*.sbt) file.
 *
 */
class AttDefInformation : public QWidget
{
  Q_OBJECT

public:
  AttDefInformation(QWidget* parent = nullptr);
  ~AttDefInformation();

public slots:
  void onAttDefChanged(const QModelIndex& currentDef, const QModelIndex& previousDef);

  /**
   * Sets the current user input in the smtk::attribute::Definition.
   */
  void onSaveAttDef();

private slots:
  void showInheritedItemDetails(const QModelIndex& index);
  void showOwnedItemDetails(const QModelIndex& index);

  void onAddItemDef();
  void onRemoveItemDef();

private:
  AttDefInformation(const AttDefInformation&) = delete;
  void operator=(const AttDefInformation&) = delete;

  void updateAttDefData(const QModelIndex& currentDef);

  void updateInheritedItemDef();

  void updateOwnedItemDef();

  std::unique_ptr<Ui::AttDefInformation> Ui;
  ItemDefinitionsDataModel* InheritedItemDefModel = nullptr;
  ItemDefinitionsDataModel* OwnedItemDefModel = nullptr;

  smtk::attribute::DefinitionPtr CurrentAttDef;
};
#endif //__AttDefInformation_h
