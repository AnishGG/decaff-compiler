#ifndef VIS_HEADER
#define VIS_HEADER

class FieldDecl;
class VarDecl;
class Identifier;
class ASTProgram;
class VarIdentifier;
class ArrIdentifier;
class MethodDecl;
class TypeIdentifier;
class Statement;
class Expression;
class BlockStatement;
class ArrayLocation;
class LitExpression;
class IntLitExpression;
class CharacterLitExpression;
class TrueLitExpression;
class AssignStatement;
class MethodCall;
class SmplMethod;
class CalloutMethod;
class CalloutArg;
class StringCalloutArg;
class ExpressionCalloutArg;
class IfStatement;
class ForStatement;
class RetStatement;
class ConStatement;
class BreakStatement;
class Location;
class VarLocation;
class FalseLitExpression;
class BinOpExpression;
class UnaryOpExpression;

class Visitor{
public:
    virtual void * visit(VarIdentifier *) = 0;
    virtual void * visit(ArrIdentifier *) = 0;
    virtual void * visit(MethodDecl *) = 0;
    virtual void * visit(TypeIdentifier *) = 0;
    virtual void * visit(Statement *) = 0;
    virtual void * visit(Expression *) = 0;
    virtual void * visit(ASTProgram *) = 0;
    virtual void * visit(FieldDecl *) = 0;
    virtual void * visit(VarDecl *) = 0;
    virtual void * visit(BlockStatement *) = 0;
    virtual void * visit(AssignStatement *) = 0;
    virtual void * visit(MethodCall *) = 0;
    virtual void * visit(SmplMethod *) = 0;
    virtual void * visit(CalloutMethod *) = 0;
    virtual void * visit(CalloutArg *) = 0;
    virtual void * visit(StringCalloutArg *) = 0;
    virtual void * visit(ExpressionCalloutArg *) = 0;
    virtual void * visit(IfStatement *) = 0;
    virtual void * visit(ForStatement *) = 0;
    virtual void * visit(RetStatement *) = 0;
    virtual void * visit(ConStatement *) = 0;
    virtual void * visit(BreakStatement *) = 0;
    virtual void * visit(Location *) = 0;
    virtual void * visit(VarLocation *) = 0;
    virtual void * visit(ArrayLocation *) = 0;
    virtual void * visit(LitExpression *) = 0;
    virtual void * visit(IntLitExpression *) = 0;
    virtual void * visit(CharacterLitExpression *) = 0;
    virtual void * visit(TrueLitExpression *) = 0;
    virtual void * visit(FalseLitExpression *) = 0;
    virtual void * visit(BinOpExpression *) = 0;
    virtual void * visit(UnaryOpExpression *) = 0;
};

#endif
