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

class pqServer;
class pqSMTKResource;
class pqSMTKResourceManager;

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

public slots:
  virtual void updateTopLevel();

protected slots:
  virtual void searchTextChanged(const QString& searchText);

  virtual void resourceManagerAdded(pqSMTKResourceManager* mgr, pqServer* server);
  virtual void resourceManagerRemoved(pqSMTKResourceManager* mgr, pqServer* server);

protected:
  class Internal;
  Internal* m_p;
};
