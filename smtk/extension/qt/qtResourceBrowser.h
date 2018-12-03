//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/Exports.h"

#include "smtk/PublicPointerDefs.h"

#include <QWidget>

class QItemSelection;
class QTreeView;

namespace smtk
{
namespace extension
{

/**\brief A panel that displays SMTK resources available to the application/user.
  *
  */
class SMTKQTEXT_EXPORT qtResourceBrowser : public QWidget
{
  Q_OBJECT
  typedef QWidget Superclass;

public:
  qtResourceBrowser(const std::string& viewName = "", QWidget* parent = nullptr);
  ~qtResourceBrowser() override;

  static QTreeView* createDefaultView(QWidget* parent);

  smtk::view::PhraseModelPtr phraseModel() const;
  void setPhraseModel(const smtk::view::PhraseModelPtr&);

  smtk::view::SubphraseGeneratorPtr phraseGenerator() const;
  void setPhraseGenerator(smtk::view::SubphraseGeneratorPtr spg);

  bool highlightOnHover() const;
  void setHighlightOnHover(bool highlight);

  void leaveEvent(QEvent*) override;

public slots:
  virtual void sendPanelSelectionToSMTK(
    const QItemSelection& selected, const QItemSelection& deselected);
  virtual void sendSMTKSelectionToPanel(const std::string& src, smtk::view::SelectionPtr seln);

  virtual void addSource(smtk::resource::ManagerPtr rsrcMgr, smtk::operation::ManagerPtr operMgr,
    smtk::view::SelectionPtr seln);
  virtual void removeSource(smtk::resource::ManagerPtr rsrcMgr, smtk::operation::ManagerPtr operMgr,
    smtk::view::SelectionPtr seln);

protected slots:
  virtual void hoverRow(const QModelIndex& idx);
  virtual void resetHover();

  /// Called when the user asks to change the color.
  /// This pops up a color editor dialog, which we can make ParaView-specific if needed
  /// and which can be a "singleton" (i.e., re-use the same dialog so that users do not
  /// accidentally pop up one per descriptive phrase and get confused).
  virtual void editObjectColor(const QModelIndex&);

protected:
  virtual void resetHover(smtk::resource::ComponentSet& add, smtk::resource::ComponentSet& del);

  class Internal;
  Internal* m_p;
};
}
}
