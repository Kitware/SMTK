//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqArtifacts.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"

#include "smtk/io/Logger.h"
#include "smtk/job/Definition.h"
#include "smtk/job/Job.h"
#include "smtk/job/Stage.h"

// ParaView includes
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqUndoStack.h"
#include "pqView.h"

#include "vtkSMParaViewPipelineControllerWithRendering.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMReaderFactory.h"
#include "vtkSMReaderReloadHelper.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMSession.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSourceProxy.h"

// Qt includes
#include <QApplication>
#include <QDebug>
#include <QPointer>
#include <QString>

#include <algorithm>
#include <map>
#include <set>
#include <unordered_map>

#include "moc_pqArtifacts.cpp"

namespace // anonymous
{

pqArtifacts* s_instance{ nullptr };

} // anonymous namespace

class pqArtifacts::Internal
{
public:
  Internal(pqArtifacts* self)
    : m_self(self)
  {
  }

  pqPipelineSource* hasPipeline(
    const std::filesystem::path& path,
    smtk::string::Token tag,
    const std::string& readerGroup,
    const std::string& readerName)
  {
    auto it = m_artifacts.find(path);
    if (it == m_artifacts.end())
    {
      return nullptr;
    }
    auto i2 = it->second.find(tag);
    if (i2 == it->second.end() || !i2->second)
    {
      return nullptr;
    }
    if (
      !readerGroup.empty() && !readerName.empty() &&
      (std::string(i2->second->getProxy()->GetXMLGroup()) != readerGroup ||
       std::string(i2->second->getProxy()->GetXMLName()) != readerName))
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Reader is a (" << std::string(i2->second->getProxy()->GetXMLGroup()) << ", "
                        << std::string(i2->second->getProxy()->GetXMLName()) << "), not a ("
                        << readerGroup << ", " << readerName << ").");
      return nullptr;
    }
    return i2->second.data();
  }

  pqPipelineSource* createPipeline(
    const std::filesystem::path& path,
    smtk::string::Token tag,
    const std::string& readerGroup,
    const std::string& readerName)
  {
    // Force the reader factory to allow ParaView (not just SMTK) readers:
    bool ppm = pqSMTKBehavior::instance()->postProcessingMode();
    pqSMTKBehavior::instance()->setPostProcessingMode(true);

    auto* core = pqApplicationCore::instance();
    auto* server = core ? core->getActiveServer() : nullptr;
    auto* session = server ? server->session() : nullptr;
    auto* proxyManager = vtkSMProxyManager::GetProxyManager();
    auto* readerFactory = proxyManager ? proxyManager->GetReaderFactory() : nullptr;
    if (!readerFactory)
    {
      return nullptr;
    }

    if (readerFactory->GetNumberOfRegisteredPrototypes() == 0)
    {
      readerFactory->UpdateAvailableReaders();
    }
    auto* builder = core->getObjectBuilder();
    const char* xmlGroup = nullptr;
    const char* xmlName = nullptr;
    if (readerGroup.empty() || readerName.empty())
    {
      if (!readerFactory->TestFileReadability(path.string().c_str(), session))
      {
        // Can't read because the file doesn't exist.
        return nullptr;
      }
      if (!readerFactory->CanReadFile(path.string().c_str(), session))
      {
        return nullptr;
      }

      xmlGroup = readerFactory->GetReaderGroup();
      xmlName = readerFactory->GetReaderName();
    }
    else
    {
      xmlGroup = readerGroup.c_str();
      xmlName = readerName.c_str();
    }
    // Re-disable post-processing mode if needed.
    pqSMTKBehavior::instance()->setPostProcessingMode(ppm);

    QStringList files;
    files << QString::fromStdString(path.string());
    pqPipelineSource* source = builder->createReader(xmlGroup, xmlName, files, server);
    if (!source)
    {
      return nullptr;
    }
    auto* readerProxy = source->getSourceProxy();
    m_artifacts[path][tag] = source;

    // Push changes to server so that when the representation gets updated,
    // it uses the property values we set.
    readerProxy->UpdateVTKObjects();

    // ensures that new timestep range, if any gets fetched from the server.
    readerProxy->UpdatePipelineInformation();

    // We have already pushed everything to the server manager.
    // Thus, there is no state left to be modified.
    source->setModifiedState(pqProxy::UNMODIFIED);

    return source;
  }

protected:
  pqArtifacts* m_self{ nullptr };
  std::
    map<std::filesystem::path, std::unordered_map<smtk::string::Token, QPointer<pqPipelineSource>>>
      m_artifacts;
};

pqArtifacts::pqArtifacts()
  : m_p(new Internal(this))
{
}

pqArtifacts::~pqArtifacts()
{
  // m_p->destroyPipelines();
}

pqArtifacts* pqArtifacts::instance()
{
  if (!s_instance)
  {
    s_instance = new pqArtifacts;
  }
  return s_instance;
}

pqPipelineSource* pqArtifacts::findOrCreate(
  const std::filesystem::path& path,
  smtk::string::Token tag,
  bool* didCreate,
  bool findOnly,
  const std::string& readerGroup,
  const std::string& readerName)
{
  std::error_code error;
  auto normalizedPath = std::filesystem::weakly_canonical(path, error);
  if (error)
  {
    normalizedPath = path.lexically_normal();
  }
  if (auto* pipeline = m_p->hasPipeline(normalizedPath, tag, readerGroup, readerName))
  {
    if (didCreate)
    {
      *didCreate = false;
    }
    return pipeline;
  }
  if (findOnly)
  {
    if (didCreate)
    {
      *didCreate = false;
    }
    return nullptr;
  }
  auto* src = m_p->createPipeline(normalizedPath, tag, readerGroup, readerName);
  if (src && didCreate)
  {
    *didCreate = true;
  }
  return src;
}

void pqArtifacts::reload(const smtk::job::Job& job, int firstStage, int endStage)
{
  auto definition = job.jobType();
  if (!definition)
  {
    return;
  }

  // Multiple newly completed stages can name the same artifact. Reload each
  // source only once per notification; a later stage may reload it again if
  // that stage also declares it as an output.
  std::set<pqPipelineSource*> sources;
  const auto& stages = definition->stages();
  endStage = std::min(endStage, static_cast<int>(stages.size()));
  for (int index = std::max(firstStage, 0); index < endStage; ++index)
  {
    for (const auto& artifact : stages[index]->artifacts())
    {
      if (!artifact.empty())
      {
        // findOnly=true keeps job progress from loading artifacts that the user
        // has never opened. Cached sources are refreshed even when hidden.
        if (auto* source = this->findOrCreate(job.caseDirectory() / artifact, "job", nullptr, true))
        {
          sources.insert(source);
        }
      }
    }
  }

  // Reload in place to preserve downstream filters and representation settings.
  // Use the helper directly to avoid the interactive file-series question.
  vtkNew<vtkSMReaderReloadHelper> helper;
  bool reloaded = false;
  // Refreshing files is an external-data update, not an undoable user edit.
  BEGIN_UNDO_EXCLUDE();
  for (auto* source : sources)
  {
    if (helper->ReloadFiles(source->getSourceProxy()))
    {
      reloaded = true;
    }
  }
  if (reloaded)
  {
    // Render once after the batch, including views other than the active view.
    pqApplicationCore::instance()->render();
  }
  END_UNDO_EXCLUDE();
}

vtkSMRepresentationProxy* pqArtifacts::activeViewRepresentation(
  pqPipelineSource* source,
  bool visibility)
{
  if (!source)
  {
    return nullptr;
  }
  // Create representation for the active view if it's a render view
  pqView* view = pqActiveObjects::instance().activeView();
  vtkSMRepresentationProxy* proxy = nullptr;
  if (pqRenderView* renderView = dynamic_cast<pqRenderView*>(view))
  {
    vtkNew<vtkSMParaViewPipelineControllerWithRendering> controller;
    if (visibility)
    {
      proxy = dynamic_cast<vtkSMRepresentationProxy*>(
        controller->Show(source->getSourceProxy(), 0, renderView->getViewProxy()));
      // renderView->resetCamera();
      if (proxy)
      {
        proxy->SetRepresentationType("Surface With Edges");
      }
    }
    else
    {
      proxy = dynamic_cast<vtkSMRepresentationProxy*>(
        controller->Hide(source->getSourceProxy(), 0, renderView->getViewProxy()));
    }
  }
  if (!proxy)
  {
    qCritical() << "No active view or data (" << source->getSMName()
                << ") cannot be displayed in it.";
  }
  return proxy;
}

void pqArtifacts::hideRepresentations(pqPipelineSource* source)
{
  if (!source)
  {
    return;
  }

  // An artifact can have a representation in more than the currently-active
  // view. During a task transition, the active view can also change before
  // cleanup is performed. Hide every representation owned by the artifact
  // source and render each affected view instead of relying on active objects.
  const auto views = source->getViews();
  for (auto* view : views)
  {
    if (!view)
    {
      continue;
    }
    for (auto* representation : source->getRepresentations(view))
    {
      if (representation)
      {
        representation->setVisible(false);
      }
    }
    view->render();
  }
}
