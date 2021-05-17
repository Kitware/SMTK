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
 * \brief Qt data model for smtk::attribute::DefinitionPtr instances
 *
 * The class was named after the XML element it represents in an attribute
 * template file (*.sbt) as a Qt data model. This model serves as an interface
 * to the attribute resource for insertion and removal of Attribute Definitions
 * (through the attribute resource instance).
 *
 */
class AttDefDataModel : public AbstractDataModel
{
  Q_OBJECT

public:
  using DefinitionPtrVec = std::vector<smtk::attribute::DefinitionPtr>;
  using AttDefElement = DataModelElement<smtk::attribute::DefinitionPtr>;

  AttDefDataModel(QObject* parent = nullptr);
  ~AttDefDataModel() override;

  /**
   * Populates the attribute definition tree.
   */
  void populate(smtk::attribute::ResourcePtr resource);

  /**
   * Query the internal data (DefinitionPtr in this case) of a given index.
   */
  const smtk::attribute::DefinitionPtr& get(const QModelIndex& index) const;

  /**
   * Insert an attribute Definition into the resource, it inserts as well a data
   * element into the tree defined by this data model.  An empty base type will
   * insert the Definition into the root node (this is how beginInsertRows()->
   * QAbstractItemMdoel::parent() handles a default/invalid QModelIndex()).
   */
  void insert(const AttDefContainer& props);

  /**
   * Remove an attribute Definition from the resource (and its corresponding data
   * element in the tree).
   */
  void remove(const QModelIndex& attDefIndex);

  //@{
  /**
   * Convenience functions to access properties of the Definition held by
   * index.
   */
  const std::string getType(const QModelIndex& index) const;
  bool hasDerivedTypes(const QModelIndex& index) const;
  //@}

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
    QTreeWidgetItem* element,
    const smtk::attribute::DefinitionPtr& dataMatch);

private:
  AttDefDataModel(const AttDefDataModel&) = delete;
  void operator=(const AttDefDataModel&) = delete;

  smtk::attribute::ResourcePtr Resource;
};
#endif //__AttDefDataModel_h
