
// Qt
#include <QMainWindow>

// KDevelop
#include "control.h"

class QTreeView;
class QTimer;

class ParseSession;
class TranslationUnitAST;
class TokenStream;

class DomModel;

namespace KTextEditor
{
	class Document;
	class View;
};

namespace ASTView
{

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
	MainWindow();

private slots:
	void recreateAST();	

private:
	void createSourceView();
	void createASTView();
	bool parse(const QByteArray& source, TranslationUnitAST*& ast, TokenStream*& tokenStream);

	Control m_control;
	ParseSession* m_lastParseSession;

	KTextEditor::Document* m_sourceDocument;
	KTextEditor::View* m_sourceView;
	QTreeView* m_astView;
	DomModel* m_astModel;
	QTimer* m_parseTimer;
};

};
