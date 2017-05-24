//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __ItemDefinitionsDataModel_h
#define __ItemDefinitionsDataModel_h

#include <memory>

#include <QModelIndex>

#include "smtk/PublicPointerDefs.h"

#include "AbstractDataModel.h"
#include "DataModelElement.h"

/**
 * \brief Qt data model used to display smtk::attribute::ItemDefinitions in a
 * view.
 *
 * The class was named after the XML element it represents in an attribute
 * template file (*.sbt) as a Qt data model.
 *
 */
class ItemDefinitionsDataModel : public AbstractDataModel
{
  Q_OBJECT

public:
  using GroupDef = smtk::attribute::GroupItemDefinition;
  using ItemDefPtr = smtk::attribute::ItemDefinitionPtr;
  using ItemDefPtrVec = std::vector<ItemDefPtr>;
  using ItemDefElement = DataModelElement<smtk::attribute::ItemDefinitionPtr>;

  /**
   * \brief Container for parameters to create an item definition.
   * TODO Move this into its own class. Different subclasses will define
   * properties for different concrete ItemDefinitions.
   */
  struct ItemDefProperties
  {
    ItemDefProperties(){};

    smtk::attribute::DefinitionPtr Definition;
    std::string Name;
    std::string Type;
    QModelIndex ParentNode;
  };

  ItemDefinitionsDataModel(QObject* parent = nullptr);
  ~ItemDefinitionsDataModel();

  void clear();

  void appendBranchToRoot(smtk::attribute::DefinitionPtr def);

  const smtk::attribute::ItemDefinitionPtr& getItemDef(const QModelIndex& index) const;

  void insertItem(ItemDefProperties const& props);

  void removeItem(const QModelIndex& itemIndex);

protected:
  void initializeRootItem();

  /**
   * Append all AttDef types recursively.
   */
  void appendRecursively(smtk::attribute::ItemDefinitionPtr parentItemDef,
    QTreeWidgetItem* parentItem, const QString& attDefType);

private:
  ItemDefinitionsDataModel(const ItemDefinitionsDataModel&) = delete;
  void operator=(const ItemDefinitionsDataModel&) = delete;
};
#endif //__ItemDefinitionsDataModel_h
