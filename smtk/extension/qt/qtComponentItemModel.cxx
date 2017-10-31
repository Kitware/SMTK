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

#include "smtk/extension/qt/qtActiveObjects.h"

#include "smtk/view/SubphraseGenerator.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/Manager.h"
#include "smtk/model/MeshPhrase.h"
#include "smtk/model/StringData.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QVariant>

#include <deque>
#include <iomanip>
#include <map>
#include <sstream>

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

using namespace smtk::model;

namespace smtk
{
namespace extension
{
std::map<std::string, QColor> QComponentItemModel::s_defaultColors = {};

/// Private storage for QComponentItemModel.
class QComponentItemModel::Internal
{
public:
  /**\brief Store a map of weak pointers to phrases by their phrase IDs.
    *
    * We hold a strong pointer to the root phrase in QComponentItemModel::m_root.
    * This map is a reverse lookup of pointers to subphrases by integer handles
    * that Qt can associate with QModelIndex entries.
    *
    * This all exists because Qt is lame and cannot associate shared pointers
    * with QModelIndex entries.
    */
  std::map<unsigned int, view::WeakDescriptivePhrasePtr> ptrs;
};

// A visitor functor called by foreach_phrase() to let the view know when to redraw data.
static bool UpdateSubphrases(
  QComponentItemModel* qmodel, const QModelIndex& qidx, const EntityRef& ent)
{
  view::DescriptivePhrasePtr phrase = qmodel->getItem(qidx);
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
static int entityModified(ManagerEventType, const smtk::model::EntityRef& ent,
  const smtk::model::EntityRef&, void* callData)
{
  QComponentItemModel* qmodel = static_cast<QComponentItemModel*>(callData);
  if (!qmodel)
    return 1;

  // Find EntityPhrase instances under the root node whose relatedEntity
  // is \a ent and rerun the subphrase generator.
  // This should in turn invoke callbacks on the QComponentItemModel
  // to handle insertions/deletions.
  return qmodel->foreach_phrase(UpdateSubphrases, ent) ? -1 : 0;
}

QComponentItemModel::QComponentItemModel(QObject* owner)
  : QAbstractItemModel(owner)
{
  this->m_deleteOnRemoval = true;
  this->P = new Internal;
  initIconResource();
}

QComponentItemModel::~QComponentItemModel()
{
  cleanupIconResource();
  delete this->P;
}

QColor QComponentItemModel::defaultEntityColor(const std::string& entityType)
{
  if (s_defaultColors.find(entityType) == s_defaultColors.end())
  {
    return QColor();
  }
  else
  {
    return s_defaultColors[entityType];
  }
}
void QComponentItemModel::clear()
{
  if (this->m_root && !this->m_root->subphrases().empty())
  {
    // provide an invalid parent since you want to clear all
    this->beginRemoveRows(QModelIndex(), 0, static_cast<int>(this->m_root->subphrases().size()));
    this->m_root = view::DescriptivePhrasePtr();
    this->endRemoveRows();
  }
}

QModelIndex QComponentItemModel::index(int row, int column, const QModelIndex& owner) const
{
  if (!this->m_root || this->m_root->subphrases().empty())
    return QModelIndex();

  if (owner.isValid() && owner.column() != 0)
    return QModelIndex();

  int rows = this->rowCount(owner);
  int columns = this->columnCount(owner);
  if (row < 0 || row >= rows || column < 0 || column >= columns)
  {
    return QModelIndex();
  }

  view::DescriptivePhrasePtr ownerPhrase = this->getItem(owner);
  std::string entName = ownerPhrase->relatedEntity().name();
  //  std::cout << "Owner index for: " << entName << std::endl;
  view::DescriptivePhrases& subphrases(ownerPhrase->subphrases());
  if (row >= 0 && row < static_cast<int>(subphrases.size()))
  {
    //std::cout << "index(_"  << ownerPhrase->title() << "_, " << row << ") = " << subphrases[row]->title() << "\n";
    //std::cout << "index(_"  << ownerPhrase->phraseId() << "_, " << row << ") = " << subphrases[row]->phraseId() << ", " << subphrases[row]->title() << "\n";

    view::DescriptivePhrasePtr entry = subphrases[row];
    this->P->ptrs[entry->phraseId()] = entry;
    return this->createIndex(row, column, entry->phraseId());
  }

  return QModelIndex();
}

QModelIndex QComponentItemModel::parent(const QModelIndex& child) const
{
  if (!child.isValid())
  {
    return QModelIndex();
  }

  view::DescriptivePhrasePtr childPhrase = this->getItem(child);
  view::DescriptivePhrasePtr parentPhrase = childPhrase->parent();
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
bool QComponentItemModel::hasChildren(const QModelIndex& owner) const
{
  // According to various Qt mailing-list posts, we might
  // speed things up by always returning true here.
  if (owner.isValid())
  {
    view::DescriptivePhrasePtr phrase = this->getItem(owner);
    if (phrase)
    { // Return whether the parent has subphrases.
      return phrase->subphrases().empty() ? false : true;
    }
  }
  // Return whether the toplevel m_phrases list is empty.
  return (this->m_root ? (this->m_root->subphrases().empty() ? false : true) : false);
}

/// The number of rows in the table "underneath" \a owner.
int QComponentItemModel::rowCount(const QModelIndex& owner) const
{
  view::DescriptivePhrasePtr ownerPhrase = this->getItem(owner);
  return static_cast<int>(ownerPhrase->subphrases().size());
}

/// Return something to display in the table header.
QVariant QComponentItemModel::headerData(int section, Qt::Orientation orientation, int role) const
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

/// Relate information, by its \a role, from a \a DescriptivePhrasePtr to the Qt model.
QVariant QComponentItemModel::data(const QModelIndex& idx, int role) const
{
  if (idx.isValid())
  {
    // A valid index may have a *parent* that is:
    // + invalid (in which case we use row() as an offset into m_phrases to obtain data)
    // + valid (in which case it points to a DescriptivePhrasePtr instance and
    //   row() is an offset into the subphrases)

    view::DescriptivePhrasePtr item = this->getItem(idx);
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
        // get luminance
        QColor color;
        FloatList rgba = item->relatedColor();
        if (rgba.size() >= 4 && rgba[3] < 0)
        {
          // make it an invalid color
          color = QColor(255, 255, 255, 0);
        }
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
        return QVariant(this->lookupIconForEntityFlags(item, color));
      }
      else if (role == EntityVisibilityRole)
      {
        // by default, everything should be visible
        bool visible = true;

        if (item->phraseType() == MESH_SUMMARY)
        {
          MeshPhrasePtr mphrase = smtk::dynamic_pointer_cast<MeshPhrase>(item);
          smtk::mesh::MeshSet meshkey;
          smtk::mesh::CollectionPtr c;
          if (!mphrase->relatedMesh().is_empty())
          {
            meshkey = mphrase->relatedMesh();
            c = meshkey.collection();
          }
          else
          {
            c = mphrase->relatedMeshCollection();
            meshkey = c->meshes();
          }
          if (c && !meshkey.is_empty() && c->hasIntegerProperty(meshkey, "visible"))
          {
            const IntegerList& prop(c->integerProperty(meshkey, "visible"));
            if (!prop.empty())
            {
              visible = (prop[0] != 0);
            }
            // check children of MESH_SUMMARY (Ex. mesh faces)
            int childrenVisibile = -1;
            bool reachEnd = true;
            if (!mphrase->relatedMesh().is_empty())
            {
              for (std::size_t i = 0; i < mphrase->relatedMesh().size(); ++i)
              {
                const IntegerList& propSub(
                  c->integerProperty(mphrase->relatedMesh().subset(i), "visible"));
                if (!propSub.empty())
                {
                  if (childrenVisibile == -1)
                  { // store first child visibility
                    childrenVisibile = propSub[0];
                  }
                  else if (childrenVisibile != propSub[0])
                  { // inconsistant visibility. Do nothing.
                    reachEnd = false;
                    break;
                  }
                }
              }
            }
            // update visibility if children's are consistant
            if (reachEnd && childrenVisibile != -1)
            {
              visible = childrenVisibile;
            }
          }
        }
        else if (item->phraseType() == ENTITY_LIST)
        {
          EntityListPhrasePtr lphrase = smtk::dynamic_pointer_cast<EntityListPhrase>(item);
          // if all its children is off, then show as off
          bool hasVisibleChild = false;
          EntityRefArray::const_iterator it;
          for (it = lphrase->relatedEntities().begin();
               it != lphrase->relatedEntities().end() && !hasVisibleChild; ++it)
          {
            if (it->hasVisibility())
            {
              const IntegerList& prop(it->integerProperty("visible"));
              if (!prop.empty() && prop[0] != 0)
                hasVisibleChild = true;
            }
            else // default is visible
              hasVisibleChild = true;
          }
          visible = hasVisibleChild;
        }
        else if (item->relatedEntity().isValid() && item->relatedEntity().hasVisibility())
        {
          const IntegerList& prop(item->relatedEntity().integerProperty("visible"));
          if (!prop.empty())
            visible = (prop[0] != 0);
        }

        if (visible)
          return QVariant(QIcon(":/icons/display/eyeball.png"));
        else
          return QVariant(QIcon(":/icons/display/eyeballClosed.png"));
      }
      else if (role == EntityColorRole &&
        (item->phraseType() == MESH_SUMMARY || item->relatedEntity().isValid() ||
                 item->phraseType() == ENTITY_LIST))
      {
        QColor color;
        FloatList rgba = item->relatedColor();
        if (rgba.size() >= 4 && rgba[3] < 0)
        {
          if (item->relatedEntity().isFace())
          {
            color = QComponentItemModel::defaultEntityColor("Face");
          }
          else if (item->relatedEntity().isEdge())
          {
            color = QComponentItemModel::defaultEntityColor("Edge");
          }
          else if (item->relatedEntity().isVertex())
          {
            color = QComponentItemModel::defaultEntityColor("Vertex");
          }
          else
          {
            // Assign an invalid color
            color = QColor(255, 255, 255, 0);
          }
        }
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
      else if (role == EntityCleanRole)
      {
        EntityRef ent = item->relatedEntity();
        return QVariant(
          static_cast<int>(ent.hasIntegerProperty("clean") ? ent.integerProperty("clean")[0] : -1));
      }
      else if (role == ModelActiveRole)
      {
        // used to bold the active model's title
        return (item->relatedEntity().entity() ==
                 qtActiveObjects::instance().activeModel().entity())
          ? QVariant(true)
          : QVariant(false);
      }
    }
  }
  return QVariant();
}

/// Remove rows from the model.
bool QComponentItemModel::removeRows(int position, int rows, const QModelIndex& parentIdx)
{
  if (rows <= 0 || position < 0)
    return false;

  this->beginRemoveRows(parentIdx, position, position + rows - 1);
  view::DescriptivePhrasePtr phrase = this->getItem(parentIdx);
  if (phrase)
  {
    phrase->subphrases().erase(
      phrase->subphrases().begin() + position, phrase->subphrases().begin() + position + rows);
  }
  this->endRemoveRows();
  return true;
}

bool QComponentItemModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
  bool didChange = false;
  view::DescriptivePhrasePtr phrase;
  if (idx.isValid() && (phrase = this->getItem(idx)))
  {
    if (role == TitleTextRole && phrase->isTitleMutable())
    {
      std::string sval = value.value<QString>().toStdString();
      didChange = phrase->setTitle(sval);

      // sort the subphrase after change
      phrase->markDirty();
      this->rebuildSubphrases(idx.parent());
      // if data did get changed, we need to emit the signal
      if (didChange)
        emit this->phraseTitleChanged(idx);
    }
    else if (role == SubtitleTextRole && phrase->isSubtitleMutable())
    {
      std::string sval = value.value<QString>().toStdString();
      didChange = phrase->setSubtitle(sval);
    }
    else if (role == EntityColorRole && phrase->isRelatedColorMutable() &&
      phrase->relatedEntity().isValid())
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
void QComponentItemModel::sort(int column, Qt::SortOrder order)
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
Qt::ItemFlags QComponentItemModel::flags(const QModelIndex& idx) const
{
  if (!idx.isValid())
    return Qt::ItemIsEnabled;

  // TODO: Check to make sure column is not "information-only".
  //       We don't want to allow people to randomly edit an enum string.
  Qt::ItemFlags itemFlags = QAbstractItemModel::flags(idx) | Qt::ItemIsEditable |
    Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
  view::DescriptivePhrasePtr dp = this->getItem(idx);
  if (dp && dp->relatedEntity().isGroup())
    itemFlags = itemFlags | Qt::ItemIsDropEnabled;
  return itemFlags;
}

static bool FindManager(
  const QComponentItemModel* qmodel, const QModelIndex& qidx, ManagerPtr& manager)
{
  view::DescriptivePhrasePtr phrase = qmodel->getItem(qidx);
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
  * Note that it is possible for a QComponentItemModel to present information
  * on entities from multiple Manager instances.
  * However, in this case, external updates to the selection must either be
  * made via EntityRef instances (which couple UUIDs with Manager instances) or
  * there will be breakage.
  */
smtk::model::ManagerPtr QComponentItemModel::manager() const
{
  ManagerPtr store;
  this->foreach_phrase(FindManager, store, QModelIndex(), false);
  return store;
}

QIcon QComponentItemModel::lookupIconForEntityFlags(view::DescriptivePhrasePtr item, QColor color)
{
  //REFERENCE: https://stackoverflow.com/questions/3942878/how-to-decide-font-color-in-white-or-black-depending-on-background-color
  double lightness = 0.2126 * color.redF() + 0.7152 * color.greenF() + 0.0722 * color.blueF();
  // phrase list(0) would use relatedBitFlags
  smtk::model::BitFlags flags =
    (item->phraseType()) ? item->relatedEntity().entityFlags() : item->relatedBitFlags();
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
    case AUX_GEOM_ENTITY:
      resourceName << "aux_geom";
      break;
    case SESSION:
      resourceName << "model"; //  every session has an model
      break;
    default:
      resourceName << "invalid";
      dimBits = false;
      break;
  }

  if (dimBits && ((flags & ENTITY_MASK) == CELL_ENTITY))
  {
    resourceName << "_" << std::setbase(16) << std::fixed << std::setw(2) << std::setfill('0')
                 << (flags & ANY_DIMENSION);
  }

  // lightness controls black/white ico
  if (lightness >= 0.179 && ((flags & ENTITY_MASK) == CELL_ENTITY))
  {
    resourceName << "_b";
  }
  else if (lightness < 0.179 && ((flags & ENTITY_MASK) == CELL_ENTITY))
  {
    resourceName << "_w";
  }

  resourceName << ".png";

  QFile rsrc(resourceName.str().c_str());
  if (!rsrc.exists())
  { // FIXME: Replace with the path of a "generic entity" or "invalid" icon.
    return QIcon(":/icons/entityTypes/generic_entity.png");
  }
  return QIcon(resourceName.str().c_str());
}

/**\brief Sort the UUIDs being displayed using the given ordered container.
  *
  * The ordered container's comparator is used to insertion-sort the UUIDs
  * displayed. Then, the \a order is used to either forward- or reverse-iterator
  * over the container to obtain a new ordering for the UUIDs.
template<typename T>
void QComponentItemModel::sortDataWithContainer(T& sorter, Qt::SortOrder order)
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

/// A utility function to retrieve the view::DescriptivePhrasePtr associated with a model index.
view::DescriptivePhrasePtr QComponentItemModel::getItem(const QModelIndex& idx) const
{
  if (idx.isValid())
  {
    unsigned int phraseIdx = idx.internalId();
    std::map<unsigned int, view::WeakDescriptivePhrasePtr>::iterator it;
    if ((it = this->P->ptrs.find(phraseIdx)) == this->P->ptrs.end())
    {
      //std::cout << "  Missing index " << phraseIdx << "\n";
      std::cout.flush();
      return this->m_root;
    }
    view::WeakDescriptivePhrasePtr weakPhrase = it->second;
    view::DescriptivePhrasePtr phrase;
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

void QComponentItemModel::rebuildSubphrases(const QModelIndex& qidx)
{
  int nrows = this->rowCount(qidx);
  view::DescriptivePhrasePtr phrase = this->getItem(qidx);

  this->removeRows(0, nrows, qidx);
  if (phrase)
    phrase->markDirty(true);

  nrows = phrase ? static_cast<int>(phrase->subphrases().size()) : 0;
  this->beginInsertRows(qidx, 0, nrows);
  this->endInsertRows();
  emit dataChanged(qidx, qidx);
}

void QComponentItemModel::updateWithOperatorResult(const OperatorResult& result)
{
}

Qt::DropActions QComponentItemModel::supportedDropActions() const
{
  return Qt::CopyAction;
}

} // namespace extension
} // namespace smtk
