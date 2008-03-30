
// Own
#include "mainwindow.h"

// Qt
#include <QBuffer>
#include <QTreeView>
#include <QHBoxLayout>
#include <QtDebug>
#include <QFile>
#include <QTimer>

// KDE
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/editorchooser.h>

// KDevelop
#include "ast.h"
#include "parser.h"
#include "parsesession.h"
#include "control.h"
#include "xmlwritervisitor.h"

// Local
#include "dommodel.h"

using namespace ASTView;

MainWindow::MainWindow()
	: m_lastParseSession(0)
{
	KTextEditor::Editor* editor = KTextEditor::EditorChooser::editor();
	m_parseTimer = new QTimer();
	m_parseTimer->setInterval(500);
	m_parseTimer->setSingleShot(true);
	connect(m_parseTimer,SIGNAL(timeout()),this,SLOT(recreateAST()));
	Q_ASSERT(editor);

	QWidget* centralWidget = new QWidget;
	setCentralWidget(centralWidget);
	setWindowTitle("AST Viewer");

	QHBoxLayout* layout = new QHBoxLayout(centralWidget);

	m_sourceDocument = editor->createDocument(this);
	m_sourceDocument->setHighlightingMode("C++");
	connect(m_sourceDocument,SIGNAL(textChanged(KTextEditor::Document*)),m_parseTimer,
		SLOT(start()));
	m_sourceView = m_sourceDocument->createView(this);
	m_sourceView->setMinimumWidth(400);

	m_astModel = new DomModel(this);
	m_astView = new QTreeView(this);
	m_astView->setModel(m_astModel);

	layout->addWidget(m_sourceView);
	layout->addWidget(m_astView);
}

void MainWindow::recreateAST()
{
	XmlWriterVisitor writer;
	TranslationUnitAST* ast;
	TokenStream* tokenStream;

	if (parse(m_sourceDocument->text().toLocal8Bit(),ast,tokenStream))
	{
		QBuffer buffer;
		buffer.open(QIODevice::ReadWrite);
		writer.write(&buffer,ast,tokenStream);
		QDomDocument xmlDoc;
		xmlDoc.setContent(buffer.data());
		
		m_astModel->setDomNode(xmlDoc);
	}
	m_astView->expandAll();
}

bool MainWindow::parse(const QByteArray& source, TranslationUnitAST*& ast,
					   TokenStream*& tokenStream)
{
	Parser parser(&m_control);
	delete m_lastParseSession;
	m_lastParseSession = new ParseSession();
	m_lastParseSession->setContentsAndGenerateLocationTable(source);

	ast = parser.parse(m_lastParseSession);
	tokenStream = m_lastParseSession->token_stream;

	return ast != 0;
}


