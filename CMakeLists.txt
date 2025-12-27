# CMakeList.txt: проект CMake для FemSolver; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.20)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("FemSolver")

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Установка кодировки для MSVC
if(MSVC)
  add_compile_options(/utf-8)
endif()

# Директории для заголовков
include_directories("include")

# Список исходных файлов
set(SOURCES
    "FemSolver.cpp"
    "src/FemSolver.cpp"
    "src/MeshGenerator.cpp"
    "src/FunctionParser.cpp"
    "src/EllipticFEMSolver.cpp"
    "src/Visualizer.cpp"
    "src/EllipticApp.cpp"
    "src/GUIApp.cpp"
    "src/ReportGenerator.cpp"
    "src/GdiVisualizer.cpp"
    "src/Localization.cpp"
    "src/PresetManager.cpp"
    "src/WindowEventHandler.cpp"
    "src/VisualizationManager.cpp"
    "src/SolutionManager.cpp"
    "src/ExportManager.cpp"
)

# Список заголовочных файлов
set(HEADERS
    "include/FemSolver.h"
    "include/Types.h"
    "include/MeshGenerator.h"
    "include/FunctionParser.h"
    "include/EllipticFEMSolver.h"
    "include/Visualizer.h"
    "include/EllipticApp.h"
    "include/GUIApp.h"
    "include/ReportGenerator.h"
    "include/IVisualizer.h"
    "include/GdiVisualizer.h"
    "include/StringUtils.h"
    "include/Localization.h"
    "include/PresetManager.h"
    "include/GUIAppTypes.h"
    "include/WindowEventHandler.h"
    "include/VisualizationManager.h"
    "include/SolutionManager.h"
    "include/ExportManager.h"
)

# Добавьте все исходники в исполняемый файл проекта
add_executable (FemSolver WIN32 ${SOURCES} ${HEADERS})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET FemSolver PROPERTY CXX_STANDARD 17)
endif()

# Set preprocessor definitions for Unicode support
add_definitions(-DUNICODE -D_UNICODE)

# Set Windows subsystem to prevent console window from appearing
if (WIN32)
  if(MSVC)
    set_target_properties(FemSolver PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup")
  else()
    set_target_properties(FemSolver PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
  endif()
endif()

# Link Windows libraries
if (WIN32)
  target_link_libraries(FemSolver comctl32)
endif()

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
