//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqJobRunnerView.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/job/pqArtifacts.h"

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtLogView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/job/Definition.h"
#include "smtk/job/Job.h"
#include "smtk/job/Queue.h"
#include "smtk/job/Stage.h"
#include "smtk/job/agents/JobAgent.h"
#include "smtk/job/operators/CancelJob.h"

#include "smtk/task/Active.h"
#include "smtk/task/Manager.h"
#include "smtk/task/ObjectsInRoles.h"
#include "smtk/task/Port.h"
#include "smtk/task/Task.h"

#include "smtk/operation/Operation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

// ParaView includes
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqReloadFilesReaction.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqView.h"

#include "vtkSMParaViewPipelineControllerWithRendering.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMReaderFactory.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMSession.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSourceProxy.h"

// VTK includes
#include "vtkNew.h"

// Qt includes
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileSystemWatcher>
#include <QFont>
#include <QIconEngine>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QSlider>
#include <QSvgRenderer>
#include <QTimer>

#include <chrono>
#include <ctime>
#include <sstream>

#include "moc_pqJobRunnerView.cpp"

// Set this to a non-zero value to print debug information.
#define SMTK_DEBUG 0

Q_DECLARE_METATYPE(smtk::attribute::Attribute::Ptr);

using namespace smtk::string::literals;

namespace // anonmyous
{

template<typename TP>
std::time_t to_time_t(TP tp)
{
  using namespace std::chrono;
  auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
  return system_clock::to_time_t(sctp);
}

// Render embedded paths directly; no application font or SVG plugin is needed.
class JobIconEngine : public QIconEngine
{
public:
  JobIconEngine(const QString& name, QWidget* widget)
    : m_name(name)
    , m_widget(widget)
  {
  }

  QIconEngine* clone() const override { return new JobIconEngine(m_name, m_widget); }

  void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State) override
  {
    const auto palette = m_widget ? m_widget->palette() : QApplication::palette();
    const auto color = palette.color(
      mode == QIcon::Disabled ? QPalette::Disabled : QPalette::Active, QPalette::WindowText);
    if (m_color != color || !m_renderer.isValid())
    {
      QFile file(":/icons/diagram/" + m_name + ".svg");
      if (file.open(QIODevice::ReadOnly))
      {
        auto data = file.readAll();
        data.replace("#000000", color.name().toUtf8());
        m_renderer.load(data);
        m_color = color;
      }
    }
    painter->save();
    painter->setOpacity(painter->opacity() * color.alphaF());
    m_renderer.render(painter, rect);
    painter->restore();
  }

  QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override
  {
    QPixmap result(size);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    this->paint(&painter, QRect(QPoint(), size), mode, state);
    return result;
  }

private:
  QString m_name;
  QPointer<QWidget> m_widget;
  QColor m_color;
  QSvgRenderer m_renderer;
};

QIcon jobIcon(const QString& name, QWidget* widget)
{
  return QIcon(new JobIconEngine(name, widget));
}

class JobStatusLabel : public QLabel
{
public:
  void setSymbol(const QString& name)
  {
    m_name = name;
    this->setAccessibleName(name);
    m_color = QColor();
    this->update();
  }

  QSize sizeHint() const override
  {
    const int side = this->fontMetrics().height();
    return QSize(side, side);
  }

protected:
  void paintEvent(QPaintEvent*) override
  {
    if (m_name.isEmpty())
    {
      return;
    }
    const auto color = this->palette().color(QPalette::WindowText);
    if (m_color != color)
    {
      QFile file(":/icons/diagram/" + m_name + ".svg");
      if (file.open(QIODevice::ReadOnly))
      {
        auto data = file.readAll();
        data.replace("#000000", color.name().toUtf8());
        m_renderer.load(data);
        m_color = color;
      }
    }
    QPainter painter(this);
    const int side = qMin(this->width(), this->height());
    m_renderer.render(&painter, QRectF(0, (this->height() - side) / 2.0, side, side));
  }

private:
  QString m_name;
  QPointer<QWidget> m_widget;
  QColor m_color;
  QSvgRenderer m_renderer;
};

} // anonymous namespace

class qtArtifactControlWidget : public QWidget
{
public:
  qtArtifactControlWidget(
    int stage,
    const std::set<std::filesystem::path>& artifacts,
    pqJobRunnerView* view)
    : m_view(view)
    , m_stage(stage)
  {
    auto* layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);
    m_artifactButton = new QPushButton;
    m_artifactButton->setIcon(jobIcon("settings", m_artifactButton));
    m_artifactButton->setToolTip("Artifact controls");
    m_artifactButton->setEnabled(false);
    layout->addWidget(m_artifactButton);
    m_artifactControls = new QWidget;
    auto* acLayout = new QVBoxLayout;
    m_artifactControls->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Popup);
    m_artifactControls->setLayout(acLayout);
    m_maximumLabelWidth = 0;
    for (const auto& artifact : artifacts)
    {
      if (artifact.empty())
      {
        continue;
      }

      std::filesystem::path fullArtifactPath;
      vtkSMProxy* repProxy = nullptr;
      auto* nameLabel = new QLabel(QString::fromStdString(artifact.string()));
      int labelWidth = nameLabel->fontMetrics().boundingRect(nameLabel->text()).width();
      m_maximumLabelWidth = std::max(m_maximumLabelWidth, labelWidth);
      auto* aLayout = new QHBoxLayout;
      // Use the path hash as the layout name so we can find widgets without lots of maps.
      aLayout->setObjectName(QString::number(std::filesystem::hash_value(artifact), 16));
      auto* visibilityButton = new QPushButton;
      visibilityButton->setObjectName("visibility");
      visibilityButton->setToolTip("Toggle artifact visibility");
      visibilityButton->setCheckable(true);
      visibilityButton->setChecked(true);
      QObject::connect(
        visibilityButton,
        &QPushButton::toggled,
        [this, visibilityButton, &artifact](bool makeVisible) {
          if (this->toggleArtifactVisibility(artifact, makeVisible))
          {
            visibilityButton->setIcon(
              jobIcon(makeVisible ? "visible" : "hidden", visibilityButton));
          }
        });
      auto* opacitySlider = new QSlider;
      opacitySlider->setObjectName("opacity");
      opacitySlider->setOrientation(Qt::Horizontal);
      opacitySlider->setRange(0, 255);
      if (repProxy)
      {
        vtkSMPropertyHelper vis(repProxy, "Visibility");
        visibilityButton->setIcon(jobIcon(vis.GetAsInt() ? "visible" : "hidden", visibilityButton));
        vtkSMPropertyHelper alpha(repProxy, "Opacity");
        opacitySlider->setValue(static_cast<int>(alpha.GetAsDouble() * 255.0));
      }
      else
      {
        visibilityButton->setIcon(jobIcon("hidden", visibilityButton));
        opacitySlider->setValue(255);
      }
      // Keep these *after* the slider is initialized above.
      QObject::connect(opacitySlider, &QSlider::sliderMoved, [&](int value) {
        this->setArtifactOpacity(artifact, value);
      });
      QObject::connect(opacitySlider, &QSlider::valueChanged, [&](int value) {
        this->setArtifactOpacity(artifact, value);
      });
      aLayout->addWidget(nameLabel);
      aLayout->addWidget(visibilityButton);
      aLayout->addWidget(opacitySlider);
      acLayout->addLayout(aLayout);
    }

    // When the m_artifactButton is clicked, pop up m_artifactControls.
    QObject::connect(
      m_artifactButton, &QPushButton::clicked, this, &qtArtifactControlWidget::toggleControls);
  }

  ~qtArtifactControlWidget() override = default;

  void updateJobStage(int currentJobStage)
  {
    // Preserve normal access to artifacts from stages the job has already passed.
    bool available = currentJobStage > m_stage;
    if (auto* job = m_view->currentJob())
    {
      // Debugging may create a reader marker
      // after a failed stage. Permit inspection of existing partial artifacts
      // once the job has stopped, without advancing its stage or changing its
      // success status. Do not use this fallback while queued or running: files
      // may still be incomplete or left over from a previous run.
      if (
        job->status() != smtk::job::Pending && job->state() != smtk::job::Running &&
        job->state() != smtk::job::Scheduled)
      {
        for (const auto& artifact : job->jobType()->stages()[m_stage]->artifacts())
        {
          std::error_code ec;
          available |= std::filesystem::exists(job->caseDirectory() / artifact, ec);
        }
      }
    }
    m_artifactButton->setEnabled(available);
  }

  void hideArtifacts()
  {
    if (m_view && m_stage >= 0)
    {
      if (auto* job = m_view->currentJob())
      {
        const auto& stages = job->jobType()->stages();
        if (static_cast<std::size_t>(m_stage) < stages.size())
        {
          const auto& stage = stages[m_stage];
          for (const auto& artifact : stage->artifacts())
          {
            auto fullArtifactPath = job->caseDirectory() / artifact;
            if (
              auto* source =
                pqArtifacts::instance()->findOrCreate(fullArtifactPath, "job", nullptr, true))
            {
              pqArtifacts::hideRepresentations(source);
            }
          }
        }
      }
    }
  }

  QLayout* findLayout(const QString& layoutName)
  {
    if (!m_artifactControls || !m_artifactControls->layout() || layoutName.isEmpty())
    {
      return nullptr;
    }
    auto* parentLayout = m_artifactControls->layout();
    for (int ii = 0; ii < parentLayout->count(); ++ii)
    {
      auto* item = parentLayout->itemAt(ii);
      if (item && item->layout() && item->layout()->objectName() == layoutName)
      {
        return item->layout();
      }
    }
    return nullptr;
  }

  void toggleControls()
  {
    m_artifactControls->setVisible(!m_artifactControls->isVisible());
    if (m_artifactControls->isVisible())
    {
      // auto rect = m_artifactButton->geometry();
      auto topLeft = m_artifactButton->mapToGlobal(QPoint(0, 0));
      auto* screen = qApp->screenAt(topLeft);
      if (!screen)
      {
        screen = qApp->primaryScreen();
      }
#if SMTK_DEBUG
      auto sr = screen->geometry();
      std::cout << "Screen " << screen << " (" << sr.x() << "," << sr.y() << " " << sr.width()
                << "×" << sr.height() << ") "
                << "at " << topLeft.x() << ", " << topLeft.y() << "\n";
      // TODO: Choose placement of m_artifactControls so it is completely on-screen to one side of m_artifactButton.
#endif
      auto bottomLeft = m_artifactButton->mapToGlobal(QPoint(10, 10));
      // auto r2 = m_artifactControls->geometry();
      // r2.setBottomLeft(bottomLeft);
      m_artifactControls->setGeometry(
        QRect(bottomLeft, QSize(m_maximumLabelWidth + 250, m_artifactButton->geometry().height())));

      if (auto* job = m_view->currentJob())
      {
        // Now we need to update the controls for each artifact
        // to reflect the current views/settings.
        for (const auto& artifact : job->jobType()->stages()[m_stage]->artifacts())
        {
          std::filesystem::path fullArtifactPath = job->caseDirectory() / artifact;
          auto layoutName = QString::number(std::filesystem::hash_value(artifact), 16);
          auto* layout = this->findLayout(layoutName);
          if (layout)
          {
            bool didCreate = false;
            auto* visibilityButton = qobject_cast<QPushButton*>(layout->itemAt(1)->widget());
            auto* source =
              pqArtifacts::instance()->findOrCreate(fullArtifactPath, "job", &didCreate);
            pqArtifacts::activeViewRepresentation(source, visibilityButton->isChecked());
          }
        }
      }
    }
  }

  bool toggleArtifactVisibility(const std::filesystem::path& artifact, bool makeVisible)
  {
    // Do not attempt to set visibility if there is no job (and thus no case directory)
    if (auto* job = m_view->currentJob())
    {
      bool didCreate;
      std::filesystem::path fullArtifactPath = job->caseDirectory() / artifact;
      auto* source = pqArtifacts::instance()->findOrCreate(fullArtifactPath, "job", &didCreate);
      // The source may be null if fullArtifactPath does not exist.
      if (auto* repProxy = pqArtifacts::activeViewRepresentation(source, true))
      {
        vtkSMPropertyHelper(repProxy, "Visibility").Set(makeVisible);
        repProxy->UpdateVTKObjects();
        if (auto view = pqActiveObjects::instance().activeView())
        {
          view->render();
        }
        return true;
      }
    }
    return false;
  }

  void setArtifactOpacity(const std::filesystem::path& artifact, int value)
  {
    // Do not attempt to set opacity if there is no job (and thus no case directory)
    if (auto* job = m_view->currentJob())
    {
      bool didCreate;
      std::filesystem::path fullArtifactPath = job->caseDirectory() / artifact;
      auto* source = pqArtifacts::instance()->findOrCreate(fullArtifactPath, "job", &didCreate);
      auto* repProxy = pqArtifacts::activeViewRepresentation(source, true);
      vtkSMPropertyHelper(repProxy, "Opacity").Set(value / 255.);
      if (auto view = pqActiveObjects::instance().activeView())
      {
        view->render();
      }
    }
  }

protected:
  void mousePressEvent(QMouseEvent* event) override
  {
    if (!m_artifactControls->underMouse())
    {
      // The click is not on the widget; hide the widget.
      m_artifactControls->setVisible(false);
      return;
    }
    QWidget::mousePressEvent(event);
  }

  QPushButton* m_artifactButton{ nullptr };
  QWidget* m_artifactControls{ nullptr };
  QPointer<pqJobRunnerView> m_view;
  int m_maximumLabelWidth{ 0 };
  int m_stage{ -1 };
};

class pqJobRunnerView::Internal
{
public:
  Internal(pqJobRunnerView* self, const smtk::view::ConfigurationPtr&)
    : m_view(self)
  {
    QPointer<pqJobRunnerView> selfp(self);
    QWidget* ww = m_view->widget();
    // Configure the widget.
    auto* topLevelLayout = new QVBoxLayout;
    auto* upperLayout = new QHBoxLayout;
    m_lastRun = new QLabel("Last update: —");
    m_lastRun->setObjectName("LastRunLabel");
    m_lastRunStatus = new JobStatusLabel;
    m_lastRunStatus->setObjectName("LastRunStatus");
    m_jobControl = new QPushButton("Run");
    m_jobControl->setObjectName("JobControl");
    m_stageGrid = new QGridLayout;
    m_stageGrid->setObjectName("StageGrid");
    m_stageGrid->setColumnMinimumWidth(0, 20);
    // m_stageGrid->setColumnStretch(0, 1);
    m_stageGrid->setColumnStretch(1, 1);
    m_artifactGrid = new QGridLayout;
    m_artifactGrid->setObjectName("ArtifactGrid");
    upperLayout->addWidget(m_lastRun);
    upperLayout->addStretch();
    upperLayout->addWidget(m_lastRunStatus);
    upperLayout->addWidget(m_jobControl);
    topLevelLayout->addLayout(upperLayout);
    topLevelLayout->addLayout(m_stageGrid);
    topLevelLayout->addLayout(m_artifactGrid);
    ww->setLayout(topLevelLayout);
    // Connect widgets.
    // One-time initialization:
    QObject::connect(m_jobControl, &QPushButton::clicked, self, &::pqJobRunnerView::onRunClicked);
    m_agent = smtk::job::agents::JobAgent::activeJobAgent(
      self->uiManager()->managers(), smtk::string::Token());
    // Task managers are owned by projects and are not required to be present
    // in the application's Managers container. Obtain the exact manager that
    // owns this view's job agent instead of looking for an application-scoped
    // task manager (which can leave the observer uninstalled).
    if (m_agent && m_agent->parent() && m_agent->parent()->manager())
    {
      m_taskManager = m_agent->parent()->manager()->shared_from_this();
    }
    if (auto taskManager = m_taskManager.lock())
    {
      // A normal task transition occurs while ParaView's rendering UI is
      // intact. Hide this task's artifacts here instead of relying on widget
      // destruction, which may occur too late to be safe during shutdown.
      m_activeTaskObserver = taskManager->active().observers().insert(
        [this](smtk::task::Task* previous, smtk::task::Task* next) {
          if (previous != next && m_agent && previous == m_agent->parent())
          {
            this->hideArtifacts();
          }
        },
        // Run before attribute-panel observers (which use the default
        // priority). They replace the old task's view synchronously and thus
        // destroy this object and unregister this observer. At the default
        // priority, that could happen before this callback was invoked,
        // leaving the old task's artifact representations visible.
        1,
        false,
        "pqJobRunnerView: Hide artifacts when leaving the owning task.");
    }
    // Creating a debug marker does not change the job's state or emit a job
    // progress notification. Refresh availability on filesystem changes so the
    // user can open Artifact controls without leaving and re-entering the task.
    QObject::connect(&m_artifactWatcher, &QFileSystemWatcher::directoryChanged, self, [this]() {
      this->updateStages(m_agent->job());
    });
    if (auto manager = self->uiManager()->operationManager())
    {
      m_operationObserver = manager->observers().insert(
        [this, selfp](
          const smtk::operation::Operation&,
          smtk::operation::EventType event,
          smtk::operation::Operation::Result result) {
          if (!selfp || event != smtk::operation::EventType::DID_OPERATE || !result)
          {
            return 0;
          }
          auto modified = result->findComponent("modified");
          if (modified)
          {
            for (const auto& component : *modified)
            {
              if (component.get() == m_agent->job())
              {
                // This also covers jobs restored while already running, for which
                // this view did not install a launch-time queue observer.
                QTimer::singleShot(0, selfp, &pqJobRunnerView::updateUI);
                break;
              }
            }
          }
          return 0;
        },
        0,
        false,
        "Refresh restored job controls.");
    }
  }

  ~Internal()
  {
    if (auto taskManager = m_taskManager.lock())
    {
      taskManager->active().observers().erase(m_activeTaskObserver);
    }
    // The owning view normally empties these grids first. Do not modify
    // representation visibility from this destructor because application
    // shutdown may already be destroying ParaView's rendering UI observers.
    this->emptyGrids(/* hideArtifacts */ false);
  }

  void updateLastRunTime(smtk::job::Job* job)
  {
    if (!job)
    {
      m_lastRun->setText("Last update: —");
      return;
    }
    auto modificationTime = job->modificationTime();
    std::ostringstream text;
    if (modificationTime < 0)
    {
      text << "Last update: —";
    }
    else
    {
      char timestamp[128];
      const auto* localTime = std::localtime(&modificationTime);
      if (
        localTime &&
        std::strftime(timestamp, sizeof(timestamp), "%a %b %d %H:%M:%S %Y\n", localTime) != 0)
      {
        text << "Last update: " << timestamp;
      }
      else
      {
        text << "Last update: —";
      }
    }
    m_lastRun->setText(QString::fromStdString(text.str()));
    QPalette palette = m_lastRunStatus->palette();
    switch (job->status())
    {
      default:
      case smtk::job::Status::Pending:
        m_lastRunStatus->setSymbol(QStringLiteral("pending"));         // Spinner
        palette.setColor(QPalette::WindowText, QColor(100, 100, 100)); // medium grey
        break;
      case smtk::job::Status::Failed:
        m_lastRunStatus->setSymbol(QStringLiteral("warning"));       // Exclamation
        palette.setColor(QPalette::WindowText, QColor(210, 65, 34)); // dark red
        break;
      case smtk::job::Status::Succeeded:
        m_lastRunStatus->setSymbol(QStringLiteral("success"));       // Circle check
        palette.setColor(QPalette::WindowText, QColor(74, 166, 33)); // dark green
        break;
      case smtk::job::Status::Terminated:
        m_lastRunStatus->setSymbol(QStringLiteral("failure"));       // Circle x-mark
        palette.setColor(QPalette::WindowText, QColor(210, 65, 34)); // dark red
        break;
    }
    m_lastRunStatus->setPalette(palette);
  }

  void addStages(smtk::job::Definition* jobType)
  {
    if (!jobType)
    {
      return;
    }
    int ii = 0;
    for (const auto& stage : jobType->stages())
    {
      auto* stageLabel = new QLabel(QString::fromStdString(stage->name()));
      stageLabel->setToolTip(QString::fromStdString(stage->description()));
      m_stageGrid->addWidget(stageLabel, ii, 1);
      auto* stageDone = new JobStatusLabel;
      QPalette palette = stageDone->palette();
      palette.setColor(QPalette::WindowText, QColor(74, 166, 33)); // medium green
      stageDone->setPalette(palette);
      m_stageGrid->addWidget(stageDone, ii, 2);
      if (!stage->log().empty())
      {
        auto* stageLog = new QPushButton;
        stageLog->setIcon(jobIcon("log", stageLog));
        stageLog->setToolTip("View stage log");
        stageLog->setObjectName("log stage " + QString::number(ii));
        m_stageGrid->addWidget(stageLog, ii, 3);
        QObject::connect(stageLog, &QPushButton::clicked, [&]() {
          auto* job = m_agent->job();
          if (job)
          {
            auto* dialog = new smtk::qt::qtLogView(job->caseDirectory() / stage->log());
            // Keep this dialog modal until you can properly detect when the log is
            // truncated by another run.
            dialog->exec();
            delete dialog;
          }
        });
      }
      if (!stage->artifacts().empty())
      {
        auto* artifactControlWidget = new qtArtifactControlWidget(ii, stage->artifacts(), m_view);
        m_stageGrid->addWidget(artifactControlWidget, ii, 4);
      }
      ++ii;
    }
    m_stageGrid->update();
  }

  void updateStages(smtk::job::Job* job)
  {
    if (!job)
    {
      return;
    }
    // Watch parent directories because a missing artifact cannot itself be
    // watched. Reconcile the paths on each job update to follow case-directory
    // changes and pick up directories created by later stages.
    QStringList directories;
    for (const auto& stage : job->jobType()->stages())
    {
      for (const auto& artifact : stage->artifacts())
      {
        auto path = (job->caseDirectory() / artifact).parent_path();
        std::error_code ec;
        if (std::filesystem::is_directory(path, ec))
        {
          directories.append(QString::fromStdString(path.string()));
        }
      }
    }
    directories.removeDuplicates();
    const auto watched = m_artifactWatcher.directories();
    for (const auto& path : watched)
    {
      if (!directories.contains(path))
      {
        m_artifactWatcher.removePath(path);
      }
    }
    for (const auto& path : directories)
    {
      if (!watched.contains(path))
      {
        m_artifactWatcher.addPath(path);
      }
    }
    int stageIndex = job->stage();
    bool crashed = job->status() == smtk::job::Status::Failed;
    for (int ii = 0; static_cast<std::size_t>(ii) < job->jobType()->stages().size(); ++ii)
    {
      if (auto* layoutItem = m_stageGrid->itemAtPosition(ii, 2))
      {
        if (auto* label = dynamic_cast<JobStatusLabel*>(layoutItem->widget()))
        {
          if (crashed)
          {
            // The final stage label should be marked with failure and colored red
            // while all completed stages should be marked with check marks and green.
            // clang-format off
            QPalette palette = label->palette();
            palette.setColor(
              QPalette::WindowText,
              ii < stageIndex - 1 ?
                QColor(74, 166, 33) /* medium green */ :
                QColor(210,  65,  34) /* dark red */);
            // clang-format on
            label->setPalette(palette);
            label->setSymbol(
              ii < stageIndex - 1 ? QStringLiteral("success")
                                  : (ii == stageIndex - 1 ? QStringLiteral("warning") : QString()));
          }
          else
          {
            QPalette palette = label->palette();
            palette.setColor(QPalette::WindowText, QColor(74, 166, 33)); // medium green
            label->setPalette(palette);
            label->setSymbol(
              ii < stageIndex ? QStringLiteral("success")
                              : (ii == stageIndex ? QStringLiteral("pending") : QString()));
          }
        }
      }
      if (auto* layoutItem = m_stageGrid->itemAtPosition(ii, 4))
      {
        if (auto* control = dynamic_cast<qtArtifactControlWidget*>(layoutItem->widget()))
        {
          control->updateJobStage(stageIndex);
        }
      }
    }
  }

  void emptyGrids(bool hideArtifacts = true)
  {
    if (hideArtifacts)
    {
      this->hideArtifacts();
    }
    // Reset grids.
    QLayoutItem* child;
    while ((child = m_stageGrid->takeAt(0)) != nullptr)
    {
      delete child->widget();
      delete child;
    }
    while ((child = m_artifactGrid->takeAt(0)) != nullptr)
    {
      delete child->widget();
      delete child;
    }
  }

  void hideArtifacts()
  {
    // Hiding is part of a normal task/view transition, not widget
    // destruction. Destructors may run while ParaView's color-map editor
    // and other active-representation observers are being torn down.
    for (int ii = 0; ii < m_stageGrid->count(); ++ii)
    {
      if (auto* control = dynamic_cast<qtArtifactControlWidget*>(m_stageGrid->itemAt(ii)->widget()))
      {
        control->hideArtifacts();
      }
    }
  }

  void updateJobControls()
  {
    auto job = m_agent->job();
    if (m_lastJob.lock().get() != job || !job)
    {
      this->emptyGrids();
      // Populate grids.
      this->addStages(job ? job->jobType() : m_agent->jobType());
      // this->addArtifacts(job);
    }
    // Update m_lastRun to show the time the last job was started.
    this->updateLastRunTime(job);

    // Update m_jobControl to show "Run" or "Cancel" depending on the job state.
    if (!job || job->status() != smtk::job::Status::Pending)
    {
      m_jobControl->setText("Run");
    }
    else
    {
      m_jobControl->setText("Cancel");
    }
    // This method is only called when it is safe to click the button:
    m_jobControl->setEnabled(true);

    // Update the "job control" area to show the job stages, logs, and artifacts.
    this->updateStages(job);
  }

  ::pqJobRunnerView* m_view{ nullptr };
  QLabel* m_lastRun{ nullptr };
  JobStatusLabel* m_lastRunStatus{ nullptr };
  QPushButton* m_jobControl{ nullptr };
  QGridLayout* m_stageGrid{ nullptr };
  QGridLayout* m_artifactGrid{ nullptr };
  QFileSystemWatcher m_artifactWatcher;
  smtk::operation::Observers::Key m_operationObserver;
  smtk::job::agents::JobAgent* m_agent{ nullptr };
  int m_jobObserver{ -1 };
  std::weak_ptr<smtk::job::Job> m_lastJob;
  std::weak_ptr<smtk::task::Manager> m_taskManager;
  smtk::task::Active::Observers::Key m_activeTaskObserver;
};

smtk::extension::qtBaseView* pqJobRunnerView::createViewWidget(const smtk::view::Information& info)
{
  pqJobRunnerView* view = new pqJobRunnerView(info);
  return view;
}

pqJobRunnerView::pqJobRunnerView(const smtk::view::Information& info)
  : Superclass(info)
{
  qRegisterMetaType<smtk::attribute::Attribute::Ptr>("smtk::attribute::Attribute::Ptr");
  this->createWidget();
  m_p =
    std::make_unique<Internal>(this, this->configuration()); // NB: Must come after createWidget().
  // Artifact controls call currentJob(), which accesses m_p. Populate them only
  // after make_unique has returned and the internal state has been assigned.
  // Doing this in Internal's constructor crashes when reopening an existing job.
  m_p->updateJobControls();
}

pqJobRunnerView::~pqJobRunnerView()
{
  // Empty the UI before the pointer to the job in m_p is destroyed, but do not
  // change representation visibility during application shutdown. Doing so
  // emits active-representation events to ParaView widgets that may already
  // be partially destroyed.
  m_p->emptyGrids(/* hideArtifacts */ false);
}

smtk::job::Job* pqJobRunnerView::currentJob() const
{
  auto lastJob = m_p->m_lastJob.lock();
  if (!lastJob)
  {
    auto* agentJob = m_p->m_agent->job();
    if (agentJob)
    {
      lastJob = agentJob->shared_from_this();
      m_p->m_lastJob = lastJob;
    }
  }
  return lastJob.get();
}

void pqJobRunnerView::updateUI()
{
  this->updateJobControls();
}

void pqJobRunnerView::onRunClicked()
{
  if (!m_p->m_agent)
  {
    return;
  }

  smtk::string::Token action = smtk::string::Token(m_p->m_jobControl->text().toStdString());
  switch (action.id())
  {
    case "Cancel"_hash:
    {
      if (auto job = m_p->m_agent->job())
      {
        if (auto operationManager = this->uiManager()->operationManager())
        {
          m_p->m_jobControl->setEnabled(false);
          auto op = operationManager->create<smtk::job::CancelJob>();
          op->parameters()->associate(job->shared_from_this());
          operationManager->launchers()(op);
        }
      }
    }
    break;
    default:
    case "Run"_hash:
    {
      auto op = m_p->m_agent ? m_p->m_agent->operation() : nullptr;
      if (!op)
      {
        QString message("Internal Error: failed to fetch operation used to schedule jobs.");
        QMessageBox::critical(this->Widget, "Error", message);
        return;
      }

      // If the operation has a "task" parameter, populate it with the given task.
      auto taskItem = op->parameters()->findComponent("task");
      if (taskItem)
      {
        taskItem->appendValue(m_p->m_agent->parent()->shared_from_this());
      }

      {
        // Check if case directory exists
        auto caseDir(m_p->m_agent->caseDirectory());
        if (std::filesystem::exists(caseDir))
        {
          QString message;
          QTextStream qs(&message);
          qs << "This action may delete the current analysis folder "
             << QString::fromStdString(caseDir.string()) << " including any solution data there."
             << "\n\nAre you sure you want to proceed?";

          auto buttons = QMessageBox::Ok | QMessageBox::Cancel;
          auto reply =
            QMessageBox::warning(this->Widget, "Warning", message, buttons, QMessageBox::Cancel);
          if (reply != QMessageBox::Ok)
          {
            qInfo() << "Aborting Run";
            return;
          }
        } // if caseDir exists
      }   // if deleteCaseDirectory

      // Now that we are sure we will launch the operation, disable the "Run" button.
      // It will get re-enabled when the job is queued.
      m_p->m_jobControl->setEnabled(false);

      // Case-construction operations may remove and recreate the entire case
      // directory. On Windows, leaving QFileSystemWatcher attached while that
      // happens makes its FindNextChangeNotification call fail with access
      // denied. Stop watching before launching the operation; the completion
      // handler below refreshes the controls and reinstalls watches for the
      // newly-created directories.
      const auto watchedDirectories = m_p->m_artifactWatcher.directories();
      if (!watchedDirectories.empty())
      {
        m_p->m_artifactWatcher.removePaths(watchedDirectories);
      }

      // Launch the operation with a handler so that when it completes we
      // can submit the input deck as a job in a separate process.
      QPointer<pqJobRunnerView> self(this);
      op->addHandler(
        [self,
         this](smtk::operation::Operation& op, const smtk::operation::Operation::Result& res) {
          (void)op;
          // std::cerr << "Handler, op " << &op << " res " << res << "\n";
          if (!self)
          {
            return;
          }
          if (smtk::operation::outcome(res) != smtk::operation::Operation::Outcome::SUCCEEDED)
          {
            // The operation that was supposed to create the job did not succeed.
            // Reset the "Run" button so users can try again.
            m_p->m_jobControl->setText("Run");
            m_p->m_jobControl->setEnabled(true);
          }
          else
          {
            for (const auto& obj : *res->findComponent("created"))
            {
              if (auto job = std::dynamic_pointer_cast<smtk::job::Job>(obj))
              {
                if (auto* queue = job->queue())
                {
                  if (m_p->m_jobObserver >= 0)
                  {
                    if (auto lastJob = m_p->m_lastJob.lock())
                    {
                      lastJob->queue()->unobserve(lastJob.get(), m_p->m_jobObserver);
                    }
                  }
                  m_p->m_lastJob = job;
                  m_p->m_jobObserver = queue->observe(
                    job.get(),
                    [self, this](const smtk::job::Job&) {
                      if (!self)
                      {
                        return;
                      }
                      this->updateJobControls();
                    },
                    true);
                }
              }
            }
          }
          this->updateJobControls();
        },
        0);
      op->manager()->launchers()(op->shared_from_this());
    }
    break;
  }
}

void pqJobRunnerView::updateJobControls()
{
  m_p->updateJobControls();
}

void pqJobRunnerView::createWidget()
{
  this->Widget = new QWidget(this->parentWidget());
  this->Widget->setObjectName("pqJobRunnerView");
}
