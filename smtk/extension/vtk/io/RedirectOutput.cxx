//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/io/RedirectOutput.h"
#include "smtk/io/Logger.h"

#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkOutputWindow.h"

namespace
{

// A derived version of vtkOutputWindow that pipes the five types of VTK
// messages to smtk::io::Logger.
class OutputWindow : public vtkOutputWindow
{
public:
  typedef std::tuple<std::string, unsigned int, std::string> FormattedOutput;

  static OutputWindow* New();
  vtkTypeMacro(OutputWindow, vtkOutputWindow);

  void SetLogger(smtk::io::Logger* log);
  [[nodiscard]] smtk::io::Logger* GetLogger() const;

  void DisplayText(const char* msg) override;
  void DisplayErrorText(const char* msg) override;
  void DisplayWarningText(const char* msg) override;
  void DisplayGenericWarningText(const char* msg) override;
  void DisplayDebugText(const char* msg) override;

  OutputWindow(const OutputWindow&) = delete;
  OutputWindow& operator=(const OutputWindow&) = delete;

protected:
  OutputWindow();
  ~OutputWindow() override;

  FormattedOutput ParseOutput(const char* msg) const;

  smtk::io::Logger* Log{};
};

vtkStandardNewMacro(OutputWindow);

OutputWindow::OutputWindow() = default;

OutputWindow::~OutputWindow() = default;

void OutputWindow::SetLogger(smtk::io::Logger* log)
{
  this->Log = log;
}

smtk::io::Logger* OutputWindow::GetLogger() const
{
  return this->Log;
}

void OutputWindow::DisplayText(const char* msg)
{
  this->Log->addRecord(smtk::io::Logger::Info, std::string(msg));
}

void OutputWindow::DisplayErrorText(const char* msg)
{
  FormattedOutput out = this->ParseOutput(msg);
  this->Log->addRecord(
    smtk::io::Logger::Error, std::get<2>(out), std::get<0>(out), std::get<1>(out));
}

void OutputWindow::DisplayWarningText(const char* msg)
{
  FormattedOutput out = this->ParseOutput(msg);
  this->Log->addRecord(
    smtk::io::Logger::Warning, std::get<2>(out), std::get<0>(out), std::get<1>(out));
}

void OutputWindow::DisplayGenericWarningText(const char* msg)
{
  FormattedOutput out = this->ParseOutput(msg);
  this->Log->addRecord(
    smtk::io::Logger::Warning, std::get<2>(out), std::get<0>(out), std::get<1>(out));
}

void OutputWindow::DisplayDebugText(const char* msg)
{
  FormattedOutput out = this->ParseOutput(msg);
  this->Log->addRecord(
    smtk::io::Logger::Debug, std::get<2>(out), std::get<0>(out), std::get<1>(out));
}

std::tuple<std::string, unsigned int, std::string> OutputWindow::ParseOutput(const char* msg) const
{
  // A typical VTK-style message looks like this:
  //
  // <SEVERITY>: In <FILEPATH>, line <LINENUMBER> \n
  // <CLASSNAME> (<OBJECTPOINTER>): <MESSAGE> \n\n
  //
  // We parse this message to be:
  //
  // Severity: <SEVERITY>
  // File: <FILEPATH>
  // Line: <LINENUMBER>
  // Message: <CLASSNAME> (<OBJECTPOINTER>): <MESSAGE>

  std::string input(msg);
  FormattedOutput output;

  std::size_t fileStart = input.find_first_of("In ", input.find_first_of(':'));
  if (fileStart != std::string::npos)
  {
    ++fileStart; // Skip space at end?
    std::size_t fileEnd = input.find_first_of(',', fileStart);

    std::size_t lineStart = fileEnd + 6;
    std::size_t lineEnd = input.find_first_of('\n', lineStart);

    std::size_t messageStart = lineEnd + 1;
    std::size_t messageEnd = input.size() - 2;

    try
    {
      return std::make_tuple(
        input.substr(fileStart, fileEnd - fileStart),
        std::stoi(input.substr(lineStart, lineEnd - lineStart)),
        input.substr(messageStart, messageEnd - messageStart));
    }
    catch (std::invalid_argument& stoibad)
    {
      (void)stoibad;
      // do nothing
    }
  }
  // The message does not match the usual pattern.
  // Pass it along with no file/line information.
  return std::make_tuple("", -1, input);
}
} // namespace

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace io
{

void RedirectVTKOutputTo(smtk::io::Logger& log)
{
  // Since OutputWindow is effectively stateless (it only holds a pointer to our
  // logger, and this method modifies that anyway), we construct a static
  // instance here.
  static vtkNew<OutputWindow> outputWindow;

  // If this redirection is moving the VTK I/O stream from one logger to
  // another, we remove the callback from the first logger.
  if (smtk::io::Logger* oldLoggerPtr = outputWindow->GetLogger())
  {
    oldLoggerPtr->setCallback(std::function<void()>());
  }

  // We reset the window every time this method is called. This way, if other
  // routines inject their own VTK output window, subsequent calls to this function
  // will return the focus back to our redirection.
  vtkOutputWindow::SetInstance(outputWindow.Get());

  // Set the logger.
  outputWindow->SetLogger(&log);

  // Set the logger's callback to reset the VTK I/O upon its destruction.
  log.setCallback(ResetVTKOutput);
}

void ResetVTKOutput()
{
  vtkOutputWindow::SetInstance(nullptr);
}
} // namespace io
} // namespace vtk
} // namespace extension
} // namespace smtk
