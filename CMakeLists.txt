# --- MODULE AVAILABLE ON WINDOWS ONLY ---

if(WIN32)

# Module name (usually identique to the directory name)
set(qt_module_name "Kinect4Slicer")
set(qt_module_title "Kinect4Slicer")

# Build module sub libraries
add_subdirectory(Logic)
add_subdirectory(MRML)

set(qt_module_export_directive "Q_SLICER_QTMODULES_KINECT4SLICER_EXPORT")

FILE(TO_CMAKE_PATH "$ENV{KINECTSDK10_DIR}" KINECTSDK)
FILE(TO_CMAKE_PATH "$ENV{KINECTSDK10_DIR}lib/amd64/Kinect10.lib" KINECTLIB)

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_BINARY_DIR}/MRML
  ${KINECTSDK}/inc
  )

# Source files
set(qt_module_SRCS
  qSlicerKinect4SlicerModule.cxx
  qSlicerKinect4SlicerModule.h
  qSlicerKinect4SlicerModuleWidget.cxx
  qSlicerKinect4SlicerModuleWidget.h
  )

# Headers that should run through moc
set(qt_module_MOC_SRCS
  qSlicerKinect4SlicerModule.h
  qSlicerKinect4SlicerModuleWidget.h
  )

# UI files
set(qt_module_UI_SRCS
  Resources/UI/qSlicerKinect4SlicerModule.ui
  )


# Additional Target libraries

add_library(KinectLib STATIC IMPORTED)
set_property(TARGET KinectLib PROPERTY IMPORTED_LOCATION ${KINECTLIB})

set(qt_module_target_libraries
  vtkSlicerKinect4SlicerModuleLogic
  vtkSlicerKinect4SlicerModuleMRML
  )

# Resources
set(qt_module_resources
  Resources/qSlicerKinect4SlicerModule.qrc
  )

slicerMacroBuildQtModule(
  NAME ${qt_module_name}
  TITLE ${qt_module_title}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )

#target_link_libraries(qSlicerKinect4SlicerModule KinectLib)




if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()


endif()
