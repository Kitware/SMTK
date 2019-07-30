//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extension_qtDescriptivePhraseModel_h
#define __smtk_extension_qtDescriptivePhraseModel_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h" // For EXPORT macro.
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/PhraseModelObserver.h"

#include <QAbstractItemModel>
#include <QIcon>

#include <map>

namespace smtk
{
namespace extension
{

/**\brief Adapt an smtk::view::PhraseModel instance into a hierarchical Qt model.
  *
  * By calling setPhraseModel(), you identify the toplevel
  * description you wish to present; it may cover any set
  * of model, mesh, and attribute resources.
  *
  * The smtk::view::PhraseModel uses an smtk::view::SubphraseGenerator
  * instance to create a hierarchy of smtk::view::DescriptivePhrase
  * instances which portray the components in a set of resources.
  */
class SMTKQTEXT_EXPORT qtDescriptivePhraseModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  qtDescriptivePhraseModel(QObject* parent = 0);
  virtual ~qtDescriptivePhraseModel();
  // Set and get default entity color
  static void setDefaultPhraseColor(const std::string& entityType, const QColor& color)
  {
    s_defaultColors[entityType] = color;
  }
  static QColor defaultPhraseColor(const std::string& entityType);

  /// Set and get the icons to be used when visibility is to be drawn
  std::string visibleIconURL() const { return m_visibleIconURL; }
  std::string invisibleIconURL() const { return m_invisibleIconURL; }
  void setVisibleIconURL(const std::string& url) { m_visibleIconURL = url; }
  void setInvisibleIconURL(const std::string& url) { m_invisibleIconURL = url; }

  /// Enumeration of model-specific data roles.
  enum DataRoles
  {
    TitleTextRole = Qt::UserRole + 100,    //!< Phrase title (usu. user-editable component name)
    SubtitleTextRole = Qt::UserRole + 101, //!< Phrase subtitle (usu. type of phrase)
    PhraseIconRole = Qt::UserRole + 102,   //!< Phrase type icon
    PhraseInvertedIconRole = Qt::UserRole + 103, //!< Phrase type icon (color inverted)
    PhraseColorRole = Qt::UserRole + 104,        //!< Phrase-specific color (e.g., component color)
    PhraseVisibilityRole = Qt::UserRole + 105,   //!< Visibility of phrase's subject
    PhraseCleanRole = Qt::UserRole + 106,        //!< Is resource clean (0), dirty (1), or N/A (-1)?
    PhraseLockRole = Qt::UserRole + 107,         //!< Is resource free (0) or locked (1)?
    ModelActiveRole = Qt::UserRole + 108,        //!< Is resource the active resource?
    TitleTextMutableRole = Qt::UserRole + 109,   //!< Is the title editable?
    ColorMutableRole = Qt::UserRole + 110,       //!< Is the subject's color editable?
    PhrasePtrRole = Qt::UserRole + 111           //!< Grab the whole descriptive phrase!
  };

  void setPhraseModel(smtk::view::PhraseModelPtr model);

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

  // void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  void setDeleteOnRemoval(bool del) { m_deleteOnRemoval = del; }

  static QIcon lookupIconForPhraseFlags(view::DescriptivePhrasePtr item, QColor color);

  view::DescriptivePhrasePtr getItem(const QModelIndex& idx) const;
  QModelIndex indexFromPath(const std::vector<int>& path) const;

  template <typename T, typename C>
  bool foreach_phrase(
    T& visitor, C& collector, const QModelIndex& top = QModelIndex(), bool onlyBuilt = true);
  template <typename T, typename C>
  bool foreach_phrase(
    T& visitor, C& collector, const QModelIndex& top = QModelIndex(), bool onlyBuilt = true) const;

  void rebuildSubphrases(const QModelIndex& qidx);

  Qt::DropActions supportedDropActions() const override;

public slots:
  virtual void toggleVisibility(const QModelIndex& idx);
  virtual void editColor(const QModelIndex& idx);

signals:
  void phraseTitleChanged(const QModelIndex&);

protected:
  void updateObserver(smtk::view::DescriptivePhrasePtr phrase, smtk::view::PhraseModelEvent event,
    const std::vector<int>& src, const std::vector<int>& dst, const std::vector<int>& range);

  smtk::view::PhraseModelPtr m_model;
  smtk::view::PhraseModelObservers::Key m_modelObserver;
  bool m_deleteOnRemoval; // remove UUIDs from mesh when they are removed from the list?
  std::string m_visibleIconURL;
  std::string m_invisibleIconURL;
  static std::map<std::string, QColor> s_defaultColors;
  class Internal;
  Internal* P;
};

/**\brief Iterate over all expanded entries in the tree.
  *
  * Note that if you call this method, you must include "smtk/extension/qt/qtTypeDeclarations.h".
  */
template <typename T, typename C>
bool qtDescriptivePhraseModel::foreach_phrase(
  T& visitor, C& collector, const QModelIndex& top, bool onlyBuilt)
{
  // visit parent, then children if we aren't told to terminate:
  if (!visitor(this, top, collector))
  {
    view::DescriptivePhrasePtr phrase =
      top.data(PhrasePtrRole).value<smtk::view::DescriptivePhrasePtr>();
    // Do not descend if top's corresponding phrase would have to invoke
    // the subphrase generator to obtain the list of children... some phrases
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
bool qtDescriptivePhraseModel::foreach_phrase(
  T& visitor, C& collector, const QModelIndex& top, bool onlyBuilt) const
{
  // visit parent, then children if we aren't told to terminate:
  if (!visitor(this, top, collector))
  {
    view::DescriptivePhrasePtr phrase =
      top.data(PhrasePtrRole).value<smtk::view::DescriptivePhrasePtr>();
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

#endif // __smtk_extension_qtDescriptivePhraseModel_h
