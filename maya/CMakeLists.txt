

find_package(Maya REQUIRED)

if(APPLE)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fPIC -mavx -fvisibility=hidden" )
	add_definitions(
		-DMAC_PLUGIN -DOSMac_ -DCC_GNU_
		-DOSMacOSX_ -DREQUIRE_IOSTREAM -DOSMac_MachO_
		-D_LANGUAGE_C_PLUS_PLUS
	)

	set(PLATFORM_LINK m)

	set(PLUGIN_SUFFIX ".bundle")

elseif(MSVC)
	add_definitions(-DNT_PLUGIN -D_CRT_SECURE_NO_WARNINGS)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /MT /Zi /Oi /fp:fast /O2")
	set(PLATFORM_LINK
		kernel32.lib
		user32.lib
		gdi32.lib
		winspool.lib
		comdlg32.lib
		advapi32.lib
		shell32.lib
		ole32.lib
		oleaut32.lib
		uuid.lib
		odbc32.lib
		odbccp32.lib
	)

	set( LINK_FLAGS
		"/INCREMENTAL:NO /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /DYNAMICBASE:NO /MACHINE:X64" # /debug:full
	)

	set(PLUGIN_SUFFIX ".mll")

	if( MSVC_VERSION GREATER 1800 )
		set( PLATFORM_LINK ${PLATFORM_LINK} legacy_stdio_definitions.lib )
	endif()

else()
	message( FATAL_ERROR "Unsupported OS -- aborting." )
endif()
	
include_directories( ${MAYA_INCLUDE} )
link_directories( ${MAYA_LIBS} )

set( SOURCES
	ANIMTools.cpp
	ANIMToolsUI.cpp
	AboutCommand.cpp
	Breakdown.cpp
	BreakdownCommand.cpp
	BreakdownList.cpp
	CurveCleanerCommand.cpp
	IncrementalSaveCommand.cpp
	RetimingCommand.cpp
	SetKeyCommand.cpp
	ShotMaskCommand.cpp

	ANIMToolsUI.h
	AboutCommand.h
	Breakdown.h
	BreakdownCommand.h
	BreakdownList.h
	CurveCleanerCommand.h
	IncrementalSaveCommand.h
	RetimingCommand.h
	SetKeyCommand.h
	ShotMaskCommand.h
)

add_library(tradigitools SHARED "${SOURCES}")

set_target_properties("${PROJECT_NAME}" PROPERTIES PREFIX "")
set_target_properties("${PROJECT_NAME}" PROPERTIES SUFFIX "${PLUGIN_SUFFIX}")
set_property(TARGET "${PROJECT_NAME}" APPEND PROPERTY LINK_FLAGS "${LINK_FLAGS}")
target_include_directories(${PROJECT_NAME}
	PRIVATE
		Maya::Maya
	PUBLIC
		"${CMAKE_CURRENT_BINARY_DIR}"
		"${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries("${PROJECT_NAME}"
#	OpenMaya
#	OpenMayaAnim
#	Foundation
	Maya::Maya
	${PLATFORM_LINK}
)

MAYA_PLUGIN("${PROJECT_NAME}")

install(
	TARGETS
		${PROJECT_NAME}
		${MAYA_TARGET_TYPE}
	DESTINATION
		"${PROJECT_SOURCE_DIR}/module/plug-ins"
)

install(
	FILES
		"${CMAKE_CURRENT_SOURCE_DIR}/icons/tradigitools_32x32.png"
	DESTINATION
		"${PROJECT_SOURCE_DIR}/module/icons"
)

install(
	FILES
		"${CMAKE_CURRENT_SOURCE_DIR}/scripts/cie_tradigitools.mel"
	DESTINATION
		"${PROJECT_SOURCE_DIR}/module/scripts"
)

