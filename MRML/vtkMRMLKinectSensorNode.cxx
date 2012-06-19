
// ONLY FOR WINDOWS
// Kinect SDK Should be installed 
//---------------------------------------------------

#include <vtkObjectFactory.h>
#include <vtkCommand.h>
#include <vtkNew.h>
#include "vtkMRMLKinectSensorNode.h"

vtkMRMLNodeNewMacro(vtkMRMLKinectSensorNode);

vtkMRMLKinectSensorNode::vtkMRMLKinectSensorNode()
{
	this->hr = S_OK;
	this->TrackingFlag = false;
	this->TerminateFlag = false;
	this->NodesTracked = vtkCollection::New();
}

vtkMRMLKinectSensorNode::~vtkMRMLKinectSensorNode()
{
	if(this->NodesTracked)
	{
		this->NodesTracked->RemoveAllItems();
		this->NodesTracked->Delete();
	}
}

void vtkMRMLKinectSensorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
}


//----------------------------------------------------------------------------
void vtkMRMLKinectSensorNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
void vtkMRMLKinectSensorNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLKinectSensorNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

void vtkMRMLKinectSensorNode::ProcessMRMLEvents(vtkObject* caller, unsigned long event, void *callData)
{
  Superclass::ProcessMRMLEvents(caller,event,callData);
}

void vtkMRMLKinectSensorNode::setTrackingFlag(bool tFlag)
{
	this->TrackingFlag = tFlag;
}

void vtkMRMLKinectSensorNode::setTerminateFlag(bool tFlag)
{
	this->TerminateFlag = tFlag;
}

void vtkMRMLKinectSensorNode::setBodyPartTracked(int bodyPart)
{
	this->BodyPartTracked = bodyPart;
}

vtkCollection* vtkMRMLKinectSensorNode::GetNodesTracked()
{
	if(this->NodesTracked)
	{
		return this->NodesTracked;
	}
	return NULL;
}

int vtkMRMLKinectSensorNode::DetectAndInitializeKinectSensor()
{
	// Create event to trigger when receiving data
	this->m_hNextVideoFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	
	// Initialize Kinect with Skeleton and Color
	this->hr = NuiInitialize( NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR);

	
	if( FAILED( this->hr ) )
	{
		std::cerr << "Kinect Initialization Failed. Is Kinect connected ?" << std::endl;
		return -1;
	}
	
	// Open Color Stream
	this->hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_1280x960,0,2,this->m_hNextVideoFrameEvent, &this->m_pVideoStreamHandle);

	return 1;
}

void vtkMRMLKinectSensorNode::ShutdownKinect()
{
	NuiShutdown();
}


void vtkMRMLKinectSensorNode::StartTracking(int dBodyPart, itk::MultiThreader::Pointer tThread)
{
	// Create Thread to run detection
	int BodyPartTracked = dBodyPart;
	tThread->SpawnThread(vtkMRMLKinectSensorNode::StartTrackingThread, this);


	return;
}

ITK_THREAD_RETURN_TYPE vtkMRMLKinectSensorNode::StartTrackingThread(void* pInfoStruct)
{
	// Get context
    vtkMRMLKinectSensorNode *MRMLSensorNode = (vtkMRMLKinectSensorNode*) (((itk::MultiThreader::ThreadInfoStruct *)(pInfoStruct))->UserData);
	std::cerr << "Thread Started !!!" << std::endl;

	NUI_SKELETON_FRAME SkeletonFrame;
	int SkeletonID = -1;

	while(1)
	{
		if(MRMLSensorNode->TrackingFlag)
		{	
			NuiSkeletonGetNextFrame( 0, &SkeletonFrame );
			
			// wait for event to be triggered by Kinect
			WaitForSingleObject( MRMLSensorNode->m_hNextVideoFrameEvent, INFINITE );

			// process the image
			const NUI_IMAGE_FRAME* imageFrame;
			MRMLSensorNode->hr = NuiImageStreamGetNextFrame( MRMLSensorNode->m_pVideoStreamHandle, 0, &imageFrame );		
			
			if(SkeletonID == -1)
			{
				for(int i = 0; i < NUI_SKELETON_COUNT ; i++ ) 
				{
					if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED )
					{
						SkeletonID = i;
					}
				}
			}

			if(SkeletonID != -1)
			{
				if(SkeletonFrame.SkeletonData[SkeletonID].eTrackingState == NUI_SKELETON_TRACKED)
				{
				// Smooth data to avoid jitter
				MRMLSensorNode->hr = NuiTransformSmooth(&SkeletonFrame, NULL); 
				MRMLSensorNode->GetBodyPartPosition(SkeletonFrame, SkeletonID);
				}
			}
			NuiImageStreamReleaseFrame( MRMLSensorNode->m_pVideoStreamHandle, imageFrame );
			Sleep(50);

			if(MRMLSensorNode->TerminateFlag)
			{
				return EXIT_SUCCESS;
			}	
		}	
	}
}


void vtkMRMLKinectSensorNode::GetBodyPartPosition(NUI_SKELETON_FRAME SkFrame, int SkeletonId)
{
	if(this->NodesTracked)
	{
		int numOfNodes = this->NodesTracked->GetNumberOfItems();
		switch(this->BodyPartTracked)
		{

		case HEAD:{
			if(numOfNodes == 1)
			{
				vtkMRMLLinearTransformNode* tHead = vtkMRMLLinearTransformNode::SafeDownCast(this->NodesTracked->GetItemAsObject(0));
				vtkMatrix4x4* mHead = tHead->GetMatrixTransformToParent();

				float head_x = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HEAD].x;
				float head_y = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HEAD].y;
				float head_z = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HEAD].z;
				
				// Invert Y and Z to match with Slicer orientation
				mHead->SetElement(0,3, head_x*100);
				mHead->SetElement(1,3, head_z*100);
				mHead->SetElement(2,3, head_y*100);
			}
			break;
		}

		case RIGHT_HAND: {
			if(numOfNodes == 1)
			{
				vtkMRMLLinearTransformNode* tRHand = vtkMRMLLinearTransformNode::SafeDownCast(this->NodesTracked->GetItemAsObject(0));
				vtkMatrix4x4* mRHand = tRHand->GetMatrixTransformToParent();

				float hand_r_x = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].x;
				float hand_r_y = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].y;
				float hand_r_z = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].z;
				
				// Invert Y and Z to match with Slicer orientation
				mRHand->SetElement(0,3, hand_r_x*100);
				mRHand->SetElement(1,3, hand_r_z*100);
				mRHand->SetElement(2,3, hand_r_y*100);
			}
			break;
		}

		case LEFT_HAND: {
			if(numOfNodes == 1)
			{
				vtkMRMLLinearTransformNode* tLHand = vtkMRMLLinearTransformNode::SafeDownCast(this->NodesTracked->GetItemAsObject(0));
				vtkMatrix4x4* mLHand = tLHand->GetMatrixTransformToParent();

				float hand_l_x = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].x;
				float hand_l_y = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].y;
				float hand_l_z = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].z;
				
				// Invert Y and Z to match with Slicer orientation
				mLHand->SetElement(0,3, hand_l_x*100);
				mLHand->SetElement(1,3, hand_l_z*100);
				mLHand->SetElement(2,3, hand_l_y*100);
			}
			break;
		}

		case BOTH_HANDS: {
			if(numOfNodes == 2)
			{	
				vtkMRMLLinearTransformNode* tLHand = vtkMRMLLinearTransformNode::SafeDownCast(this->NodesTracked->GetItemAsObject(0));
				vtkMatrix4x4* mLHand = tLHand->GetMatrixTransformToParent();

				float hand_l_x = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].x;
				float hand_l_y = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].y;
				float hand_l_z = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].z;
				
				// Invert Y and Z to match with Slicer orientation
				mLHand->SetElement(0,3, hand_l_x*100);
				mLHand->SetElement(1,3, hand_l_z*100);
				mLHand->SetElement(2,3, hand_l_y*100);


				vtkMRMLLinearTransformNode* tRHand = vtkMRMLLinearTransformNode::SafeDownCast(this->NodesTracked->GetItemAsObject(1));
				vtkMatrix4x4* mRHand = tRHand->GetMatrixTransformToParent();

				float hand_r_x = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].x;
				float hand_r_y = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].y;
				float hand_r_z = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].z;
				
				// Invert Y and Z to match with Slicer orientation
				mRHand->SetElement(0,3, hand_r_x*100);
				mRHand->SetElement(1,3, hand_r_z*100);
				mRHand->SetElement(2,3, hand_r_y*100);
			}
			break;
		}
		case RIGHT_ARM: {
			if(numOfNodes == 3)
			{
				vtkMRMLLinearTransformNode* tRHand = vtkMRMLLinearTransformNode::SafeDownCast(this->NodesTracked->GetItemAsObject(0));
				vtkMatrix4x4* mRHand = tRHand->GetMatrixTransformToParent();

				float hand_r_x = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].x;
				float hand_r_y = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].y;
				float hand_r_z = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].z;
				
				// Invert Y and Z to match with Slicer orientation
				mRHand->SetElement(0,3, hand_r_x*100);
				mRHand->SetElement(1,3, hand_r_z*100);
				mRHand->SetElement(2,3, hand_r_y*100);

				vtkMRMLLinearTransformNode* tRElbow = vtkMRMLLinearTransformNode::SafeDownCast(this->NodesTracked->GetItemAsObject(1));
				vtkMatrix4x4* mRElbow = tRElbow->GetMatrixTransformToParent();

				float elbow_r_x = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT].x;
				float elbow_r_y = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT].y;
				float elbow_r_z = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT].z;
				
				// Invert Y and Z to match with Slicer orientation
				mRElbow->SetElement(0,3, elbow_r_x*100);
				mRElbow->SetElement(1,3, elbow_r_z*100);
				mRElbow->SetElement(2,3, elbow_r_y*100);

				vtkMRMLLinearTransformNode* tRShoulder = vtkMRMLLinearTransformNode::SafeDownCast(this->NodesTracked->GetItemAsObject(2));
				vtkMatrix4x4* mRShoulder = tRShoulder->GetMatrixTransformToParent();

				float shoulder_r_x = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x;
				float shoulder_r_y = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_RIGHT].y;
				float shoulder_r_z = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_RIGHT].z;
				
				// Invert Y and Z to match with Slicer orientation
				mRShoulder->SetElement(0,3, shoulder_r_x*100);
				mRShoulder->SetElement(1,3, shoulder_r_z*100);
				mRShoulder->SetElement(2,3, shoulder_r_y*100);
			}
			break;
		}
		case LEFT_ARM: {
			if(numOfNodes == 3)
			{
				vtkMRMLLinearTransformNode* tLHand = vtkMRMLLinearTransformNode::SafeDownCast(this->NodesTracked->GetItemAsObject(0));
				vtkMatrix4x4* mLHand = tLHand->GetMatrixTransformToParent();

				float hand_l_x = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].x;
				float hand_l_y = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].y;
				float hand_l_z = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].z;
				
				// Invert Y and Z to match with Slicer orientation
				mLHand->SetElement(0,3, hand_l_x*100);
				mLHand->SetElement(1,3, hand_l_z*100);
				mLHand->SetElement(2,3, hand_l_y*100);

				vtkMRMLLinearTransformNode* tLElbow = vtkMRMLLinearTransformNode::SafeDownCast(this->NodesTracked->GetItemAsObject(1));
				vtkMatrix4x4* mLElbow = tLElbow->GetMatrixTransformToParent();

				float elbow_l_x = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT].x;
				float elbow_l_y = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT].y;
				float elbow_l_z = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT].z;
				
				// Invert Y and Z to match with Slicer orientation
				mLElbow->SetElement(0,3, elbow_l_x*100);
				mLElbow->SetElement(1,3, elbow_l_z*100);
				mLElbow->SetElement(2,3, elbow_l_y*100);

				vtkMRMLLinearTransformNode* tLShoulder = vtkMRMLLinearTransformNode::SafeDownCast(this->NodesTracked->GetItemAsObject(2));
				vtkMatrix4x4* mLShoulder = tLShoulder->GetMatrixTransformToParent();

				float shoulder_l_x = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_LEFT].x;
				float shoulder_l_y = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_LEFT].y;
				float shoulder_l_z = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_LEFT].z;
				
				// Invert Y and Z to match with Slicer orientation
				mLShoulder->SetElement(0,3, shoulder_l_x*100);
				mLShoulder->SetElement(1,3, shoulder_l_z*100);
				mLShoulder->SetElement(2,3, shoulder_l_y*100);
			}
			break;
		}
		case BOTH_ARMS: {
			if(numOfNodes == 6)
			{
				vtkMRMLLinearTransformNode* tRHand = vtkMRMLLinearTransformNode::SafeDownCast(this->NodesTracked->GetItemAsObject(0));
				vtkMatrix4x4* mRHand = tRHand->GetMatrixTransformToParent();

				float hand_r_x = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].x;
				float hand_r_y = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].y;
				float hand_r_z = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].z;
				
				// Invert Y and Z to match with Slicer orientation
				mRHand->SetElement(0,3, hand_r_x*100);
				mRHand->SetElement(1,3, hand_r_z*100);
				mRHand->SetElement(2,3, hand_r_y*100);

				vtkMRMLLinearTransformNode* tRElbow = vtkMRMLLinearTransformNode::SafeDownCast(this->NodesTracked->GetItemAsObject(1));
				vtkMatrix4x4* mRElbow = tRElbow->GetMatrixTransformToParent();

				float elbow_r_x = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT].x;
				float elbow_r_y = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT].y;
				float elbow_r_z = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT].z;
				
				// Invert Y and Z to match with Slicer orientation
				mRElbow->SetElement(0,3, elbow_r_x*100);
				mRElbow->SetElement(1,3, elbow_r_z*100);
				mRElbow->SetElement(2,3, elbow_r_y*100);

				vtkMRMLLinearTransformNode* tRShoulder = vtkMRMLLinearTransformNode::SafeDownCast(this->NodesTracked->GetItemAsObject(2));
				vtkMatrix4x4* mRShoulder = tRShoulder->GetMatrixTransformToParent();

				float shoulder_r_x = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x;
				float shoulder_r_y = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_RIGHT].y;
				float shoulder_r_z = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_RIGHT].z;
				
				// Invert Y and Z to match with Slicer orientation
				mRShoulder->SetElement(0,3, shoulder_r_x*100);
				mRShoulder->SetElement(1,3, shoulder_r_z*100);
				mRShoulder->SetElement(2,3, shoulder_r_y*100);

				vtkMRMLLinearTransformNode* tLHand = vtkMRMLLinearTransformNode::SafeDownCast(this->NodesTracked->GetItemAsObject(3));
				vtkMatrix4x4* mLHand = tLHand->GetMatrixTransformToParent();

				float hand_l_x = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].x;
				float hand_l_y = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].y;
				float hand_l_z = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].z;
				
				// Invert Y and Z to match with Slicer orientation
				mLHand->SetElement(0,3, hand_l_x*100);
				mLHand->SetElement(1,3, hand_l_z*100);
				mLHand->SetElement(2,3, hand_l_y*100);

				vtkMRMLLinearTransformNode* tLElbow = vtkMRMLLinearTransformNode::SafeDownCast(this->NodesTracked->GetItemAsObject(4));
				vtkMatrix4x4* mLElbow = tLElbow->GetMatrixTransformToParent();

				float elbow_l_x = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT].x;
				float elbow_l_y = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT].y;
				float elbow_l_z = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT].z;
				
				// Invert Y and Z to match with Slicer orientation
				mLElbow->SetElement(0,3, elbow_l_x*100);
				mLElbow->SetElement(1,3, elbow_l_z*100);
				mLElbow->SetElement(2,3, elbow_l_y*100);

				vtkMRMLLinearTransformNode* tLShoulder = vtkMRMLLinearTransformNode::SafeDownCast(this->NodesTracked->GetItemAsObject(5));
				vtkMatrix4x4* mLShoulder = tLShoulder->GetMatrixTransformToParent();

				float shoulder_l_x = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_LEFT].x;
				float shoulder_l_y = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_LEFT].y;
				float shoulder_l_z = SkFrame.SkeletonData[SkeletonId].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_LEFT].z;
				
				// Invert Y and Z to match with Slicer orientation
				mLShoulder->SetElement(0,3, shoulder_l_x*100);
				mLShoulder->SetElement(1,3, shoulder_l_z*100);
				mLShoulder->SetElement(2,3, shoulder_l_y*100);
			}
			break;
		}
		case BODY: break;
		default: break;
		}
	}
}