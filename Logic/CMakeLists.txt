project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

set(KIT_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")

set(KIT_INCLUDE_DIRECTORIES
   ${vtkSlicerKinect4SlicerModuleMRML_SOURCE_DIR}
   ${vtkSlicerKinect4SlicerModuleMRML_BINARY_DIR}
  )

set(KIT_SRCS
  vtkSlicerKinect4SlicerLogic.cxx
  vtkSlicerKinect4SlicerLogic.h
  )

set(KIT_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleMRML
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT_MODULE_NAME}
  EXPORT_DIRECTIVE ${KIT_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${KIT_INCLUDE_DIRECTORIES}
  SRCS ${KIT_SRCS}
  TARGET_LIBRARIES ${KIT_TARGET_LIBRARIES}
  )

