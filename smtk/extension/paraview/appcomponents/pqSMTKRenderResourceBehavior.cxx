//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKRenderResourceBehavior.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/extension/vtk/geometry/Backend.h"

#include "smtk/geometry/Resource.h"

#include "smtk/resource/Manager.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqApplyBehavior.h"
#include "pqCameraReaction.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqProxy.h"

#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"

namespace
{

bool g_alwaysCreatePipelines = false;

} // anonymous namespace

// pqApplyBehavior is ParaView's behavior for "handling the logic that needs to
// happen after the user hits "Apply" on the pqPropertiesPanel." We require its
// functionality so we can show the proxies for resources that were created
// outside of ParaView's File->Open command.
class pqSMTKApplyBehavior : public pqApplyBehavior
{
public:
  ~pqSMTKApplyBehavior() override = default;
  void operator()(pqProxy* proxy) { this->applied(nullptr, proxy); }
};

class pqSMTKRenderResourceBehavior::Internal
{
public:
  ~Internal() = default;

  pqSMTKApplyBehavior ApplyBehavior;

  smtk::resource::Observers::Key key;
};

static pqSMTKRenderResourceBehavior* g_instance = nullptr;

pqSMTKRenderResourceBehavior::pqSMTKRenderResourceBehavior(QObject* parent)
  : Superclass(parent)
  , m_p(new Internal)
{
  // Whenever an SMTK wrapper is added to the server...
  auto onAddedManagerOnServer = [this](pqSMTKWrapper* wrapper, pqServer* /*unused*/) {
    if (!wrapper)
    {
      return;
    }

    // ...add an observer to its resource manager that destroys the pipeline
    // source associated with a resource removed from the manager.
    m_p->key = wrapper->smtkResourceManager()->observers().insert(
      [this](const smtk::resource::Resource& resource, smtk::resource::EventType eventType) {
        auto rsrc = const_cast<smtk::resource::Resource&>(resource).shared_from_this();
        switch (eventType)
        {
          case smtk::resource::EventType::REMOVED:
          {
            this->destroyPipelineSource(rsrc);
            break;
          }
          case smtk::resource::EventType::ADDED:
            QTimer::singleShot(0, this, [this, rsrc]() {
              if (pqSMTKRenderResourceBehavior::alwaysCreatePipelines())
              {
                this->createPipelineSource(rsrc);
                return;
              }
              // Only add pipeline sources for geometry resources...
              if (auto geomRsrc = std::dynamic_pointer_cast<smtk::geometry::Resource>(rsrc))
              {
                // ... that also provide geometry for the VTK backend:
                smtk::extension::vtk::geometry::Backend vtk;
                if (geomRsrc->geometry(vtk))
                {
                  this->createPipelineSource(rsrc);
                }
              }
            });
            break;
          default:
            // Do nothing.
            break;
        }
      },
      "pqSMTKRenderResourceBehavior: Destroy pipeline sources when resource is removed.");
    return;
  };

  QObject::connect(
    pqSMTKBehavior::instance(),
    (void (pqSMTKBehavior::*)(pqSMTKWrapper*, pqServer*)) & pqSMTKBehavior::addedManagerOnServer,
    onAddedManagerOnServer);
}

pqSMTKRenderResourceBehavior* pqSMTKRenderResourceBehavior::instance(QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new pqSMTKRenderResourceBehavior(parent);
  }

  if (g_instance->parent() == nullptr && parent)
  {
    g_instance->setParent(parent);
  }

  return g_instance;
}

pqSMTKRenderResourceBehavior::~pqSMTKRenderResourceBehavior()
{
  if (g_instance == this)
  {
    g_instance = nullptr;
  }

  QObject::disconnect(this);
  delete m_p;
}

bool pqSMTKRenderResourceBehavior::setAlwaysCreatePipelines(bool shouldCreate)
{
  if (g_alwaysCreatePipelines == shouldCreate)
  {
    return false;
  }
  g_alwaysCreatePipelines = shouldCreate;
  return true;
}

bool pqSMTKRenderResourceBehavior::alwaysCreatePipelines()
{
  return g_alwaysCreatePipelines;
}

pqSMTKResource* pqSMTKRenderResourceBehavior::createPipelineSource(
  const smtk::resource::Resource::Ptr& resource)
{
  pqApplicationCore* pqCore = pqApplicationCore::instance();
  pqServer* server = pqActiveObjects::instance().activeServer();

  // If there is no active server, there is not much we can do.
  if (server == nullptr)
  {
    return nullptr;
  }

  if (auto preExisting = pqSMTKBehavior::instance()->getPVResource(resource))
  {
    return preExisting;
  }

  pqObjectBuilder* builder = pqCore->getObjectBuilder();

  pqSMTKResource* source =
    static_cast<pqSMTKResource*>(builder->createSource("sources", "SMTKResourceSource", server));
  vtkSMPropertyHelper(source->getProxy(), "ResourceId").Set(resource->id().toString().c_str());

  this->renderPipelineSource(source);
  Q_EMIT this->pipelineSourceCreated(resource, source);

  return source;
}

void pqSMTKRenderResourceBehavior::destroyPipelineSource(
  const smtk::resource::Resource::Ptr& resource)
{
  pqSMTKResource* source = pqSMTKBehavior::instance()->getPVResource(resource);

  if (source != nullptr)
  {
    Q_EMIT this->aboutToDestroyPipelineSource(resource, source);

    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    builder->destroy(source);

    if (auto* activeView = pqActiveObjects::instance().activeView())
    {
      activeView->render();
    }
  }
}

void pqSMTKRenderResourceBehavior::renderPipelineSource(pqSMTKResource* source)
{
  source->getProxy()->UpdateVTKObjects();

  m_p->ApplyBehavior(source);

  if (auto* activeView = pqActiveObjects::instance().activeView())
  {
    activeView->render();
    pqCameraReaction::zoomToData();
  }
}
