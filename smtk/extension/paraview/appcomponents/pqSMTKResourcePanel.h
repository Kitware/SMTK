//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/Exports.h"

#include "smtk/PublicPointerDefs.h"

#include <QDockWidget>

class QItemSelection;

class pqSMTKResource;
class pqSMTKWrapper;

class pqRepresentation;
class pqServer;
class pqView;

/**\brief A panel that displays SMTK resources available to the application/user.
  *
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKResourcePanel : public QDockWidget
{
  Q_OBJECT
  typedef QDockWidget Superclass;

public:
  pqSMTKResourcePanel(QWidget* parent = nullptr);
  ~pqSMTKResourcePanel() override;

  smtk::view::PhraseModelPtr model() const;

  smtk::view::SubphraseGeneratorPtr phraseGenerator() const;
  void setPhraseGenerator(smtk::view::SubphraseGeneratorPtr spg);

  void leaveEvent(QEvent*) override;

public slots:
  virtual void sendPanelSelectionToSMTK(
    const QItemSelection& selected, const QItemSelection& deselected);
  virtual void sendSMTKSelectionToPanel(const std::string& src, smtk::view::SelectionPtr seln);

protected slots:
  virtual void searchTextChanged(const QString& searchText);

  virtual void resourceManagerAdded(pqSMTKWrapper* mgr, pqServer* server);
  virtual void resourceManagerRemoved(pqSMTKWrapper* mgr, pqServer* server);

  /// Used to update phrase model with new visibility info for the active view.
  virtual void activeViewChanged(pqView*);

  /// Used to keep list of resource representations in active view up-to-date.
  virtual void representationAddedToActiveView(pqRepresentation*);
  virtual void representationRemovedFromActiveView(pqRepresentation*);

  /// Used to listen for self and others making changes to component visibilities in active view's representations.
  virtual void componentVisibilityChanged(smtk::resource::ComponentPtr comp, bool visible);

  virtual void hoverRow(const QModelIndex& idx);
  virtual void resetHover();

  /// Called when vtkSMTKSettings is modified, indicating highlight-on-hover behavior may change.
  virtual void updateSettings();

protected:
  virtual void resetHover(smtk::resource::ComponentSet& add, smtk::resource::ComponentSet& del);

  class Internal;
  Internal* m_p;
};
