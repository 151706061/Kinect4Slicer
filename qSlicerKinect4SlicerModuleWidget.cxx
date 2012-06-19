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

// SlicerQt includes
#include "qSlicerKinect4SlicerModuleWidget.h"
#include "ui_qSlicerKinect4SlicerModule.h"

#include <qMRMLNodeFactory.h>
#include <vtkMRMLScene.h>
#include <vtkMatrix4x4.h>
#include <vtkSphereSource.h>
#include <vtkCollection.h>
#include <vtkNew.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Kinect4Slicer
class qSlicerKinect4SlicerModuleWidgetPrivate: public Ui_qSlicerKinect4SlicerModule
{
public:
  qSlicerKinect4SlicerModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerKinect4SlicerModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerKinect4SlicerModuleWidgetPrivate::qSlicerKinect4SlicerModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerKinect4SlicerModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerKinect4SlicerModuleWidget::qSlicerKinect4SlicerModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerKinect4SlicerModuleWidgetPrivate )
{
	this->KinectSensor = NULL;
	this->TrackingRunning = false;
	this->itkThreadPointer = itk::MultiThreader::New();
	this->ThreadID = -1;
}

//-----------------------------------------------------------------------------
qSlicerKinect4SlicerModuleWidget::~qSlicerKinect4SlicerModuleWidget()
{
	if(this->KinectSensor)
	{
		this->KinectSensor->ShutdownKinect();
		this->KinectSensor->setTerminateFlag(true);
		
		if(this->itkThreadPointer && (this->ThreadID != -1))
		{
			this->itkThreadPointer->TerminateThread(this->ThreadID);
		}

		this->KinectSensor->Delete();
	}

}

//-----------------------------------------------------------------------------
void qSlicerKinect4SlicerModuleWidget::setup()
{
  Q_D(qSlicerKinect4SlicerModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  connect(d->InitializeButton, SIGNAL(clicked()),
		  this, SLOT(onInitializeButtonClicked()));

  connect(d->TrackButton, SIGNAL(clicked()),
		  this, SLOT(onTrackButtonClicked()));
}

void qSlicerKinect4SlicerModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  if (scene == NULL)
    {
    return;
    }

  scene->RegisterNodeClass(vtkNew<vtkMRMLKinectSensorNode>().GetPointer());
}

void qSlicerKinect4SlicerModuleWidget::CreateTransformationModel(vtkMRMLLinearTransformNode* transformation, double r, double g, double b)
{
	if(transformation)
	{
		vtkMRMLModelNode* model = vtkMRMLModelNode::New();
		vtkMRMLModelDisplayNode *display = vtkMRMLModelDisplayNode::New();

		vtkSphereSource* sphere = vtkSphereSource::New();
		sphere->SetRadius(5.0);
		sphere->SetCenter(0,0,0);
		sphere->Update();

		model->SetAndObservePolyData(sphere->GetOutput());
		display->SetPolyData(model->GetPolyData());
		display->SetColor(r,g,b);

		this->mrmlScene()->SaveStateForUndo();
		this->mrmlScene()->AddNode(display);
		this->mrmlScene()->AddNode(model);
		display->SetScene(this->mrmlScene());
		model->SetScene(this->mrmlScene());
		model->SetAndObserveDisplayNodeID(display->GetID());
		model->SetHideFromEditors(0);
		model->SetAndObserveTransformNodeID(transformation->GetID());

		model->Delete();
		display->Delete();
		sphere->Delete();
	}
}


void qSlicerKinect4SlicerModuleWidget::onInitializeButtonClicked()
{
	Q_D(qSlicerKinect4SlicerModuleWidget);
	
	if(this->KinectSensor == NULL)
	{
		this->KinectSensor = vtkMRMLKinectSensorNode::New();
		this->KinectSensor->SetHideFromEditors(0);
	    this->mrmlScene()->AddNode(this->KinectSensor);
	}

	if(this->KinectSensor)
	{
		int init = this->KinectSensor->DetectAndInitializeKinectSensor();
		if(init)
		{
			d->BodyPartSelector->setEnabled(true);
			d->TrackButton->setEnabled(true);

			// Start Thread
			if(this->itkThreadPointer)
			{
				this->KinectSensor->StartTracking(d->BodyPartSelector->currentIndex(), this->itkThreadPointer);
			}
		}
		else
		{
			d->BodyPartSelector->setEnabled(false);
			d->TrackButton->setEnabled(false);
		}
	}
}

void qSlicerKinect4SlicerModuleWidget::onTrackButtonClicked()
{
	Q_D(qSlicerKinect4SlicerModuleWidget);

	if(this->KinectSensor)
	{
		this->TrackingRunning = !this->TrackingRunning;
		if(this->TrackingRunning)
		{
			int PartTracked = d->BodyPartSelector->currentIndex();
			this->KinectSensor->setBodyPartTracked(PartTracked);

			vtkCollection* BodyNodes = this->KinectSensor->GetNodesTracked();
			if(BodyNodes)
			{
				if(BodyNodes->GetNumberOfItems() != 0)
					{
						for(int i = 0; i < BodyNodes->GetNumberOfItems(); i++)
						{
							BodyNodes->GetItemAsObject(i)->Delete();
						}
						BodyNodes->RemoveAllItems();
					}


				switch(PartTracked)
				{
				case vtkMRMLKinectSensorNode::HEAD: {
					vtkMRMLLinearTransformNode* itHead = vtkMRMLLinearTransformNode::New();
					vtkMatrix4x4* imHead = vtkMatrix4x4::New();
					itHead->SetAndObserveMatrixTransformToParent(imHead);
					BodyNodes->AddItem(itHead);
					itHead->SetName("Head");
					this->mrmlScene()->AddNode(itHead);
					this->CreateTransformationModel(itHead, 1.0, 0.0, 0.0);
					itHead->Delete();
					imHead->Delete();
					break;
													}

				case vtkMRMLKinectSensorNode::RIGHT_HAND: {
					vtkMRMLLinearTransformNode* itRHand = vtkMRMLLinearTransformNode::New();
					vtkMatrix4x4* imRHand = vtkMatrix4x4::New();
					itRHand->SetAndObserveMatrixTransformToParent(imRHand);
					BodyNodes->AddItem(itRHand);
					itRHand->SetName("Right Hand");
					this->mrmlScene()->AddNode(itRHand);
					this->CreateTransformationModel(itRHand, 1.0, 0.0, 0.0);
					itRHand->Delete();
					imRHand->Delete();
					break;
													}

				case vtkMRMLKinectSensorNode::LEFT_HAND: {
					vtkMRMLLinearTransformNode* itLHand = vtkMRMLLinearTransformNode::New();
					vtkMatrix4x4* imLHand = vtkMatrix4x4::New();
					itLHand->SetAndObserveMatrixTransformToParent(imLHand);
					BodyNodes->AddItem(itLHand);
					itLHand->SetName("Left Hand");
					this->mrmlScene()->AddNode(itLHand);
					this->CreateTransformationModel(itLHand, 1.0, 0.0, 0.0);
					itLHand->Delete();
					imLHand->Delete();
					break;
													}

				case vtkMRMLKinectSensorNode::BOTH_HANDS: {
					vtkMRMLLinearTransformNode* itLHand = vtkMRMLLinearTransformNode::New();
					vtkMatrix4x4* imLHand = vtkMatrix4x4::New();
					itLHand->SetAndObserveMatrixTransformToParent(imLHand);
					BodyNodes->AddItem(itLHand);
					itLHand->SetName("Left Hand");
					this->mrmlScene()->AddNode(itLHand);
					this->CreateTransformationModel(itLHand, 1.0, 0.0, 0.0);
					itLHand->Delete();
					imLHand->Delete();

					vtkMRMLLinearTransformNode* itRHand = vtkMRMLLinearTransformNode::New();
					vtkMatrix4x4* imRHand = vtkMatrix4x4::New();
					itRHand->SetAndObserveMatrixTransformToParent(imRHand);
					BodyNodes->AddItem(itRHand);
					itRHand->SetName("Right Hand");
					this->mrmlScene()->AddNode(itRHand);
					this->CreateTransformationModel(itRHand, 0.0, 1.0, 0.0);
					itRHand->Delete();
					imRHand->Delete();
					break;
													}

				case vtkMRMLKinectSensorNode::RIGHT_ARM: {
					vtkMRMLLinearTransformNode* itRHand = vtkMRMLLinearTransformNode::New();
					vtkMatrix4x4* imRHand = vtkMatrix4x4::New();
					itRHand->SetAndObserveMatrixTransformToParent(imRHand);
					BodyNodes->AddItem(itRHand);
					itRHand->SetName("Right Hand");
					this->mrmlScene()->AddNode(itRHand);
					this->CreateTransformationModel(itRHand, 1.0, 0.0, 0.0);
					itRHand->Delete();
					imRHand->Delete();

					vtkMRMLLinearTransformNode* itRElbow = vtkMRMLLinearTransformNode::New();
					vtkMatrix4x4* imRElbow = vtkMatrix4x4::New();
					itRElbow->SetAndObserveMatrixTransformToParent(imRElbow);
					BodyNodes->AddItem(itRElbow);
					itRElbow->SetName("Right Elbow");
					this->mrmlScene()->AddNode(itRElbow);
					this->CreateTransformationModel(itRElbow, 0.0, 1.0, 0.0);
					itRElbow->Delete();
					imRElbow->Delete();

					vtkMRMLLinearTransformNode* itRShoulder = vtkMRMLLinearTransformNode::New();
					vtkMatrix4x4* imRShoulder = vtkMatrix4x4::New();
					itRShoulder->SetAndObserveMatrixTransformToParent(imRShoulder);
					BodyNodes->AddItem(itRShoulder);
					itRShoulder->SetName("Right Shoulder");
					this->mrmlScene()->AddNode(itRShoulder);
					this->CreateTransformationModel(itRShoulder, 0.0, 0.0, 1.0);
					itRShoulder->Delete();
					imRShoulder->Delete();
					break;
														 }

				case vtkMRMLKinectSensorNode::LEFT_ARM: {
					vtkMRMLLinearTransformNode* itLHand = vtkMRMLLinearTransformNode::New();
					vtkMatrix4x4* imLHand = vtkMatrix4x4::New();
					itLHand->SetAndObserveMatrixTransformToParent(imLHand);
					BodyNodes->AddItem(itLHand);
					itLHand->SetName("Left Hand");
					this->mrmlScene()->AddNode(itLHand);
					this->CreateTransformationModel(itLHand, 1.0, 0.0, 0.0);
					itLHand->Delete();
					imLHand->Delete();

					vtkMRMLLinearTransformNode* itLElbow = vtkMRMLLinearTransformNode::New();
					vtkMatrix4x4* imLElbow = vtkMatrix4x4::New();
					itLElbow->SetAndObserveMatrixTransformToParent(imLElbow);
					BodyNodes->AddItem(itLElbow);
					itLElbow->SetName("Left Elbow");
					this->mrmlScene()->AddNode(itLElbow);
					this->CreateTransformationModel(itLElbow, 0.0, 1.0, 0.0);
					itLElbow->Delete();
					imLElbow->Delete();

					vtkMRMLLinearTransformNode* itLShoulder = vtkMRMLLinearTransformNode::New();
					vtkMatrix4x4* imLShoulder = vtkMatrix4x4::New();
					itLShoulder->SetAndObserveMatrixTransformToParent(imLShoulder);
					BodyNodes->AddItem(itLShoulder);
					itLShoulder->SetName("Left Shoulder");
					this->mrmlScene()->AddNode(itLShoulder);
					this->CreateTransformationModel(itLShoulder, 0.0, 0.0, 1.0);
					itLShoulder->Delete();
					imLShoulder->Delete();
					break;
														 }

				case vtkMRMLKinectSensorNode::BOTH_ARMS: {
					vtkMRMLLinearTransformNode* itRHand = vtkMRMLLinearTransformNode::New();
					vtkMatrix4x4* imRHand = vtkMatrix4x4::New();
					itRHand->SetAndObserveMatrixTransformToParent(imRHand);
					BodyNodes->AddItem(itRHand);
					itRHand->SetName("Right Hand");
					this->mrmlScene()->AddNode(itRHand);
					this->CreateTransformationModel(itRHand, 1.0, 0.0, 0.0);
					itRHand->Delete();
					imRHand->Delete();

					vtkMRMLLinearTransformNode* itRElbow = vtkMRMLLinearTransformNode::New();
					vtkMatrix4x4* imRElbow = vtkMatrix4x4::New();
					itRElbow->SetAndObserveMatrixTransformToParent(imRElbow);
					BodyNodes->AddItem(itRElbow);
					itRElbow->SetName("Right Elbow");
					this->mrmlScene()->AddNode(itRElbow);
					this->CreateTransformationModel(itRElbow, 0.0, 1.0, 0.0);
					itRElbow->Delete();
					imRElbow->Delete();

					vtkMRMLLinearTransformNode* itRShoulder = vtkMRMLLinearTransformNode::New();
					vtkMatrix4x4* imRShoulder = vtkMatrix4x4::New();
					itRShoulder->SetAndObserveMatrixTransformToParent(imRShoulder);
					BodyNodes->AddItem(itRShoulder);
					itRShoulder->SetName("Right Shoulder");
					this->mrmlScene()->AddNode(itRShoulder);
					this->CreateTransformationModel(itRShoulder, 0.0, 0.0, 1.0);
					itRShoulder->Delete();
					imRShoulder->Delete();

										vtkMRMLLinearTransformNode* itLHand = vtkMRMLLinearTransformNode::New();
					vtkMatrix4x4* imLHand = vtkMatrix4x4::New();
					itLHand->SetAndObserveMatrixTransformToParent(imLHand);
					BodyNodes->AddItem(itLHand);
					itLHand->SetName("Left Hand");
					this->mrmlScene()->AddNode(itLHand);
					this->CreateTransformationModel(itLHand, 0.0, 1.0, 1.0);
					itLHand->Delete();
					imLHand->Delete();

					vtkMRMLLinearTransformNode* itLElbow = vtkMRMLLinearTransformNode::New();
					vtkMatrix4x4* imLElbow = vtkMatrix4x4::New();
					itLElbow->SetAndObserveMatrixTransformToParent(imLElbow);
					BodyNodes->AddItem(itLElbow);
					itLElbow->SetName("Left Elbow");
					this->mrmlScene()->AddNode(itLElbow);
					this->CreateTransformationModel(itLElbow, 1.0, 1.0, 0.0);
					itLElbow->Delete();
					imLElbow->Delete();

					vtkMRMLLinearTransformNode* itLShoulder = vtkMRMLLinearTransformNode::New();
					vtkMatrix4x4* imLShoulder = vtkMatrix4x4::New();
					itLShoulder->SetAndObserveMatrixTransformToParent(imLShoulder);
					BodyNodes->AddItem(itLShoulder);
					itLShoulder->SetName("Left Shoulder");
					this->mrmlScene()->AddNode(itLShoulder);
					this->CreateTransformationModel(itLShoulder, 1.0, 0.0, 1.0);
					itLShoulder->Delete();
					imLShoulder->Delete();

					break;
														 }

				case vtkMRMLKinectSensorNode::BODY: break;
				default: break;
				}
			this->KinectSensor->setTrackingFlag(this->TrackingRunning);
			}
			else
			{
				std::cerr << "Tracking NOT Running" << std::endl;
			}
		}
	}
}