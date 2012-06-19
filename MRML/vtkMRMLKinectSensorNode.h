
// ONLY FOR WINDOWS
// Kinect SDK 1.0 Should be installed 
//---------------------------------------------------

#ifndef __vtkMRMLKinectSensorNode_h
#define __vtkMRMLKinectSensorNode_h

#include <Windows.h>
#include <ObjBase.h>
#include <NuiApi.h>

//#include <NuiImageCamera.h>
//#include <NuiSkeleton.h>
#include <mmsystem.h>
#include <string>

#include "vtkSlicerKinect4SlicerModuleMRMLExport.h"

#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMatrix4x4.h>
#include <vtkCollection.h>
#include "itkMultiThreader.h"

#include <vtkObject.h>

class VTK_SLICER_KINECT4SLICER_MODULE_MRML_EXPORT vtkMRMLKinectSensorNode : public vtkMRMLNode
{
 public:

  static vtkMRMLKinectSensorNode *New();
  vtkTypeMacro(vtkMRMLKinectSensorNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();
  virtual void ReadXMLAttributes(const char** atts);
  virtual void WriteXML(ostream& of, int indent);
  virtual void Copy(vtkMRMLNode *node);
  virtual const char* GetNodeTagName() {return "KinectSensor";};
  virtual void ProcessMRMLEvents(vtkObject* caller, unsigned long event, void *callData);
 
  void setTrackingFlag(bool tFlag);
  void setTerminateFlag(bool tFlag);
  void setBodyPartTracked(int bodyPart);

  int DetectAndInitializeKinectSensor();
  void ShutdownKinect();
  void StartTracking(int dBodyPart, itk::MultiThreader::Pointer tThread);
  static ITK_THREAD_RETURN_TYPE StartTrackingThread(void* pInfoStruct);
  void GetBodyPartPosition(NUI_SKELETON_FRAME SkFrame, int SkeletonId);
  vtkCollection* GetNodesTracked();

    enum  {
    HEAD,
    RIGHT_HAND,
    LEFT_HAND,
    BOTH_HANDS,
    RIGHT_ARM,
    LEFT_ARM,
    BOTH_ARMS,
    BODY
    };

 protected:

	 vtkMRMLKinectSensorNode();
	 ~vtkMRMLKinectSensorNode();
	 vtkMRMLKinectSensorNode(const vtkMRMLKinectSensorNode&);
	 void operator=(const vtkMRMLKinectSensorNode&);

 private:

  HRESULT hr;
  int nCount;
  int  BodyPartTracked;
  bool TrackingFlag;
  bool TerminateFlag;

  bool SkeletonFound;
  bool SingleSkeleton;

  HANDLE   m_hNextDepthFrameEvent;
  HANDLE   m_hNextVideoFrameEvent;
  HANDLE   m_hNextSkeletonEvent;
  HANDLE   m_pDepthStreamHandle;
  HANDLE   m_pVideoStreamHandle;

  vtkCollection* NodesTracked;

};

#endif
