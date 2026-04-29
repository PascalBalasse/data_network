cmake_minimum_required(VERSION 3.19)
project(data_network LANGUAGES CXX)

# Get absolute path to the project root
set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# -------------------------
# Find Qt6
# -------------------------
find_package(Qt6 6.4 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Sql
)

# -------------------------
# Find Poppler (poppler-cpp)
# -------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED poppler-cpp)

# -------------------------
# Find zlib for ZIP handling
# -------------------------
find_package(ZLIB REQUIRED)

# -------------------------
# Standard Qt setup
# -------------------------
qt_standard_project_setup()

# -------------------------
# Add executable
# -------------------------
qt_add_executable(data_network
    WIN32 MACOSX_BUNDLE
    src/main.cpp

    src/network/core/DataTable.h
    src/network/core/DataTable.cpp
    src/network/core/ExpressionEvaluator.h
    src/network/core/ExpressionEvaluator.cpp
    src/network/transformations/Transformation.h
    src/network/transformations/Transformation.cpp
    src/network/transformations/FilterTransformation.h
    src/network/transformations/FilterTransformation.cpp
    src/network/transformations/SelectColumnsTransformation.cpp
    src/network/transformations/RenameColumnsTransformation.cpp
    src/network/transformations/CalculatedColumnTransformation.cpp
    src/network/connectors/DataConnector.h
    src/network/connectors/DataConnector.cpp
    src/network/connectors/CSVConnector.h
    src/network/connectors/CSVConnector.cpp
    src/network/connectors/TXTConnector.h
    src/network/connectors/XMLConnector.h
    src/network/connectors/XMLConnector.cpp
    src/network/connectors/WebConnector.h
    src/network/connectors/WebConnector.cpp
    src/network/Network.h
    src/network/Network.cpp

    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/mainwindow.ui
    src/ui/models/DataTableModel.h
    src/ui/models/DataTableModel.cpp
    src/ui/models/TwoLineHeader.h
    src/ui/GraphView.h
    src/ui/GraphView.cpp
    src/ui/CustomShapeItem.h
    src/ui/CustomShapeItem.cpp
    src/ui/TableView.h
    src/ui/GraphView.cpp

    src/network/nodes/RuntimeNode.h
    src/network/nodes/RuntimeNode.cpp
    src/network/nodes/SourceNode.h
    src/network/nodes/SourceNode.cpp
    src/network/nodes/TransformationNode.h
    src/network/nodes/TransformationNode.cpp
    src/network/nodes/MergeNode.h
    src/network/nodes/MergeNode.cpp
    src/network/nodes/TargetNode.h
    src/network/nodes/TargetNode.cpp

    src/network/connectors/ExcelConnector.h
    src/network/connectors/ExcelConnector.cpp
    src/network/connectors/ZipReader.h
    src/network/connectors/ZipReader.cpp
    src/network/connectors/ZipWriter.h
    src/network/connectors/ZipWriter.cpp
    src/network/connectors/JSONConnector.h
    src/network/connectors/JSONConnector.cpp
    src/network/connectors/FECConnector.h
    src/network/connectors/FECConnector.cpp
    src/network/connectors/SQLConnector.h
    src/network/connectors/SQLConnector.cpp
    src/network/connectors/PDFConnector.h
    src/network/connectors/PDFConnector.cpp
    src/network/serialization/NetworkSerializer.h

    src/ui/dialogs/customcsvdialog.h
    src/ui/dialogs/customcsvdialog.cpp
    src/ui/dialogs/custompdfdialog.h
    src/ui/dialogs/custompdfdialog.cpp
    src/ui/dialogs/customfilterdialog.h
    src/ui/dialogs/customfilterdialog.cpp
    src/ui/dialogs/customselectcolumnsdialog.h
    src/ui/dialogs/customselectcolumnsdialog.cpp
    src/ui/dialogs/customexceldialog.h
    src/ui/dialogs/customexceldialog.cpp
    src/ui/dialogs/customfecdialog.h
    src/ui/dialogs/customfecdialog.cpp
    src/ui/dialogs/customsqldialog.h
    src/ui/dialogs/customsqldialog.cpp
    src/ui/dialogs/customtxtdialog.h
    src/ui/dialogs/customtxtdialog.cpp
    src/ui/dialogs/customjsondialog.h
    src/ui/dialogs/customjsondialog.cpp
    src/ui/dialogs/customxmldialog.h
    src/ui/dialogs/customxmldialog.cpp
    src/ui/dialogs/customwebdialog.h
    src/ui/dialogs/customwebdialog.cpp
    src/ui/dialogs/customcalculatedcolumndialog.h
    src/ui/dialogs/customcalculatedcolumndialog.cpp
    src/network/core/enums.h
    src/ui/TableView.cpp
)

# -------------------------
# Include Poppler headers
# -------------------------
target_include_directories(data_network PRIVATE ${POPPLER_CPP_INCLUDE_DIRS})
target_include_directories(data_network PRIVATE
    ${POPPLER_CPP_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui  # Ajoutez cette ligne
    ${ZLIB_INCLUDE_DIRS}
)
target_compile_options(data_network PRIVATE ${POPPLER_CPP_CFLAGS_OTHER})

# -------------------------
# Link libraries
# -------------------------
target_link_libraries(data_network
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt6::Network
        Qt6::Sql
        ${POPPLER_CPP_LIBRARIES}
        ZLIB::ZLIB
)

install(TARGETS data_network
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# qt_generate_deploy_app_script(
#     TARGET data_network
#     OUTPUT_SCRIPT deploy_script
#     NO_UNSUPPORTED_PLATFORM_ERROR
# )
# install(SCRIPT ${deploy_script})
