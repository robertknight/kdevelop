
#ifndef TRANSFORMDESCRIPTIONPARSER_H
#define TRANSFORMDESCRIPTIONPARSER_H

class ASTManipulator;
class TransformDescriptionParser
{
public:

    ASTManipulator* createManipulator(const QString& description);
};

#endif // TRANSFORMDESCRIPTIONPARSER_H

