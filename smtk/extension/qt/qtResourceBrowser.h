//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtResourceBrowser_h
#define smtk_extension_qtResourceBrowser_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/PublicPointerDefs.h"

#include <QWidget>

class QAbstractItemModel;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace extension
{

class qtDescriptivePhraseModel;

/**\brief A panel that displays SMTK resources available to the application/user.
  *
  * This contains Qt widget that displays a tree or list view holding an SMTK
  * descriptive phrase model.
  *
  * Its Information should be initialized with json/xml that contains:
  * (1) an smtk::view::PhraseModel that you have configured,
  * (2) the string name registered to a QAbstractItemView subclass constructor,
  * (3) a QAbstactItemModel implementing qtDescriptivePhraseModel model index queries, and
  * (4) a parent QWidget.
  *
  * This QAbstractItemModel class should either be a
  * qtDescriptivePhraseModel or QAbstractProxyModel whose source is a
  * qtDescriptivePhraseModel.
  * Because of that, indices from the QAbstractItemModel will provide properties
  * that can be used by a qtDescriptivePhraseDelegate instance, which this
  * class creates and owns to control how the tree rows are rendered.
  */
class SMTKQTEXT_EXPORT qtResourceBrowser : public qtBaseView
{
  Q_OBJECT
  typedef smtk::extension::qtBaseView Superclass;

public:
  smtkTypenameMacro(qtResourceBrowser);

  static qtBaseView* createViewWidget(const smtk::view::Information& info);
  qtResourceBrowser(const smtk::view::Information& info);
  ~qtResourceBrowser() override;

  static QTreeView* createDefaultView(QWidget* parent);
  QTreeView* view() const;

  smtk::view::PhraseModelPtr phraseModel() const;
  void setPhraseModel(const smtk::view::PhraseModelPtr&);

  smtk::view::SubphraseGeneratorPtr phraseGenerator() const;
  void setPhraseGenerator(smtk::view::SubphraseGeneratorPtr spg);

  smtk::extension::qtDescriptivePhraseModel* descriptivePhraseModel() const;
  void setDescriptivePhraseModel(QAbstractItemModel* qmodel);

  bool highlightOnHover() const;
  void setHighlightOnHover(bool highlight);

  /// Return the string that represents the configuration for browser components
  static const std::string& getJSONConfiguration() { return s_configurationJSON; }

public Q_SLOTS:
  virtual void sendPanelSelectionToSMTK(
    const QItemSelection& selected,
    const QItemSelection& deselected);
  virtual void sendSMTKSelectionToPanel(const std::string& src, smtk::view::SelectionPtr seln);

  virtual void addSource(smtk::common::TypeContainer& managers);
  virtual void removeSource(smtk::common::TypeContainer& managers);

protected Q_SLOTS:
  virtual void hoverRow(const QModelIndex& idx);
  virtual void resetHover();

  /// Called when the user asks to change the color.
  /// This pops up a color editor dialog, which we can make ParaView-specific if needed
  /// and which can be a "singleton" (i.e., re-use the same dialog so that users do not
  /// accidentally pop up one per descriptive phrase and get confused).
  // virtual void editObjectColor(const QModelIndex&);

protected:
  virtual void resetHover(smtk::resource::ComponentSet& add, smtk::resource::ComponentSet& del);
  bool eventFilter(QObject*, QEvent*) override;

  /// Internal in @file qtResourceBrowserP.h
  class Internal;
  Internal* m_p;
  static std::string s_configurationJSON;
};
} // namespace extension
} // namespace smtk
#endif // smtk_extension_qtResourceBrowser_h
