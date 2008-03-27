#include <QtTest/QtTest>

#include "ast.h"

#include "parser.h"
#include "rpp/preprocessor.h"
#include "control.h"
#include "parsesession.h"

#include "testconfig.h"

#include "xmlwritervisitor.h"

#include <QFile>

class TestXmlWriterVisitor : public QObject
{
  Q_OBJECT

  Control control;
  QFile stdOut;	
  XmlWriterVisitor writer;

public:
  TestXmlWriterVisitor()
  {
	  stdOut.open(stdout,QIODevice::WriteOnly);
  }
  ~TestXmlWriterVisitor()
  {
  }

private slots:

  void initTestCase()
  {
  }

  void testWriteSimpleFunction()
  {
    QByteArray unit(	"void func(); void func() {"
						"}"
						);
  	writeStdout(unit);  
  }
  void testWriteSimpleStruct()
  {
  		QByteArray unit(	"struct KeyValue {"
							"QString key;"
							"QString value;"
							"};"
							);
		writeStdout(unit);		  
  }
  void testWriteTemplate()
  {
	  QByteArray unit(	"class ListNode<T> {"
	  					"T 	element;"
						"T*	next;"
						"};" );
	  writeStdout(unit); 
  }
  void testWriteTemplate2()
  {
	  QByteArray unit(	"class Pair<T1,T2> {"
	  					"T1 first;"
						"T2 second;"
						"};" );
	  writeStdout(unit);
  }
  void testWriteLoop()
  {
	  QByteArray unit(	
	  					"void func() {"
	  					"int t = 42;"
	  					"while ( t > 0 )"
						"{ t--; }"
						"}" );
	  writeStdout(unit);
  }
  void testMethodCall()
  {
  	  QByteArray unit(	"void func() {"
	  					"List<int> aList;"
						"aList.append(1);"
						"aList.removeLast();"
						"List<int>* listPtr = new List<int>();"
						"listPtr->append(1);"
						"listPtr->removeLast();"
						"}" );
	  writeStdout(unit);
  }

private:
  void writeStdout(const QByteArray& unit)
  {
  	 TokenStream* stream;
	 TranslationUnitAST* ast = parse(unit,stream);
	 QVERIFY(ast != 0);
	 writer.write(&stdOut,ast,stream);
  }

  ParseSession* lastSession;

  TranslationUnitAST* parse(const QByteArray& unit,TokenStream*& stream)
  {
    Parser parser(&control);
    lastSession = new ParseSession();
    lastSession->setContentsAndGenerateLocationTable(unit);
    
	TranslationUnitAST* ast = parser.parse(lastSession);
	stream = lastSession->token_stream;

	return ast;
  }
};

#include "test_xmlwritervisitor.moc"

QTEST_MAIN(TestXmlWriterVisitor)
