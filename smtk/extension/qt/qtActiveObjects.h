//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef qtActiveObjects_h
#define qtActiveObjects_h

#include <QObject>
#include "smtk/PublicPointerDefs.h"
#include "smtk/model/Model.h" //for smtk::model::Model

namespace smtk {
  namespace model {
    class Model;
  }
}

class pqView;
class vtkSMSessionProxyManager;
class vtkSMViewProxy;


/**
  qtActiveObjects keeps track of active objects.
  This is similar to pqActiveObjects in ParaView, however it tracks objects
  relevant to SMTK and CMB.
*/
class qtActiveObjects : public QObject
{
  Q_OBJECT

  typedef QObject Superclass;

public:
  /// Returns reference to the singleton instance.
  static qtActiveObjects& instance();

  /// Returns the active model.
  smtk::model::Model activeModel() const {return this->m_activeModel;}

public slots:
  /// Set the active module.
  void setActiveModel(const smtk::model::Model& inputModel);

signals:
  /// Fire when activeModel changes.
  void activeModelChanged();

private slots:

protected:
  qtActiveObjects();
  virtual ~qtActiveObjects();

  smtk::model::Model m_activeModel;

private:
  Q_DISABLE_COPY(qtActiveObjects)
};

#endif
