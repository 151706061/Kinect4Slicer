project(vtkSlicer${qt_module_name}ModuleMRML)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#


set(module_mrml_name "${PROJECT_NAME}")


FILE(TO_CMAKE_PATH "$ENV{KINECTSDK10_DIR}" KINECTSDK)
FILE(TO_CMAKE_PATH "$ENV{KINECTSDK10_DIR}lib/amd64/Kinect10.lib" KINECTLIB)


# The header '${PROJECT_NAME}Export.h' will be automatically configured.
set(module_mrml_export_directive "VTK_SLICER_KINECT4SLICER_MODULE_MRML_EXPORT")

# Additional directories to include
set(module_mrml_include_directories
  ${Slicer_Base_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/../Logic
  ${CMAKE_CURRENT_BINARY_DIR}/../Logic
  ${KINECTSDK}/inc
  )

# Source files
set(module_mrml_SRCS
    vtkMRMLKinectSensorNode.cxx
  )

# Additional Target libraries
add_library(KinectLib STATIC IMPORTED)
set_property(TARGET KinectLib PROPERTY IMPORTED_LOCATION ${KINECTLIB})

set(module_mrml_target_libraries
  ${MRML_LIBRARIES}
  SlicerBaseLogic
  KinectLib
  )

SlicerMacroBuildModuleMRML(
  NAME ${module_mrml_name}
  EXPORT_DIRECTIVE ${module_mrml_export_directive}
  INCLUDE_DIRECTORIES ${module_mrml_include_directories}
  SRCS ${module_mrml_SRCS}
  TARGET_LIBRARIES ${module_mrml_target_libraries}
  )


#target_link_libraries(vtkSlicerKinect4SlicerModuleMRML KinectLib)