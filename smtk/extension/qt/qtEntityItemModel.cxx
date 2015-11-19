//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtEntityItemModel.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/Manager.h"
#include "smtk/model/MeshPhrase.h"
#include "smtk/model/StringData.h"
#include "smtk/model/SubphraseGenerator.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/MeshItem.h"
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QVariant>

#include <deque>
#include <iomanip>
#include <map>
#include <sstream>

// -----------------------------------------------------------------------------
// The following is used to ensure that the QRC file
// containing the entity-type icons is registered.
// This is required when building SMTK with static libraries.
// Note that the inlined functions may *NOT* be placed inside a namespace
// according to the Qt docs here:
//   http://qt-project.org/doc/qt-5.0/qtcore/qdir.html#Q_INIT_RESOURCE
static int resourceCounter = 0;

inline void initIconResource()
{
  if (resourceCounter <= 0)
    {
    Q_INIT_RESOURCE(qtEntityItemModelIcons);
    }
  ++resourceCounter;
}

inline void cleanupIconResource()
{
  if (--resourceCounter == 0)
    {
    Q_CLEANUP_RESOURCE(qtEntityItemModelIcons);
    }
}
// -----------------------------------------------------------------------------

namespace smtk {
  namespace model {

/// Private storage for QEntityItemModel.
class QEntityItemModel::Internal
{
public:
  /**\brief Store a map of weak pointers to phrases by their phrase IDs.
    *
    * We hold a strong pointer to the root phrase in QEntityItemModel::m_root.
    * This map is a reverse lookup of pointers to subphrases by integer handles
    * that Qt can associate with QModelIndex entries.
    *
    * This all exists because Qt is lame and cannot associate shared pointers
    * with QModelIndex entries.
    */
  std::map<unsigned int,WeakDescriptivePhrasePtr> ptrs;
};

// A visitor functor called by foreach_phrase() to let the view know when to redraw data.
static bool UpdateSubphrases(QEntityItemModel* qmodel, const QModelIndex& qidx, const EntityRef& ent)
{
  DescriptivePhrasePtr phrase = qmodel->getItem(qidx);
  if (phrase)
    {
    if (phrase->relatedEntity() == ent)
      {
      qmodel->rebuildSubphrases(qidx);
      }
    }
  return false; // Always visit every phrase, since \a ent may appear multiple times.
}

// Callback function, invoked when a new arrangement is added to an entity.
static int entityModified(ManagerEventType, const smtk::model::EntityRef& ent, const smtk::model::EntityRef&, void* callData)
{
  QEntityItemModel* qmodel = static_cast<QEntityItemModel*>(callData);
  if (!qmodel)
    return 1;

  // Find EntityPhrase instances under the root node whose relatedEntity
  // is \a ent and rerun the subphrase generator.
  // This should in turn invoke callbacks on the QEntityItemModel
  // to handle insertions/deletions.
  return qmodel->foreach_phrase(UpdateSubphrases, ent) ? -1 : 0;
}

// -----------------------------------------------------------------------------
QEntityItemModel::QEntityItemModel(QObject* owner)
  : QAbstractItemModel(owner)
{
  this->m_deleteOnRemoval = true;
  this->P = new Internal;
  initIconResource();
}

QEntityItemModel::~QEntityItemModel()
{
  cleanupIconResource();
  delete this->P;
}

//-----------------------------------------------------------------------------
void QEntityItemModel::clear()
{
  if(this->m_root && !this->m_root->subphrases().empty())
    {
    this->beginRemoveRows(index(0,0,QModelIndex()), 0, this->m_root->subphrases().size());
    this->m_root = DescriptivePhrasePtr();
    this->endRemoveRows();
    }
}

QModelIndex QEntityItemModel::index(int row, int column, const QModelIndex& owner) const
{
  if(!this->m_root || this->m_root->subphrases().empty())
    return QModelIndex();

  if (owner.isValid() && owner.column() != 0)
    return QModelIndex();

  int rows = this->rowCount(owner);
  int columns = this->columnCount(owner);
  if(row < 0 || row >= rows || column < 0 || column >= columns)
    {
    return QModelIndex();
    }

  DescriptivePhrasePtr ownerPhrase = this->getItem(owner);
  std::string entName = ownerPhrase->relatedEntity().name();
//  std::cout << "Owner index for: " << entName << std::endl;
  DescriptivePhrases& subphrases(ownerPhrase->subphrases());
  if (row >= 0 && row < static_cast<int>(subphrases.size()))
    {
    //std::cout << "index(_"  << ownerPhrase->title() << "_, " << row << ") = " << subphrases[row]->title() << "\n";
    //std::cout << "index(_"  << ownerPhrase->phraseId() << "_, " << row << ") = " << subphrases[row]->phraseId() << ", " << subphrases[row]->title() << "\n";

    DescriptivePhrasePtr entry = subphrases[row];
    this->P->ptrs[entry->phraseId()] = entry;
    return this->createIndex(row, column, entry->phraseId());
    }

  return QModelIndex();
}

QModelIndex QEntityItemModel::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    {
    return QModelIndex();
    }

  DescriptivePhrasePtr childPhrase = this->getItem(child);
  DescriptivePhrasePtr parentPhrase = childPhrase->parent();
  if (parentPhrase == this->m_root)
    {
    return QModelIndex();
    }

  int rowInGrandparent = parentPhrase ? parentPhrase->indexInParent() : -1;
  if (rowInGrandparent < 0)
    {
    //std::cerr << "parent(): could not find child " << childPhrase->title() << " in parent " << parentPhrase->title() << "\n";
    return QModelIndex();
    }
  this->P->ptrs[parentPhrase->phraseId()] = parentPhrase;
  return this->createIndex(rowInGrandparent, 0, parentPhrase->phraseId());
}

/// Return true when \a owner has subphrases.
bool QEntityItemModel::hasChildren(const QModelIndex& owner) const
{
  // According to various Qt mailing-list posts, we might
  // speed things up by always returning true here.
  if (owner.isValid())
    {
    DescriptivePhrasePtr phrase = this->getItem(owner);
    if (phrase)
      { // Return whether the parent has subphrases.
      return phrase->subphrases().empty() ? false : true;
      }
    }
  // Return whether the toplevel m_phrases list is empty.
  return (this->m_root ?
    (this->m_root->subphrases().empty() ? false : true) :
    false);
}

/// The number of rows in the table "underneath" \a owner.
int QEntityItemModel::rowCount(const QModelIndex& owner) const
{
  DescriptivePhrasePtr ownerPhrase = this->getItem(owner);
  return static_cast<int>(ownerPhrase->subphrases().size());
}

/// Return something to display in the table header.
QVariant QEntityItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    {
    return QVariant();
    }
  if (orientation == Qt::Horizontal)
    {
    switch (section)
      {
    case 0:
      return "Entity"; // "Type";
    case 1:
      return "Dimension";
    case 2:
      return "Name";
      }
    }
  // ... could add "else" here to present per-row header.
  return QVariant();
}

/// Relate information, by its \a role, from a \a DescriptivePhrasePtrto the Qt model.
QVariant QEntityItemModel::data(const QModelIndex& idx, int role) const
{
  if (idx.isValid())
    {
    // A valid index may have a *parent* that is:
    // + invalid (in which case we use row() as an offset into m_phrases to obtain data)
    // + valid (in which case it points to a DescriptivePhrasePtrinstance and row() is an offset into the subphrases)

    DescriptivePhrasePtr item = this->getItem(idx);
    if (item)
      {
      if (role == TitleTextRole)
        {
        return QVariant(item->title().c_str());
        }
      else if (role == SubtitleTextRole)
        {
        return QVariant(item->subtitle().c_str());
        }
      else if (role == EntityIconRole)
        {
        return QVariant(
          this->lookupIconForEntityFlags(
            item->phraseType()));
        }
      else if (role == EntityVisibilityRole)
        {
        // by default, everything should be visible
        bool visible = true;

        if(item->phraseType() == MESH_SUMMARY)
          {
          MeshPhrasePtr mphrase = smtk::dynamic_pointer_cast<MeshPhrase>(item);
          smtk::mesh::MeshSet meshkey;
          smtk::mesh::CollectionPtr c;
          if(!mphrase->relatedMesh().is_empty())
            {
            meshkey = mphrase->relatedMesh();
            c = meshkey.collection();
            }
          else
            {
            c = mphrase->relatedMeshCollection();
            meshkey = c->meshes();
            }
          if(c && !meshkey.is_empty() && c->hasIntegerProperty(meshkey, "visible"))
            {
            const IntegerList& prop(c->integerProperty(meshkey, "visible"));
            if(!prop.empty())
              visible = (prop[0] != 0);
            }
          }
        else if(item->relatedEntity().isValid() && item->relatedEntity().hasVisibility())
          {
          const IntegerList& prop(item->relatedEntity().integerProperty("visible"));
          if(!prop.empty())
            visible = (prop[0] != 0);
          }

        if (visible)
          return QVariant(QIcon(":/icons/display/eyeball_16.png"));
        else
          return QVariant(QIcon(":/icons/display/eyeballx_16.png"));
        }
      else if (role == EntityColorRole &&
        (item->phraseType() == MESH_SUMMARY || item->relatedEntity().isValid()))
        {
        QColor color;
        FloatList rgba = item->relatedColor();
        if (rgba.size() >= 4 && rgba[3] < 0)
          color = QColor(255, 255, 255, 255);
        else
          {
          // Color may be luminance, luminance+alpha, rgb, or rgba:
          switch (rgba.size())
            {
          case 0:
            color = QColor(0, 0, 0, 0);
            break;
          case 1:
            color.setHslF(0., 0., rgba[0], 1.);
            break;
          case 2:
            color.setHslF(0., 0., rgba[0], rgba[1]);
            break;
          case 3:
            color.setRgbF(rgba[0], rgba[1], rgba[2], 1.);
            break;
          case 4:
          default:
            color.setRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
            break;
            }
          }
        return color;
        }
      }
    }
  return QVariant();
}
/*
bool QEntityItemModel::insertRows(int position, int rows, const QModelIndex& owner)
{
  DescriptivePhrasePtr phrase = this->getItem(owner);
  if(!phrase || !phrase->relatedEntity().isValid())
    return false;

//      this->beginInsertRows(qidx, row, row);

//      parntDp->subphrases().insert(
//        parntDp->subphrases().begin() + row, it->first);

//subrefs.push_back(it->first);
//      parntDp->insertSubphrase(row, *it);
//    parntDp->markDirty(true);
//      this->endInsertRows();

//  (void)owner;
  beginInsertRows(owner, position, position + rows - 1);

  DescriptivePhrases& subrefs(phrase->subphrases());
  subrefs
  int maxPos = position + rows;
  for (int row = position; row < maxPos; ++row)
    {
    smtk::common::UUID uid = this->m_manager->addEntityOfTypeAndDimension(smtk::model::INVALID, -1);
    this->m_phrases.insert(this->m_phrases.begin() + row, uid);
    this->m_reverse[uid] = row;
    }

  endInsertRows();
  return true;
}
*/
/// Remove rows from the model.
bool QEntityItemModel::removeRows(int position, int rows, const QModelIndex& parentIdx)
{
  if (rows <= 0 || position < 0)
    return false;

  this->beginRemoveRows(parentIdx, position, position + rows - 1);
  DescriptivePhrasePtr phrase = this->getItem(parentIdx);
  if (phrase)
    {
    phrase->subphrases().erase(
      phrase->subphrases().begin() + position,
      phrase->subphrases().begin() + position + rows);
    }
  this->endRemoveRows();
  return true;
}

bool QEntityItemModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
  bool didChange = false;
  DescriptivePhrasePtr phrase;
  if(idx.isValid() &&
    (phrase = this->getItem(idx)))
    {
    if (role == TitleTextRole && phrase->isTitleMutable())
      {
      std::string sval = value.value<QString>().toStdString();
      didChange = phrase->setTitle(sval);
      // if data did get changed, we need to emit the signal
      if(didChange)
        emit this->phraseTitleChanged(idx);
      }
    else if (role == SubtitleTextRole && phrase->isSubtitleMutable())
      {
      std::string sval = value.value<QString>().toStdString();
      didChange = phrase->setSubtitle(sval);
      }
    else if (role == EntityColorRole && phrase->isRelatedColorMutable()
            && phrase->relatedEntity().isValid())
      {
      QColor color = value.value<QColor>();
      FloatList rgba(4);
      rgba[0] = color.redF();
      rgba[1] = color.greenF();
      rgba[2] = color.blueF();
      rgba[3] = color.alphaF();
      didChange = phrase->setRelatedColor(rgba);
      }
    else if (role == EntityVisibilityRole && phrase->relatedEntity().isValid())
      {
      int vis = value.toInt();
      phrase->relatedEntity().setVisible(vis > 0 ? true : false);
      didChange = true;
      }
    }
  return didChange;
}

/*
void QEntityItemModel::sort(int column, Qt::SortOrder order)
{
  switch (column)
    {
  case -1:
      { // Sort by UUID.
      std::multiset<smtk::common::UUID> sorter;
      this->sortDataWithContainer(sorter, order);
      }
    break;
  case 1:
      {
      smtk::model::SortByEntityFlags comparator(this->m_manager);
      std::multiset<smtk::common::UUID,smtk::model::SortByEntityFlags>
        sorter(comparator);
      this->sortDataWithContainer(sorter, order);
      }
    break;
  case 0:
  case 2:
      {
      smtk::model::SortByEntityProperty<
        smtk::model::Manager,
        smtk::model::StringList,
        &smtk::model::Manager::stringProperty,
        &smtk::model::Manager::hasStringProperty> comparator(
          this->m_manager, "name");
      std::multiset<
        smtk::common::UUID,
        smtk::model::SortByEntityProperty<
          smtk::model::Manager,
          smtk::model::StringList,
          &smtk::model::Manager::stringProperty,
          &smtk::model::Manager::hasStringProperty> >
        sorter(comparator);
      this->sortDataWithContainer(sorter, order);
      }
    break;
  default:
    std::cerr << "Bad column " << column << " for sorting\n";
    break;
    }
  emit dataChanged(
    this->index(0, 0, QModelIndex()),
    this->index(
      static_cast<int>(this->m_phrases.size()),
      this->columnCount(),
      QModelIndex()));
}
*/

/// Provide meta-information about a model entry.
Qt::ItemFlags QEntityItemModel::flags(const QModelIndex& idx) const
{
  if(!idx.isValid())
    return Qt::ItemIsEnabled;

  // TODO: Check to make sure column is not "information-only".
  //       We don't want to allow people to randomly edit an enum string.
  Qt::ItemFlags itemFlags = QAbstractItemModel::flags(idx) |
     Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
  DescriptivePhrasePtr dp = this->getItem(idx);
  if( dp && dp->relatedEntity().isGroup() )
    itemFlags = itemFlags | Qt::ItemIsDropEnabled;
  return itemFlags;
}

static bool FindManager(const QEntityItemModel* qmodel, const QModelIndex& qidx, ManagerPtr& manager)
{
  DescriptivePhrasePtr phrase = qmodel->getItem(qidx);
  if (phrase)
    {
    EntityRef related = phrase->relatedEntity();
    if (related.isValid())
      {
      manager = related.manager();
      if (manager)
        return true;
      }
    }
  return false;
}

/**\brief Return the first smtk::model::Manager instance presented by this model.
  *
  * Note that it is possible for a QEntityItemModel to present information
  * on entities from multiple Manager instances.
  * However, in this case, external updates to the selection must either be
  * made via EntityRef instances (which couple UUIDs with Manager instances) or
  * there will be breakage.
  */
smtk::model::ManagerPtr QEntityItemModel::manager() const
{
  ManagerPtr store;
  this->foreach_phrase(FindManager, store, QModelIndex(), false);
  return store;
}

QIcon QEntityItemModel::lookupIconForEntityFlags(smtk::model::BitFlags flags)
{
  std::ostringstream resourceName;
  resourceName << ":/icons/entityTypes/";
  bool dimBits = true;
  switch (flags & ENTITY_MASK)
    {
  case CELL_ENTITY:
    resourceName << "cell";
    break;
  case USE_ENTITY:
    resourceName << "use";
    break;
  case SHELL_ENTITY:
    resourceName << "shell";
    break;
  case GROUP_ENTITY:
    resourceName << "group";
    break;
  case MODEL_ENTITY:
    resourceName << "model";
    break;
  case INSTANCE_ENTITY:
    resourceName << "instance";
    break;
  default:
    resourceName << "invalid";
    dimBits = false;
    break;
    }
  if (dimBits)
    {
    resourceName
      << "_"
      << std::setbase(16) << std::fixed << std::setw(2) << std::setfill('0')
      << (flags & ANY_DIMENSION)
      ;
    }
  resourceName << ".png";
  QFile rsrc(resourceName.str().c_str());
  if (!rsrc.exists())
    { // FIXME: Replace with the path of a "generic entity" or "invalid" icon.
    return QIcon(":/icons/entityTypes/cell_08.png");
    }
  return QIcon(resourceName.str().c_str());
}

/**\brief Sort the UUIDs being displayed using the given ordered container.
  *
  * The ordered container's comparator is used to insertion-sort the UUIDs
  * displayed. Then, the \a order is used to either forward- or reverse-iterator
  * over the container to obtain a new ordering for the UUIDs.
template<typename T>
void QEntityItemModel::sortDataWithContainer(T& sorter, Qt::SortOrder order)
{
  smtk::common::UUIDArray::iterator ai;
  // Insertion into the set sorts the UUIDs.
  for (ai = this->m_phrases.begin(); ai != this->m_phrases.end(); ++ai)
    {
    sorter.insert(*ai);
    }
  // Now we reset m_phrases and m_reverse and recreate based on the sorter's order.
  this->m_phrases.clear();
  this->m_reverse.clear();
  int i;
  if (order == Qt::AscendingOrder)
    {
    typename T::iterator si;
    for (i = 0, si = sorter.begin(); si != sorter.end(); ++si, ++i)
      {
      this->m_phrases.push_back(*si);
      this->m_reverse[*si] = i;
      / *
      std::cout << i << "  " << *si << "  " <<
        (this->m_manager->hasStringProperty(*si, "name") ?
         this->m_manager->stringProperty(*si, "name")[0].c_str() : "--") << "\n";
         * /
      }
    }
  else
    {
    typename T::reverse_iterator si;
    for (i = 0, si = sorter.rbegin(); si != sorter.rend(); ++si, ++i)
      {
      this->m_phrases.push_back(*si);
      this->m_reverse[*si] = i;
      / *
      std::cout << i << "  " << *si << "  " <<
        (this->m_manager->hasStringProperty(*si, "name") ?
         this->m_manager->stringProperty(*si, "name")[0].c_str() : "--") << "\n";
         * /
      }
    }
}
  */

/// A utility function to retrieve the DescriptivePhrasePtr associated with a model index.
DescriptivePhrasePtr QEntityItemModel::getItem(const QModelIndex& idx) const
{
  if (idx.isValid())
    {
    unsigned int phraseIdx = idx.internalId();
    std::map<unsigned int,WeakDescriptivePhrasePtr>::iterator it;
    if ((it = this->P->ptrs.find(phraseIdx)) == this->P->ptrs.end())
      {
      //std::cout << "  Missing index " << phraseIdx << "\n";
      std::cout.flush();
      return this->m_root;
      }
    WeakDescriptivePhrasePtr weakPhrase = it->second;
    DescriptivePhrasePtr phrase;
    if ((phrase = weakPhrase.lock()))
      {
      return phrase;
      }
    else
      { // The phrase has disappeared. Remove the weak pointer from the freelist.
      this->P->ptrs.erase(phraseIdx);
      }
    }
  return this->m_root;
}

void QEntityItemModel::rebuildSubphrases(const QModelIndex& qidx)
{
  int nrows = this->rowCount(qidx);
  DescriptivePhrasePtr phrase = this->getItem(qidx);

  this->removeRows(0, nrows, qidx);
  if (phrase)
    phrase->markDirty(true);

  nrows = phrase ? static_cast<int>(phrase->subphrases().size()) : 0;
  this->beginInsertRows(qidx, 0, nrows);
  this->endInsertRows();
  emit dataChanged(qidx, qidx);
}

/*
inline QModelIndex _internal_getPhraseIndex(
  smtk::model::QEntityItemModel* qmodel,
  const smtk::model::EntityRef& entRef,
  const QModelIndex& top, bool recursive = false)
{
  for (int row = 0; row < qmodel->rowCount(top); ++row)
    {
    QModelIndex cIdx(qmodel->index(row, 0, top));
    DescriptivePhrasePtr dp = qmodel->getItem(cIdx);
    if(dp && (dp->relatedEntity() == entRef))
      return cIdx;
    if(recursive)
      {
      QModelIndex recurIdx(_internal_getPhraseIndex(qmodel, entRef, cIdx, recursive));
      if(recurIdx.isValid())
        return recurIdx;
      }
    }
  return QModelIndex();
}
*/

inline QModelIndex _internal_getPhraseIndex(
  smtk::model::QEntityItemModel* qmodel,
  const DescriptivePhrasePtr& phrase,
  const QModelIndex& top, bool recursive = false)
{
  DescriptivePhrasePtr dp = qmodel->getItem(top);
  if(dp == phrase)
    return top;

  for (int row = 0; row < qmodel->rowCount(top); ++row)
    {
    QModelIndex cIdx(qmodel->index(row, 0, top));
    dp = qmodel->getItem(cIdx);
    if(dp == phrase)
      return cIdx;
    if(recursive)
      {
      QModelIndex recurIdx(_internal_getPhraseIndex(qmodel, phrase, cIdx, recursive));
      if(recurIdx.isValid())
        return recurIdx;
      }
    }
  return QModelIndex();
}

inline void _internal_findAllExistingPhrases(
  const DescriptivePhrasePtr& parntDp,
  const smtk::attribute::ModelEntityItemPtr& modEnts,
  DescriptivePhrases& modifiedPhrases)
{
  if(!parntDp || !parntDp->areSubphrasesBuilt())
    return;

  smtk::model::DescriptivePhrases& subs(parntDp->subphrases());
  for (smtk::model::DescriptivePhrases::iterator it = subs.begin();
    it != subs.end(); ++it)
    {
    if (modEnts->has((*it)->relatedEntity()))
      {
      modifiedPhrases.push_back(*it);
      }

    // Descend 
    _internal_findAllExistingPhrases(*it, modEnts, modifiedPhrases);
    }
}

inline void _internal_findAllExistingMeshPhrases(
  const DescriptivePhrasePtr& parntDp,
  const smtk::attribute::MeshItemPtr& modMeshes,
  DescriptivePhrases& modifiedPhrases)
{
  if(!parntDp || !parntDp->areSubphrasesBuilt())
    return;

  smtk::model::DescriptivePhrases& subs(parntDp->subphrases());
  for (smtk::model::DescriptivePhrases::iterator it = subs.begin();
    it != subs.end(); ++it)
    {
    if (modMeshes->hasValue((*it)->relatedMesh()))
      {
      modifiedPhrases.push_back(*it);
      }

    // Descend 
    _internal_findAllExistingMeshPhrases(*it, modMeshes, modifiedPhrases);
    }
}

void QEntityItemModel::addChildPhrases(
    const DescriptivePhrasePtr& parntDp,
    const std::vector< std::pair<DescriptivePhrasePtr, int> > & newDphrs,
    const QModelIndex& topIndex, bool descend)
{
  QModelIndex qidx;
  if(parntDp != this->m_root)
    {
    qidx = (_internal_getPhraseIndex(
      this, parntDp, topIndex, descend));
    if(!qidx.isValid())
      {
      std::cerr << "Can't find valid QModelIndex for phrase: " << parntDp->title() << "\n";
      return;  
      }
    }

  if(!parntDp->areSubphrasesBuilt())
    {
    this->rebuildSubphrases(qidx);
    return;
    }

  DescriptivePhrases& subrefs(parntDp->subphrases());
  int row = 0;
  // iterate all subphrases, if child is part of newDphrs, insert an QModelIndex
  // smtk::model::DescriptivePhrases subs = parntDp->subphrases();
  // DescriptivePhrases listPhrases;
  std::vector< std::pair<DescriptivePhrasePtr, int> >::const_iterator it;
  for (it = newDphrs.begin(); it != newDphrs.end(); ++it)
    {
  /*
    if(it->first->phraseType() == ENTITY_LIST)
      listPhrases.insert(listPhrases.end(),
        it->first->subphrases().begin(),
        it->first->subphrases().end());
  */
    // this sub is new, insert the QModelIndex for it
    row = it->second;
    this->beginInsertRows(qidx, row, row);
    if(row >= static_cast<int>(subrefs.size()))
        subrefs.push_back(it->first);
    else
      subrefs.insert(subrefs.begin() + row, it->first);

    this->endInsertRows();
    }
/* TODO: We need to handle the case when adding new subphrases will create a Entity_List

  if(listPhrases.size() > 0)
    {
    //remove every subphrases that are both in the new listPhrases and parent phrase
    std::vector< std::pair<DescriptivePhrasePtr, int> > remDphrs;
    int newIdx = 0;
    for (smtk::model::DescriptivePhrases::iterator pit = subrefs.begin();
      pit != subrefs.end(); ++pit, ++newIdx)
      {
      EntityRef related = (*pit)->relatedEntity();
      for (smtk::model::DescriptivePhrases::iterator lit = listPhrases.begin();
        lit != listPhrases.end(); ++lit)
        {
        if(related == (*lit)->relatedEntity())
          {
          remDphrs.push_back(std::make_pair(*pit, newIdx));
          break;
          }
        }
      }
    if(remDphrs.size() > 0)
      {
      this->removeChildPhrases(parntDp, remDphrs, qidx);
      }
    }
  else
*/
  emit dataChanged(qidx, qidx);
  if(newDphrs.size() > 0)
    {
    QModelIndex lastnewChild = qidx.child(row, 0);
    // give view an opportunity to scroll to the new child
    emit newIndexAdded(lastnewChild);
    }
}

void QEntityItemModel::removeChildPhrases(
    const DescriptivePhrasePtr& parntDp,
    const std::vector< std::pair<DescriptivePhrasePtr, int> > & remDphrs,
    const QModelIndex& topIndex)
{
  QModelIndex qidx(_internal_getPhraseIndex(
    this, parntDp, topIndex, true));
  if(!qidx.isValid())
    {
    std::cerr << "Can't find valid QModelIndex for phrase: " << parntDp->title() << "\n";
    return;  
    }

  std::vector< std::pair<DescriptivePhrasePtr, int> >::const_reverse_iterator rit;
  int row = 0;

  // if child is part of remDphrs, remove the QModelIndex
  DescriptivePhrases& subs(parntDp->subphrases());
  for (rit = remDphrs.rbegin(); rit != remDphrs.rend(); ++rit)
    {
    row = rit->second;
//      this->removeRows(row, row, qidx);
    this->beginRemoveRows(qidx, row, row);
    subs.erase(subs.begin() + row);

    this->endRemoveRows();
    }
/* TODO: We need to handle the case where the parent is a Entity_List
// and removing the children could make the entity_list being removed too.
  if(parntDp->phraseType == ENTITY_LIST &&
     subs.size() < parntDp->findDelegate()->directLimit())
    {
    // move parnt's children up as parnt sibling, and remove parnt
    QModelIndex parentIdx = qidx.parent();

    int parId = parnt->indexInParent();
    this->beginRemoveRows(parentIdx, parId, parId);
    parnt->parent()->subphrases().erase(
      parnt->parent()->subphrases().begin() + parId);
    this->endRemoveRows();

    int startIdx = parId;
    for (smtk::model::DescriptivePhrases::iterator it = subs.begin();
      it != subs.end(); ++it, ++startIdx)
      {
      this->beginInsertRows(parentIdx, startIdx, startIdx);
      parnt->parent()->subphrases().insert(
        parnt->parent()->subphrases().begin() + startIdx, *it);
      this->endInsertRows();
      }

    }
  else
  */
    emit dataChanged(qidx, qidx);
}

void QEntityItemModel::updateChildPhrases(
  const DescriptivePhrasePtr& phrase, const QModelIndex& topIndex)
{
  if(!phrase)
    {
    std::cerr << "The input phrase is null!\n";
    return;  
    }

  // we need to rebuild a temporary subphrases using the same subphrase generator
  // to figure out whether new entities are added or entities are removed
  // from the \a ent (such as add to or remove from a group)
  // This will NOT update subphrases of parntDp, and should not.

  smtk::model::DescriptivePhrases newSubs =
    this->m_root->findDelegate()->subphrases(phrase);
  smtk::model::DescriptivePhrases& origSubs(phrase->subphrases());

  // find what's going to be removed with proper indices
  std::vector< std::pair<DescriptivePhrasePtr, int> > remDphrs;
  int remIdx = 0;
  bool found = false;
  for (smtk::model::DescriptivePhrases::iterator it = origSubs.begin();
    it != origSubs.end(); ++it, ++remIdx)
    {
    EntityRef related = (*it)->relatedEntity();
    found = false;
    for (smtk::model::DescriptivePhrases::iterator nit = newSubs.begin();
      nit != newSubs.end(); ++nit)
      {
      if(related == (*nit)->relatedEntity())
        {
        found = true;
        break;
        }
      }

    if( !found )
      remDphrs.push_back(std::make_pair(*it, remIdx));
    }

  // find what's new with proper indices
  std::vector< std::pair<DescriptivePhrasePtr, int> > newDphrs;
  int newIdx = 0;
  for (smtk::model::DescriptivePhrases::iterator it = newSubs.begin();
    it != newSubs.end(); ++it, ++newIdx)
    {
    int origId = phrase->argFindChild((*it)->relatedEntity());
    if( origId < 0 )
      newDphrs.push_back(std::make_pair(*it, newIdx));
    }

  // remove non-existing first, then add new ones
  if(remDphrs.size() > 0)
    this->removeChildPhrases(phrase, remDphrs, topIndex);
  if(newDphrs.size() > 0)
    this->addChildPhrases(phrase, newDphrs, topIndex);
  // subphrases did not change, just emit dataChanged signal
  if(remDphrs.size() == 0 && newDphrs.size() == 0)
    {
    QModelIndex qidx(_internal_getPhraseIndex(
      this, phrase, topIndex, true));
    if(!qidx.isValid())
      {
      std::cerr << "Can't find valid QModelIndex for phrase: " << phrase->title() << "\n";
      return;  
      }

    emit dataChanged(qidx, qidx);
    }
}

void QEntityItemModel::findDirectParentPhrasesForRemove(
  const DescriptivePhrasePtr& parntDp,
  const smtk::attribute::ModelEntityItemPtr& remEnts,
  std::map<DescriptivePhrasePtr,
    std::vector< std::pair<DescriptivePhrasePtr, int> > >& changedPhrases)
{
  EntityRef relatedParEnt = parntDp->relatedEntity();
  // if the parntDp related entity is being removed, do not descend
  if (remEnts->has(relatedParEnt))
    return;

  smtk::model::DescriptivePhrases& origSubs(parntDp->subphrases());
  int newIdx = 0;
  for (smtk::model::DescriptivePhrases::iterator it = origSubs.begin();
    it != origSubs.end(); ++it, ++newIdx)
    {
    EntityRef related = (*it)->relatedEntity();
    if (remEnts->has(related)/*!(*it)->relatedEntity().isValid()*/)
      {
      // this parent has the subphrase for \a ent
      // so it is changed
      // parntDp->removeSubphrase(*it); // Don't remove them yet.
      changedPhrases[parntDp].push_back(std::make_pair(*it, newIdx));
      }
    else
      this->findDirectParentPhrasesForRemove(*it, remEnts, changedPhrases);
    }
}

void QEntityItemModel::findDirectParentPhrasesForAdd(
  const DescriptivePhrasePtr& parntDp,
  const smtk::attribute::ModelEntityItemPtr& newEnts,
  std::map<DescriptivePhrasePtr,
    std::vector< std::pair<DescriptivePhrasePtr, int> > >& changedPhrases
    /*,std::map<DescriptivePhrasePtr, DescriptivePhrases>& newLists*/)
{
  smtk::model::DescriptivePhrases newSubs =
    this->m_root->findDelegate()->subphrases(parntDp);
  int newIdx = 0;
  for (smtk::model::DescriptivePhrases::iterator it = newSubs.begin();
    it != newSubs.end(); ++it, ++newIdx)
    {
    EntityRef related = (*it)->relatedEntity();
    if (newEnts->has(related))
      {
      // this parent has the subphrase for \a ent
      // so it is changed
      // parntDp->removeSubphrase(*it); // Don't remove them yet.
      changedPhrases[parntDp].push_back(std::make_pair(*it, newIdx));
      }
    else
      {
      int origId = parntDp->argFindChild(related);
    // we only want to descend with the built subphrases that
    // are already inside its parents.
      if( origId >= 0 )
        this->findDirectParentPhrasesForAdd(parntDp->subphrases()[origId],
            newEnts, changedPhrases/*, newLists*/);
      }
/*
    else if( (*it)->phraseType() == ENTITY_LIST)
      {
      // in case this is a newly generated entity_list phrase, due to number
      // of total entities triggered a new list entity being build
      changedPhrases[parntDp].push_back(std::make_pair(*it, newIdx));
      newLists[parntDp].push_back(*it);
//        this->findDirectParentPhrases(parntDp->subphrases()[origId],
//                                      ent, changedPhrases, onlyBuilt);
      }
*/
    }
}

void QEntityItemModel::updateWithOperatorResult(
    const QModelIndex& sessIndex,
    const OperatorResult& result)
{
  if(!sessIndex.isValid())
    {
    std::cerr << "The input QModelIndex is not valid!\n";
    return;  
    }
  DescriptivePhrasePtr startPhr = this->getItem(sessIndex);
  // We only searching those that already have subphrases built.
  // because
  if(!startPhr)
    return;

  // looking for the parent of the new entities
  // process "created" in result to figure out if there are new cell entities
  std::map<DescriptivePhrasePtr,  std::vector< std::pair<DescriptivePhrasePtr, int> > > changedPhrases;
  std::map<DescriptivePhrasePtr,  std::vector< std::pair<DescriptivePhrasePtr, int> > >::const_iterator pit;
  smtk::model::EntityRefArray::const_iterator it;
  smtk::attribute::ModelEntityItem::Ptr remEnts =
    result->findModelEntity("expunged");
  if(remEnts && remEnts->numberOfValues() > 0 && startPhr->areSubphrasesBuilt())
    {
    this->findDirectParentPhrasesForRemove(startPhr, remEnts, changedPhrases);
    for(pit = changedPhrases.begin(); pit != changedPhrases.end(); ++pit)
      this->removeChildPhrases(pit->first, pit->second, sessIndex);
    }

  changedPhrases.clear();
  smtk::attribute::ModelEntityItem::Ptr newEnts =
    result->findModelEntity("created");
  if(newEnts && newEnts->numberOfValues() > 0)
    {
//    std::map<DescriptivePhrasePtr, DescriptivePhrases>& newLists;
    this->findDirectParentPhrasesForAdd(startPhr, newEnts, changedPhrases);
/*
    // If there are new lists, the relatedEntities in the new lists need to be removed
    // from their current parents first
    std::map<DescriptivePhrasePtr, DescriptivePhrases>::const_iterator newit;
    for(newit = newLists.begin(); newit != newLists.end(); ++newit)
      {
      DescriptivePhrases& tmpphrases(newit->first->subphrases());
      DescriptivePhrases::const_iterator currit;
      // if this subphrase is already in one of the new list, remove it.
      std::vector< std::pair<DescriptivePhrasePtr, int> > remDphrs;
      int rmrow = 0;
      for(currit = tmpphrases.begin(); currit != tmpphrases.end(); ++currit, ++rmrow)
        {
        DescriptivePhrases::const_iterator lit;
        for(lit = newit->second.begin(); lit != newit->second.end(); ++lit)
          {
          EntityListPhrasePtr lphrase = smtk::dynamic_pointer_cast<EntityListPhrase>(*lit);
          if (!lphrase)
            continue;
          if(std::find(lphrase->relatedEntities().begin(),
                    lphrase->relatedEntities().end(),
                    (*currit)->relatedEntity())
                  != lphrase->relatedEntities().end())
            {
            remDphrs.push_back(std::make_pair(*currit, rmrow));
            break;
            }
          }
        }
      if(remDphrs.size() > 0)
        this->removeChildPhrases(newit->first, remDphrs, sessIndex)
      }
*/
    for(pit = changedPhrases.begin(); pit != changedPhrases.end(); ++pit)
      this->addChildPhrases(pit->first, pit->second, sessIndex);
    }

  DescriptivePhrases modifiedPhrases;
  smtk::attribute::ModelEntityItem::Ptr modEnts =
    result->findModelEntity("modified");
  if(modEnts && modEnts->numberOfValues() > 0)
    {
    _internal_findAllExistingPhrases(startPhr, modEnts, modifiedPhrases);
    smtk::model::DescriptivePhrases::iterator mit;
    for(mit = modifiedPhrases.begin(); mit != modifiedPhrases.end(); ++mit)
      this->updateChildPhrases(*mit, sessIndex);
    }

  // mesh properties modifed
  DescriptivePhrases modifiedMeshPhrases;
  smtk::attribute::MeshItem::Ptr modifiedMeshes =
    result->findMesh("mesh_modified");
  if(modifiedMeshes && modifiedMeshes->numberOfValues() > 0)
    {
    _internal_findAllExistingMeshPhrases(startPhr, modifiedMeshes, modifiedMeshPhrases);
    smtk::model::DescriptivePhrases::iterator mit;
    for(mit = modifiedMeshPhrases.begin(); mit != modifiedMeshPhrases.end(); ++mit)
      {
//      (*mit)->setup()
      QModelIndex qidx(_internal_getPhraseIndex(
        this, *mit, sessIndex, true));
      if(!qidx.isValid())
        {
        std::cerr << "Can't find valid QModelIndex for phrase: " << (*mit)->title() << "\n";
        return;  
        }

      emit dataChanged(qidx, qidx);
      }
    }
}

void QEntityItemModel::newSessionOperatorResult(
    const smtk::model::SessionRef& sref, const OperatorResult& result)
{
  if(!sref.isValid())
    {
    std::cerr << "The input SessionRef is not valid!\n";
    return;  
    }

  EntityListPhrasePtr lphrase = smtk::dynamic_pointer_cast<EntityListPhrase>(this->m_root);
  if (lphrase)
    lphrase->relatedEntities().push_back(sref);

  smtk::model::DescriptivePhrases newSubs =
    this->m_root->findDelegate()->subphrases(this->m_root);
  std::vector< std::pair<DescriptivePhrasePtr, int> > newDphrs;
  int newIdx = 0;
  for (smtk::model::DescriptivePhrases::iterator it = newSubs.begin();
    it != newSubs.end(); ++it, ++newIdx)
    {
    if(sref == (*it)->relatedEntity())
      {
      newDphrs.push_back(std::make_pair(*it, newIdx));
      break;
      }
    }
  if(newDphrs.size() > 0)
    {
    this->addChildPhrases(this->m_root, newDphrs, QModelIndex(), false);
    // hopefully, we have the proper index for the new session
    QModelIndex top;
    for (int row = 0; row < this->rowCount(top); ++row)
      {
      QModelIndex sessIdx = this->index(row, 0, top);
      DescriptivePhrasePtr dp = this->getItem(sessIdx);
      if(dp && (dp->relatedEntity() == sref))
        {
        this->updateWithOperatorResult(sessIdx, result);
        return;
        }
      }
    }
}

void QEntityItemModel::updateObserver()
{
  ManagerPtr store = this->manager();
  if (store)
    {
    store->observe(std::make_pair(ANY_EVENT,MODEL_INCLUDES_FREE_CELL), &entityModified, this);
    store->observe(std::make_pair(ANY_EVENT,MODEL_INCLUDES_GROUP), &entityModified, this);
    store->observe(std::make_pair(ANY_EVENT,MODEL_INCLUDES_MODEL), &entityModified, this);
    // Group membership changing
    store->observe(std::make_pair(ANY_EVENT,GROUP_SUPERSET_OF_ENTITY), &entityModified, this);
    }
}

  } // namespace model
} // namespace smtk
