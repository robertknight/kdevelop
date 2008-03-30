
// KDE
#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>

// Local
#include "mainwindow.h"

int main(int argc, char** argv)
{
	KAboutData about("ast-view",0,ki18n("AST Viewer"),"0.1",ki18n("AST Viewer"),KAboutData::License_GPL_V2);
	KCmdLineArgs::init(argc,argv,&about);

	KApplication app;

	ASTView::MainWindow window;
	window.show();

	return app.exec();
}
