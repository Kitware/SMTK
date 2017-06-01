//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __AttDefDataModel_h
#define __AttDefDataModel_h

#include <memory>

#include <smtk/PublicPointerDefs.h>

#include "AbstractDataModel.h"
#include "DataModelContainers.h"
#include "DataModelElement.h"

/**
 * \brief Qt data model used to display smtk::attribute::Definitions in a view.
 *
 * The class was named after the XML element it represents in an attribute
 * template file (*.sbt) as a Qt data model.
 *
 */
class AttDefDataModel : public AbstractDataModel
{
  Q_OBJECT

public:
  using DefinitionPtrVec = std::vector<smtk::attribute::DefinitionPtr>;
  using AttDefElement = DataModelElement<smtk::attribute::DefinitionPtr>;

  AttDefDataModel(QObject* parent = nullptr);
  ~AttDefDataModel();

  void populate(smtk::attribute::SystemPtr system);

  const smtk::attribute::DefinitionPtr& get(const QModelIndex& index) const;

  /**
   * An empty base type will insert the Definition into the root node (this is
   * how beginInsertRows()->QAbstractItemMdoel::parent() handles a default/invalid
   * QModelIndex()).
   */
  void insert(const AttDefContainer& props);

  void remove(const QModelIndex& attDefIndex);

  const std::string getType(const QModelIndex& index) const;

  bool hasDerivedTypes(const QModelIndex& index) const;

protected:
  void initializeRootItem() override;

  /**
   * Append all of the derived AttDef instances (children of parentDef).
   */
  void appendRecursively(smtk::attribute::DefinitionPtr parentDef, QTreeWidgetItem* parentItem);

  /**
   * Traverse the tree and find the item under the parameter element (parent)
   * referencing the dataMatch. This method is used when inserting a
   * new smtk::attribute::Definition (to update the model).
   */
  QModelIndex findElementByData(
    QTreeWidgetItem* element, const smtk::attribute::DefinitionPtr& dataMatch);

private:
  AttDefDataModel(const AttDefDataModel&) = delete;
  void operator=(const AttDefDataModel&) = delete;

  smtk::attribute::SystemPtr System;
};
#endif //__AttDefDataModel_h
