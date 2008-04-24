
// Qt
#include <QMainWindow>

// KDevelop
#include "control.h"

class QTreeView;
class QTimer;
class QListWidget;
class QLineEdit;
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
	void applyTransform();

private:
	QWidget* createSourceView();
	void createProblemView();
	QWidget* createASTViews();
	void updateProblemList();
	bool parse(const QByteArray& source, TranslationUnitAST*& ast, TokenStream*& tokenStream);

	Control m_control;
	ParseSession* m_lastParseSession;

	KTextEditor::Document* m_sourceDocument;
	KTextEditor::View* m_sourceView;
	QTreeView* m_astView;
	QTreeView* m_transformedAstView;
	QListWidget* m_parseProblemView;
	DomModel* m_astModel;
	DomModel* m_transformedAstModel;
	QLineEdit* m_transformEdit;
	QTimer* m_parseTimer;
	
};

};
