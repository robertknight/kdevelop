
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set( SHELL_LIBS 
${KDE4_KDECORE_LIBS} ${KDE4_KTEXTEDITOR_LIBS} kdev4cppparser kdev4cppduchain)

set ( 	shell_SRCS
		main.cpp
	)

kde4_add_executable(cpprefactor ${shell_SRCS})
target_link_libraries(cpprefactor ${SHELL_LIBS})

