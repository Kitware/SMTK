//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __ItemDefDataModel_h
#define __ItemDefDataModel_h
#include <memory>

#include <QModelIndex>

#include "smtk/PublicPointerDefs.h"

#include "AbstractDataModel.h"
#include "DataModelElement.h"

/**
 * \brief Qt data model for smtk::attribute::ItemDefinitionPtr instances.
 *
 * The class was named after the XML element it represents in an attribute
 * template file (*.sbt) as a Qt data model. This model serves as an interface
 * to the attribute collection for insertion and removal of Item Definitions
 * (through a given Attribute Definition).
 *
 */
class ItemDefDataModel : public AbstractDataModel
{
  Q_OBJECT

public:
  using GroupDef = smtk::attribute::GroupItemDefinition;
  using ItemDefPtr = smtk::attribute::ItemDefinitionPtr;
  using ItemDefPtrVec = std::vector<ItemDefPtr>;
  using ItemDefElement = DataModelElement<smtk::attribute::ItemDefinitionPtr>;

  ItemDefDataModel(QObject* parent = nullptr);
  ~ItemDefDataModel() override;

  ItemDefDataModel(const ItemDefDataModel&) = delete;
  ItemDefDataModel& operator=(const ItemDefDataModel&) = delete;

  /**
   * Appends a branch of ItemDefinition instances contained in a Definition
   * to the data model's root node.  This method is used to populate the tree.
   */
  void appendBranchToRoot(smtk::attribute::DefinitionPtr def);

  /**
   * Query the internal data (ItemDefinitionPtr in this case) of a given index.
   */
  const smtk::attribute::ItemDefinitionPtr& get(const QModelIndex& index) const;

  /**
   * \brief Container for parameters to insert an item definition.
   * Holds the ItemDefinition itself, the Definition it belongs to and its
   * parent index in the tree.
   */
  struct Container
  {
    Container() = default;

    smtk::attribute::ItemDefinitionPtr ItemDefinition;
    smtk::attribute::DefinitionPtr Definition;
    QModelIndex ParentIndex;
  };

  /**
   * Insert an ItemDefinition into an AttDef. It inserts as well a data element
   * into the tree defined by this data model.
   */
  void insert(const Container& props);

  /**
   * Remove an ItemDef from an AttDef (and its corresponding data element in the
   * tree.
   */
  void remove(const QModelIndex& itemIndex, smtk::attribute::DefinitionPtr def);

protected:
  void initializeRootItem() override;

  /**
   * Append all ItemDefinition types in an AttDef recursively to populate the
   * tree.
   */
  void appendRecursively(
    smtk::attribute::ItemDefinitionPtr parentItemDef,
    QTreeWidgetItem* parentItem,
    const QString& attDefType);

  /**
   * Update the attribute collection. This ensures the attribute::collection instance is
   * purged from any Attributes affected by a change in this ItemDefinition (currently
   * wipes anything related to the Definition). qtUIManager->qtInstancedView generates
   * an attribute and items for each (or some) of the ui elements, and stores them in
   * attribute::collection, which could lead to Attributes with Item Definitions in an
   * invalid state.
   *
   * Call this function whenever an ItemDefinition has be modified in the attribute::
   * collection instance.
   *
   * \note TODO Reproduce the issue by not calling this during insertion and talk to
   * Bob, it could be a bug in the UIManager. Bob says PreviewPanel::createView should
   * be enough to flush the 'involved' attributes (no additional ones should be
   * created?). Try killing the view before inserting any new itemdefinition. Also
   * invalidate the view as soon as the ItemDef changes (otherwise manipulating it
   * could cause a crash).
   */
  void clearAttributes(smtk::attribute::DefinitionPtr def);
};
#endif //__ItemDefDataModel_h
