
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set( AST_VIEW_LIBS 
${KDE4_KDECORE_LIBS} ${KDE4_KTEXTEDITOR_LIBS} kdev4cppparser kdev4cppduchain)

set ( 	ast_view_SRCS
		main.cpp
		mainwindow.cpp		
		dommodel.cpp
	)

kde4_add_executable(ast-view ${ast_view_SRCS})
target_link_libraries(ast-view ${AST_VIEW_LIBS})

