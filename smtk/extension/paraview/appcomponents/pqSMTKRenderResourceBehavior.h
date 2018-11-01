//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKRenderResourceBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKRenderResourceBehavior_h

#include "smtk/extension/paraview/appcomponents/Exports.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"

#include "smtk/resource/Resource.h"

#include <QObject>

/// Given an SMTK resource, this behavior constructs a pipeline source for the
/// resource and adds it to the active view. Resources created using ParaView's
/// File->Open do not need this functionality, as it occurs internally within
/// ParaView. Other created resources (such as those loaded when resolving links,
/// resources created via the operation panel, etc.) do not automatically go
/// through ParaView's machinations; we duplicate ParaView's source-handling
/// process here.
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKRenderResourceBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  static pqSMTKRenderResourceBehavior* instance(QObject* parent = nullptr);
  ~pqSMTKRenderResourceBehavior() override;

public slots:
  pqSMTKResource* createPipelineSource(const smtk::resource::Resource::Ptr&);
  void renderPipelineSource(pqSMTKResource* source);

protected:
  pqSMTKRenderResourceBehavior(QObject* parent = nullptr);

  class Internal;
  Internal* m_p;

private:
  Q_DISABLE_COPY(pqSMTKRenderResourceBehavior);
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKRenderResourceBehavior_h
