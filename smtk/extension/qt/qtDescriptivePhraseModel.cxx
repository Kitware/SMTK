//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/extension/qt/qtTypeDeclarations.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/Resource.h"
#include "smtk/model/StringData.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/Resource.h"

#include <QBitmap>

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

static void initIconResource()
{
  if (resourceCounter <= 0)
  {
    Q_INIT_RESOURCE(qtDescriptivePhraseModelIcons);
  }
  ++resourceCounter;
}

static void cleanupIconResource()
{
  if (--resourceCounter == 0)
  {
    Q_CLEANUP_RESOURCE(qtDescriptivePhraseModelIcons);
  }
}

using namespace smtk::model;

namespace smtk
{
namespace extension
{
std::map<std::string, QColor> qtDescriptivePhraseModel::s_defaultColors = {};

/// Private storage for qtDescriptivePhraseModel.
class qtDescriptivePhraseModel::Internal
{
public:
  /**\brief Store a map of weak pointers to phrases by their phrase IDs.
    *
    * We hold a strong pointer to the phrase-model in qtDescriptivePhraseModel::m_model.
    * The phrase-model owns the root of the descriptive phrase hierarchy.
    * This map is a reverse lookup of pointers to subphrases by integer handles
    * that Qt can associate with QModelIndex entries.
    *
    * This all exists because Qt is lame and cannot associate shared pointers
    * with QModelIndex entries.
    */
  std::map<unsigned int, view::WeakDescriptivePhrasePtr> ptrs;
};

qtDescriptivePhraseModel::qtDescriptivePhraseModel(QObject* owner)
  : QAbstractItemModel(owner)
  , m_modelObserver()
  , m_visibleIconURL(":/icons/display/eyeball.png")
  , m_invisibleIconURL(":/icons/display/eyeballClosed.png")
{
  m_deleteOnRemoval = true;
  this->P = new Internal;
  initIconResource();
}

qtDescriptivePhraseModel::~qtDescriptivePhraseModel()
{
  cleanupIconResource();
  delete this->P;
}

QColor qtDescriptivePhraseModel::defaultPhraseColor(const std::string& entityType)
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

void qtDescriptivePhraseModel::setPhraseModel(smtk::view::PhraseModelPtr model)
{
  // Get rid of old phrases
  if (m_model)
  {
    m_model->observers().erase(m_modelObserver);
    if (!m_model->root()->subphrases().empty())
    {
      // Provide an invalid parent since you want to clear all
      this->beginRemoveRows(
        QModelIndex(), 0, static_cast<int>(m_model->root()->subphrases().size()));
      m_model = smtk::view::PhraseModelPtr();
      this->endRemoveRows();
    }
  }
  m_model = model;
  if (m_model)
  {
    // Observe all changes to the model, which includes calling the observer on all
    // existing top-level phrases.
    m_modelObserver = m_model->observers().insert(
      [this](smtk::view::DescriptivePhrasePtr phrase, smtk::view::PhraseModelEvent event,
        const std::vector<int>& src, const std::vector<int>& dst,
        const std::vector<int>& range) { this->updateObserver(phrase, event, src, dst, range); },
      0, true);
  }
}

QModelIndex qtDescriptivePhraseModel::index(int row, int column, const QModelIndex& owner) const
{
  if (!m_model || m_model->root()->subphrases().empty())
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
  // std::string entName = ownerPhrase->relatedComponent().name();
  // std::cout << "Owner index for: " << entName << std::endl;
  view::DescriptivePhrases& subphrases(ownerPhrase->subphrases());
  if (row >= 0 && row < static_cast<int>(subphrases.size()))
  {
    //std::cout << "index(_"  << ownerPhrase->title() << "_, " << row << ") = " << subphrases[row]->title() << "\n";
    //std::cout << "index(_"  << ownerPhrase->phraseId() << "_, " << row << ") = " << subphrases[row]->phraseId() << ", " << subphrases[row]->title() << "\n";

    view::DescriptivePhrasePtr entry = subphrases[row];
    if (entry)
    {
      this->P->ptrs[entry->phraseId()] = entry;
      return this->createIndex(row, column, entry->phraseId());
    }
  }

  return QModelIndex();
}

QModelIndex qtDescriptivePhraseModel::parent(const QModelIndex& child) const
{
  if (!child.isValid())
  {
    return QModelIndex();
  }

  view::DescriptivePhrasePtr childPhrase = this->getItem(child);
  view::DescriptivePhrasePtr parentPhrase = childPhrase->parent();
  if (parentPhrase == m_model->root())
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
bool qtDescriptivePhraseModel::hasChildren(const QModelIndex& owner) const
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
  return (m_model ? (m_model->root()->subphrases().empty() ? false : true) : false);
}

/// The number of rows in the table "underneath" \a owner.
int qtDescriptivePhraseModel::rowCount(const QModelIndex& owner) const
{
  view::DescriptivePhrasePtr ownerPhrase = this->getItem(owner);
  return static_cast<int>(ownerPhrase->subphrases().size());
}

/// Return something to display in the table header.
QVariant qtDescriptivePhraseModel::headerData(
  int section, Qt::Orientation orientation, int role) const
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
        return "Resource";
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
QVariant qtDescriptivePhraseModel::data(const QModelIndex& idx, int role) const
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
      if (role == PhrasePtrRole)
      {
        QVariant result;
        result.setValue(item);
        return result;
      }
      else if (role == TitleTextRole || role == Qt::DisplayRole)
      {
        return QVariant(item->title().c_str());
      }
      else if (role == SubtitleTextRole)
      {
        return QVariant(item->subtitle().c_str());
      }
      else if (role == PhraseIconRole || role == PhraseInvertedIconRole)
      {
        // get luminance
        QColor color;
        FloatList rgba = item->relatedColor();
        if (rgba.size() >= 4 && rgba[3] < 0)
        {
          if (role == PhraseIconRole)
          {
            // return white by default
            color = QColor(255, 255, 255, 255);
          }
          else
          {
            // return black by default
            color = QColor(0, 0, 0, 255);
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
        return QVariant(this->lookupIconForPhraseFlags(item, color));
      }
      else if (role == PhraseVisibilityRole)
      {
        bool drawVisibility = item->displayVisibility();
        if (!drawVisibility)
        {
          return QVariant();
        }
        // by default, everything should be visible
        bool visible = item->relatedVisibility();
        if (visible)
          return QVariant(QIcon(m_visibleIconURL.c_str()));
        else
          return QVariant(QIcon(m_invisibleIconURL.c_str()));
      }
      else if (role == PhraseColorRole &&
        (item->relatedComponent() || item->phraseType() == smtk::view::DescriptivePhraseType::LIST))
      {
        QColor color;
        FloatList rgba = item->relatedColor();
        bool gotColor = false;
        if (rgba.size() >= 4 && rgba[3] < 0)
        {
          auto modelComp = dynamic_pointer_cast<smtk::model::Entity>(item->relatedComponent());
          if (modelComp)
          {
            smtk::model::EntityRef ent(modelComp);
            if (ent.isFace())
            {
              color = qtDescriptivePhraseModel::defaultPhraseColor("Face");
              gotColor = true;
            }
            else if (ent.isEdge())
            {
              color = qtDescriptivePhraseModel::defaultPhraseColor("Edge");
              gotColor = true;
            }
            else if (ent.isVertex())
            {
              color = qtDescriptivePhraseModel::defaultPhraseColor("Vertex");
              gotColor = true;
            }
          }
          if (!gotColor)
          {
            // return an invalid color by default
            color = QColor();
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
      else if (role == PhraseCleanRole)
      {
        int clean = -1;
        auto rsrc = item->relatedResource();
        auto comp = item->relatedComponent();
        if (rsrc && !comp)
        {
          clean = rsrc->clean() ? 1 : 0; // Clean == 1, dirty == 0, N/A == -1
        }
        else
        {
          // TODO: Remove this eventually; we should only show file-level clean/dirty.
          auto modelComp = dynamic_pointer_cast<smtk::model::Entity>(comp);
          if (modelComp)
          {
            auto ent = modelComp->referenceAs<smtk::model::EntityRef>();
            clean = static_cast<int>(
              ent.hasIntegerProperty("clean") ? ent.integerProperty("clean")[0] : -1);
          }
        }
        return QVariant(clean);
      }
      else if (role == PhraseLockRole)
      {
        int locked = 0;
        auto rsrc = item->relatedResource();
        auto comp = item->relatedComponent();
        if (rsrc && !comp)
        {
          // Locked == 1, Free == 0
          locked = rsrc->locked() == smtk::resource::LockType::Write ? 1 : 0;
        }
        return QVariant(locked);
      }
      else if (role == ModelActiveRole)
      {
        // used to bold the active resource's title
        return QVariant(false);
      }
      else if (role == TitleTextMutableRole)
      {
        return QVariant(item->isTitleMutable());
      }
      else if (role == ColorMutableRole)
      {
        return QVariant(item->isRelatedColorMutable());
      }
    }
  }
  return QVariant();
}

/// Remove rows from the model.
bool qtDescriptivePhraseModel::removeRows(int position, int rows, const QModelIndex& parentIdx)
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

bool qtDescriptivePhraseModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
  bool didChange = false;
  view::DescriptivePhrasePtr phrase;
  if (idx.isValid() && (phrase = this->getItem(idx)))
  {
    if (role == TitleTextRole && phrase->isTitleMutable())
    {
      std::string sval = value.value<QString>().toStdString();
      didChange = phrase->setTitle(sval);
    }
    else if (role == SubtitleTextRole && phrase->isSubtitleMutable())
    {
      std::string sval = value.value<QString>().toStdString();
      didChange = phrase->setSubtitle(sval);
    }
    else if (role == PhraseColorRole && phrase->isRelatedColorMutable() &&
      phrase->relatedComponent())
    {
      QColor color = value.value<QColor>();
      FloatList rgba(4);
      rgba[0] = color.redF();
      rgba[1] = color.greenF();
      rgba[2] = color.blueF();
      rgba[3] = color.alphaF();
      didChange = phrase->setRelatedColor(rgba);
    }
    else if (role == PhraseVisibilityRole && phrase->relatedComponent())
    {
      /*
      int vis = value.toInt();
      phrase->relatedComponent().setVisible(vis > 0 ? true : false);
      didChange = true;
      */
      didChange = false;
    }
  }
  return didChange;
}

/// Provide meta-information about a model entry.
Qt::ItemFlags qtDescriptivePhraseModel::flags(const QModelIndex& idx) const
{
  if (!idx.isValid())
    return Qt::ItemIsEnabled;

  // TODO: Check to make sure column is not "information-only".
  //       We don't want to allow people to randomly edit an enum string.
  Qt::ItemFlags itemFlags =
    QAbstractItemModel::flags(idx) | Qt::ItemIsEditable | Qt::ItemIsSelectable;
  view::DescriptivePhrasePtr dp = this->getItem(idx);
  /* TODO:
  if (dp && dp->relatedComponent().isGroup())
    itemFlags = itemFlags | Qt::ItemIsDropEnabled;
    */
  return itemFlags;
}

QIcon qtDescriptivePhraseModel::lookupIconForPhraseFlags(
  view::DescriptivePhrasePtr item, QColor color)
{
  // REFERENCE: https://stackoverflow.com/questions/3942878/how-to-decide-font-color-in-white-or-black-depending-on-background-color
  double lightness = 0.2126 * color.redF() + 0.7152 * color.greenF() + 0.0722 * color.blueF();
  auto modelComp = dynamic_pointer_cast<smtk::model::Entity>(item->relatedComponent());
  auto attComp = dynamic_pointer_cast<smtk::attribute::Attribute>(item->relatedComponent());
  auto meshComp = dynamic_pointer_cast<smtk::mesh::Component>(item->relatedComponent());
  std::ostringstream resourceName;
  resourceName << ":/icons/entityTypes/";
  if (item->phraseType() == smtk::view::DescriptivePhraseType::COMPONENT_LIST)
  {
    resourceName << "list";
  }
  else if (modelComp)
  {
    smtk::model::BitFlags flags = modelComp->entityFlags();
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
        resourceName << "model"; //  every session has a model
        break;
      default:
        // Sometimes group entities have other bits set.
        // Otherwise... it's garbage-in, garbage-out
        resourceName << (flags & GROUP_ENTITY ? "group" : "invalid");
        dimBits = false;
        break;
    }

    if (dimBits && ((flags & ENTITY_MASK) == CELL_ENTITY))
    {
      resourceName << "_" << std::setbase(16) << std::fixed << std::setw(2) << std::setfill('0')
                   << (flags & ANY_DIMENSION);
    }
  }
  else if (attComp)
  {
    resourceName << "attribute";
  }
  else if (meshComp)
  {
    resourceName << "meshset";
  }
  else if (item->relatedComponent() == nullptr)
  {
    auto relatedResource = item->relatedResource();
    // Lets check the resource
    if (dynamic_pointer_cast<smtk::model::Resource>(relatedResource) != nullptr)
    {
      resourceName << "modelResource";
    }
    else if (dynamic_pointer_cast<smtk::attribute::Resource>(relatedResource) != nullptr)
    {
      resourceName << "attributeResource";
    }
    else if (dynamic_pointer_cast<smtk::mesh::Resource>(relatedResource))
    {
      resourceName << "meshResource";
    }
    else
    {
      resourceName << "invalid";
    }
  }
  else
  {
    resourceName << "invalid";
  }

  // lightness controls black/white icon
  if (lightness >= 0.179)
  {
    resourceName << "_b";
  }
  else // if (lightness < 0.179)
  {
    resourceName << "_w";
  }

  resourceName << ".png";

  QFile rsrc(resourceName.str().c_str());
  if (!rsrc.exists())
  { // FIXME: Replace with the path of a "generic entity" or "invalid" icon.
    if (lightness >= 0.179)
    {
      return QIcon(":/icons/entityTypes/generic_entity_b.png");
    }
    else
    {
      return QIcon(":/icons/entityTypes/generic_entity_w.png");
    }
  }
  return QIcon(resourceName.str().c_str());
}

/**\brief Sort the UUIDs being displayed using the given ordered container.
  *
  * The ordered container's comparator is used to insertion-sort the UUIDs
  * displayed. Then, the \a order is used to either forward- or reverse-iterator
  * over the container to obtain a new ordering for the UUIDs.
template<typename T>
void qtDescriptivePhraseModel::sortDataWithContainer(T& sorter, Qt::SortOrder order)
{
  smtk::common::UUIDArray::iterator ai;
  // Insertion into the set sorts the UUIDs.
  for (ai = m_phrases.begin(); ai != m_phrases.end(); ++ai)
    {
    sorter.insert(*ai);
    }
  // Now we reset m_phrases and m_reverse and recreate based on the sorter's order.
  m_phrases.clear();
  m_reverse.clear();
  int i;
  if (order == Qt::AscendingOrder)
    {
    typename T::iterator si;
    for (i = 0, si = sorter.begin(); si != sorter.end(); ++si, ++i)
      {
      m_phrases.push_back(*si);
      m_reverse[*si] = i;
      / *
      std::cout << i << "  " << *si << "  " <<
        (m_manager->hasStringProperty(*si, "name") ?
         m_manager->stringProperty(*si, "name")[0].c_str() : "--") << "\n";
         * /
      }
    }
  else
    {
    typename T::reverse_iterator si;
    for (i = 0, si = sorter.rbegin(); si != sorter.rend(); ++si, ++i)
      {
      m_phrases.push_back(*si);
      m_reverse[*si] = i;
      / *
      std::cout << i << "  " << *si << "  " <<
        (m_manager->hasStringProperty(*si, "name") ?
         m_manager->stringProperty(*si, "name")[0].c_str() : "--") << "\n";
         * /
      }
    }
}
  */

/// A utility function to retrieve the view::DescriptivePhrasePtr associated with a model index.
view::DescriptivePhrasePtr qtDescriptivePhraseModel::getItem(const QModelIndex& idx) const
{
  if (idx.isValid())
  {
    unsigned int phraseIdx = idx.internalId();
    std::map<unsigned int, view::WeakDescriptivePhrasePtr>::iterator it;
    if ((it = this->P->ptrs.find(phraseIdx)) == this->P->ptrs.end())
    {
      //std::cout << "  Missing index " << phraseIdx << "\n";
      // std::cout.flush();
      return m_model->root();
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
  return m_model->root();
}

QModelIndex qtDescriptivePhraseModel::indexFromPath(const std::vector<int>& path) const
{
  QModelIndex result;
  for (auto entry : path)
  {
    result = this->index(entry, 0, result);
  }
  return result;
}

void qtDescriptivePhraseModel::rebuildSubphrases(const QModelIndex& qidx)
{
  int nrows = this->rowCount(qidx);
  view::DescriptivePhrasePtr phrase = this->getItem(qidx);

  this->removeRows(0, nrows, qidx);
  auto root = m_model ? m_model->root() : nullptr;
  if (phrase && phrase != root)
    phrase->markDirty(true);

  nrows = phrase ? static_cast<int>(phrase->subphrases().size()) : 0;
  this->beginInsertRows(qidx, 0, nrows);
  this->endInsertRows();
  emit dataChanged(qidx, qidx);
}

Qt::DropActions qtDescriptivePhraseModel::supportedDropActions() const
{
  return Qt::CopyAction;
}

void qtDescriptivePhraseModel::toggleVisibility(const QModelIndex& idx)
{
  auto phrase = idx.data(PhrasePtrRole).value<smtk::view::DescriptivePhrase::Ptr>();
  if (!phrase)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Asked to toggle visibility for item unrelated to SMTK.");
    return;
  }
  if (!phrase->setRelatedVisibility(!phrase->relatedVisibility()))
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Could not toggle visibility of \"" << phrase->title() << "\"");
  }
}

void qtDescriptivePhraseModel::editColor(const QModelIndex& idx)
{
  auto phrase = idx.data(PhrasePtrRole).value<smtk::view::DescriptivePhrase::Ptr>();
  if (!phrase)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Asked to toggle visibility for item unrelated to SMTK.");
    return;
  }
  smtkWarningMacro(smtk::io::Logger::instance(), "Color editing not implemented yet.");
}

void qtDescriptivePhraseModel::updateObserver(smtk::view::DescriptivePhrasePtr phrase,
  smtk::view::PhraseModelEvent event, const std::vector<int>& src, const std::vector<int>& dst,
  const std::vector<int>& range)
{
  (void)phrase;

  using smtk::view::PhraseModelEvent;

  switch (event)
  {
    case PhraseModelEvent::ABOUT_TO_INSERT:
      emit this->beginInsertRows(this->indexFromPath(src), range[0], range[1]);
      break;
    case PhraseModelEvent::INSERT_FINISHED:
      emit this->endInsertRows();
      break;
    case PhraseModelEvent::ABOUT_TO_REMOVE:
      emit this->beginRemoveRows(this->indexFromPath(src), range[0], range[1]);
      break;
    case PhraseModelEvent::REMOVE_FINISHED:
      emit this->endRemoveRows();
      break;
    case PhraseModelEvent::ABOUT_TO_MOVE:
      emit this->beginMoveRows(
        this->indexFromPath(src), range[0], range[1], this->indexFromPath(dst), range[2]);
      break;
    case PhraseModelEvent::MOVE_FINISHED:
      emit this->endMoveRows();
      break;
    case PhraseModelEvent::PHRASE_MODIFIED:
      emit this->dataChanged(this->indexFromPath(src), this->indexFromPath(dst));
      break;
  }
}

} // namespace extension
} // namespace smtk
