project(vtkSlicer${qt_module_name}ModuleLogic)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#

set(module_logic_name "${PROJECT_NAME}")

# The header '${module_logic_name}Export.h' will be automatically configured.
set(module_logic_export_directive "VTK_SLICER_KINECT4SLICER_MODULE_LOGIC_EXPORT")

# Additional directories to include
set(module_logic_include_directories
   ${vtkSlicerKinect4SlicerModuleMRML_SOURCE_DIR}
   ${vtkSlicerKinect4SlicerModuleMRML_BINARY_DIR}
  )

# Source files
set(module_logic_SRCS
  vtkSlicerKinect4SlicerLogic.cxx
  vtkSlicerKinect4SlicerLogic.h
  )

# Additional Target libraries
set(module_logic_target_libraries
  vtkSlicerKinect4SlicerModuleMRML
  )


SlicerMacroBuildModuleLogic(
  NAME ${module_logic_name}
  EXPORT_DIRECTIVE ${module_logic_export_directive}
  INCLUDE_DIRECTORIES ${module_logic_include_directories}
  SRCS ${module_logic_SRCS}
  TARGET_LIBRARIES ${module_logic_target_libraries}
  )
