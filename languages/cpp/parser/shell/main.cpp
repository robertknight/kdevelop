
// Qt
#include <QtCore/QCoreApplication>

// KDE
#include <KAboutData>
#include <KCmdLineArgs>

int main(int argc, char** argv)
{
	KAboutData about("cpprefactor",0,ki18n("Refactoring Shell"),"0.1",
        ki18n("Refactoring Shell"),KAboutData::License_GPL_V2);
	KCmdLineArgs::init(argc,argv,&about);

    QCoreApplication app(argc,argv);

    return 0;
}
