//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extension_qtEntityItemModel_h
#define __smtk_extension_qtEntityItemModel_h

#include "QAbstractItemModel"
#include "QIcon"

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h" // For EXPORT macro.
#include "smtk/view/DescriptivePhrase.h"
#include <map>

namespace smtk
{
namespace extension
{

/**\brief Adapt an smtk::model::Manager instance into a hierarchical Qt model.
  *
  * This is done by generating instances of smtk::view::DescriptivePhrase
  * subclasses to portray the entities in the model manager both in terms of
  * their inherent attributes and their relations to other entities.
  *
  * By calling setPhrases() on the model, you identify the toplevel
  * description you wish to present; it may cover any subset of
  * an underlying model manager and may even describe entities from different
  * model managers.
  *
  * You may also call setPhraseFilter() on the model with a filter.
  * The filter is used to alter the available subphrases of each
  * descriptive phrase for presentation. For instance, you may write a
  * filter that omits descriptions of attributes on model items.
  */
class SMTKQTEXT_EXPORT QComponentItemModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  QComponentItemModel(QObject* parent = 0);
  virtual ~QComponentItemModel();
  // Set and get default entity color
  static void setDefaultEntityColor(const std::string& entityType, const QColor& color)
  {
    s_defaultColors[entityType] = color;
  }

  static QColor defaultEntityColor(const std::string& entityType);

  void clear();

  /// Enumeration of model-specific data roles.
  enum DataRoles
  {
    TitleTextRole = Qt::UserRole + 100,        //!< Entity name (user-editable)
    SubtitleTextRole = Qt::UserRole + 101,     //!< Entity type description
    EntityIconRole = Qt::UserRole + 102,       //!< Entity type icon
    EntityColorRole = Qt::UserRole + 103,      //!< Per-entity color
    EntityVisibilityRole = Qt::UserRole + 104, //!< Entity visibility
    EntityCleanRole = Qt::UserRole + 105,      //!< Is entity clean (0), dirty (1), or N/A (-1)?
    ModelActiveRole = Qt::UserRole + 106       //!< Is entity the active model?
  };

  QModelIndex index(int row, int column, const QModelIndex& parent) const override;
  QModelIndex parent(const QModelIndex& child) const override;
  bool hasChildren(const QModelIndex& parent) const override;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& inParent = QModelIndex()) const override
  {
    (void)inParent;
    return 1;
  }

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  //bool insertRows(int position, int rows, const QModelIndex& parent = QModelIndex()) override;
  bool removeRows(int position, int rows, const QModelIndex& parent = QModelIndex()) override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  //void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  void setRoot(view::DescriptivePhrasePtr root) { this->m_root = root; }

  smtk::model::ManagerPtr manager() const;

  void setDeleteOnRemoval(bool del) { this->m_deleteOnRemoval = del; }

  static QIcon lookupIconForEntityFlags(view::DescriptivePhrasePtr item, QColor color);

  view::DescriptivePhrasePtr getItem(const QModelIndex& idx) const;

  template <typename T, typename C>
  bool foreach_phrase(
    T& visitor, C& collector, const QModelIndex& top = QModelIndex(), bool onlyBuilt = true);
  template <typename T, typename C>
  bool foreach_phrase(
    T& visitor, C& collector, const QModelIndex& top = QModelIndex(), bool onlyBuilt = true) const;

  void rebuildSubphrases(const QModelIndex& qidx);

  Qt::DropActions supportedDropActions() const override;

public slots:
  /**\brief Update the model given an operator's \a result.
    *
    * It will always rebuild the root model index's children
    * if the root node has a subphrase generator attached;
    * only the differences will be emitted to the Qt model.
    * If no subphrase generator is attached, then any
    * expunged entities will have their corresponding phrases
    * removed (whether they are direct children of the root
    * node or not).
    *
    * All QModelIndex values currently in existence will be
    * visited to validate that their extant children match
    * and that they themselves have not been modified.
    */
  virtual void updateWithOperatorResult(const model::OperatorResult& result);

signals:
  void phraseTitleChanged(const QModelIndex&);
  void newIndexAdded(const QModelIndex& newidx);

protected:
  smtk::view::DescriptivePhrasePtr m_root;
  bool m_deleteOnRemoval; // remove UUIDs from mesh when they are removed from the list?
  static std::map<std::string, QColor> s_defaultColors;
  class Internal;
  Internal* P;
};

/**\brief Iterate over all expanded entries in the tree.
  *
  */
template <typename T, typename C>
bool QComponentItemModel::foreach_phrase(
  T& visitor, C& collector, const QModelIndex& top, bool onlyBuilt)
{
  // visit parent, then children if we aren't told to terminate:
  if (!visitor(this, top, collector))
  {
    view::DescriptivePhrasePtr phrase = this->getItem(top);
    // Do not descend if top's corresponding phrase would have to invoke
    // the subphrase generator to obtain the list of children... some models
    // are cyclic graphs. In these cases, only descend if "onlyBuilt" is false.
    if (phrase && (!onlyBuilt || phrase->areSubphrasesBuilt()))
    {
      for (int row = 0; row < this->rowCount(top); ++row)
      {
        if (this->foreach_phrase(visitor, collector, this->index(row, 0, top), onlyBuilt))
          return true; // early termination;
      }
    }
  }
  return false;
}

/// A const version of foreach_phrase. See the non-const version for documentation.
template <typename T, typename C>
bool QComponentItemModel::foreach_phrase(
  T& visitor, C& collector, const QModelIndex& top, bool onlyBuilt) const
{
  // visit parent, then children if we aren't told to terminate:
  if (!visitor(this, top, collector))
  {
    view::DescriptivePhrasePtr phrase = this->getItem(top);
    // Do not descend if top's corresponding phrase would have to invoke
    // the subphrase generator to obtain the list of children... some models
    // are cyclic graphs. In these cases, only descend if "onlyBuilt" is false.
    if (phrase && (!onlyBuilt || phrase->areSubphrasesBuilt()))
    {
      for (int row = 0; row < this->rowCount(top); ++row)
      {
        if (this->foreach_phrase(visitor, collector, this->index(row, 0, top), onlyBuilt))
          return true; // early termination;
      }
    }
  }
  return false;
}

} // namespace model
} // namespace smtk

#endif // __smtk_extension_qtEntityItemModel_h
