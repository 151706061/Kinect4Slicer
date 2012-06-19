/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QtPlugin>

// Kinect4Slicer Logic includes
#include <vtkSlicerKinect4SlicerLogic.h>

// Kinect4Slicer includes
#include "qSlicerKinect4SlicerModule.h"
#include "qSlicerKinect4SlicerModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerKinect4SlicerModule, qSlicerKinect4SlicerModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Kinect4Slicer
class qSlicerKinect4SlicerModulePrivate
{
public:
  qSlicerKinect4SlicerModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerKinect4SlicerModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerKinect4SlicerModulePrivate::qSlicerKinect4SlicerModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerKinect4SlicerModule methods

//-----------------------------------------------------------------------------
qSlicerKinect4SlicerModule::qSlicerKinect4SlicerModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerKinect4SlicerModulePrivate)
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerKinect4SlicerModule::categories()const
{
  return QStringList() << "Developer Tools";
}

//-----------------------------------------------------------------------------
qSlicerKinect4SlicerModule::~qSlicerKinect4SlicerModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerKinect4SlicerModule::helpText()const
{
  QString help = 
    "This template module is meant to be used with the"
    "with the ModuleWizard.py script distributed with the"
    "Slicer source code (starting with version 4)."
    "Developers can generate their own source code using the"
    "wizard and then customize it to fit their needs.";
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerKinect4SlicerModule::acknowledgementText()const
{
  return "This work was supported by NAMIC, NAC, and the Slicer Community...";
}

//-----------------------------------------------------------------------------
QStringList qSlicerKinect4SlicerModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (Organization)");
  // moduleContributors << QString("Richard Roe (Organization2)");
  // ...
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerKinect4SlicerModule::icon()const
{
  return QIcon(":/Icons/Kinect4Slicer.png");
}

//-----------------------------------------------------------------------------
void qSlicerKinect4SlicerModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerKinect4SlicerModule::createWidgetRepresentation()
{
  return new qSlicerKinect4SlicerModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerKinect4SlicerModule::createLogic()
{
  return vtkSlicerKinect4SlicerLogic::New();
}
