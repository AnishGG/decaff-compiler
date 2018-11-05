#ifndef MY_AST_HEADER
#define MY_AST_HEADER
#include <vector>
#include <string>
#include "Visitor.h"

class FieldDecl;
class VarIdentifier;
class ArrIdentifier;
class MethodDecl;
class TypeIdentifier;
class BlockStatement;
class CalloutArg;
class Location;

enum class BinOp : char {
    add, sub, mul, div, percentage, lt, gt, lte, gte, not_equal, equal, conditional_and, conditional_or
};

enum class DataType : char {
    int_literal, void_literal, bool_literal
};

enum class AssignOp : char {
    add_assign, sub_assign, assign 
};

enum class UnOp : char {
    sub, not_operator
};

class ASTNode {
    std::string empty = "";
public:
    ASTNode(){}
    std::string parseBinOp(BinOp op){
        switch(op){
            case BinOp::add: 
                return "+";
            case BinOp::sub: 
                return "-";
            case BinOp::mul: 
                return "*";
            case BinOp::div: 
                return "/";
            case BinOp::percentage: 
                return "%";
            case BinOp::not_equal: 
                return "!=";
            case BinOp::equal: 
                return "==";
            case BinOp::conditional_and: 
                return "&&";
            case BinOp::conditional_or: 
                return "||";
            case BinOp::gte: 
                return ">=";
            case BinOp::lte: 
                return "<=";
            case BinOp::gt: 
                return ">";
            case BinOp::lt: 
                return "<";
        }
        return empty;
    }
    std::string parseDataType(DataType type){
        switch(type){
            case DataType::int_literal: 
                return "int";
            case DataType::bool_literal: 
                return "bool";
            case DataType::void_literal: 
                return "void";
        }
        return empty;
    }
    std::string parseUnOp(UnOp op){
        switch(op){
            case UnOp::not_operator: 
                return "!";
            case UnOp::sub: 
                return "-";
        }
        return empty;
    }
    std::string parseAssignOp(AssignOp op){
        switch(op){
            case AssignOp::sub_assign: 
                return "-=";
            case AssignOp::assign: 
                return "=";
            case AssignOp::add_assign: 
                return "+=";
        }
        return empty;
    }
    virtual void accept(Visitor *v) = 0;
    ~ASTNode(){}
};

class ASTProgram : public ASTNode{
private:
    std::string id;
    std::vector<FieldDecl *> *field_decl_list;
    std::vector<MethodDecl *> *method_decl_list;
    void set_fdl(std::vector<FieldDecl *> *fdl){
        this->field_decl_list = fdl;
    }
    void set_mdl(std::vector<MethodDecl *> *mdl){
        this->method_decl_list = mdl;
    }
    void set_id(std::string id){
        this->id = id;
    }
public:
    ASTProgram(std::string id, std::vector<FieldDecl *> *field_decl_list, std::vector<MethodDecl *> *method_decl_list){
        set_fdl(field_decl_list);
        set_mdl(method_decl_list);
        set_id(id);
    }
    std::vector<FieldDecl *> *getFdl() {
        return this->field_decl_list;
    }
    std::vector<MethodDecl *> *getMdl() {
        return this->method_decl_list;
    }
    std::string getID(){
        return this->id;
    }
    void accept(Visitor *v) {
        v->visit(this);
    }
    ~ASTProgram(){}
};

class FieldDecl : public ASTNode{
private:
    DataType type;
    std::vector<VarIdentifier *> *var_list = NULL;
    std::vector<ArrIdentifier *> *array_list = NULL;
    void set_vil(std::vector<VarIdentifier *> *vil){
        this->var_list = vil;
    }
    void set_ail(std::vector<ArrIdentifier *> *ail){
        this->array_list = ail;
    }
    void set_type(DataType t){
        this-> type = t;
    }
public:
    FieldDecl(std::vector<ArrIdentifier *> *array_list, DataType type){
        set_type(type);
        set_ail(array_list);
    }
    FieldDecl(std::vector<VarIdentifier *> *var_list, DataType type){
        set_type(type);
        set_vil(var_list);
    }
    DataType getType(){
        return this->type;
    }
    std::vector<VarIdentifier *> *getVar_id_list(){
        return this->var_list;
    }
    std::vector<ArrIdentifier *> *getArray_id_list(){
        return this->array_list;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~FieldDecl(){}
};

class VarDecl : public ASTNode{
private:
    DataType type;
    std::vector<VarIdentifier *> *id_list;
    void set_type(DataType type){
        this->type = type;
    }
    void set_id_list(std::vector<VarIdentifier *> *il){
        this->id_list = il;
    }
public:
    VarDecl(std::vector<VarIdentifier *> *id_list, DataType type){
        set_type(type);
        set_id_list(id_list);
    }
    std::vector<VarIdentifier *> *getID_list(){
        return this->id_list;
    }
    DataType getType(){
        return this->type;
    }
    void accept(Visitor *v) {
        v->visit(this);
    }
    ~VarDecl(){}
};

class Identifier : public ASTNode {
public: 
    Identifier(){}
    virtual void accept(Visitor *v) = 0;
    ~Identifier(){}
};

class VarIdentifier : public Identifier{
private:
    std::string id;
    void set_id(std::string id){
        this->id = id;
    }
public:
    VarIdentifier(std::string id){
        set_id(id);
    }
    std::string getID(){
        return this->id;
    }
    void accept(Visitor * v) {
        v->visit(this);
    }
    ~VarIdentifier(){}
};

class ArrIdentifier : public Identifier{
private:
    int size;
    std::string id;
    void set_id(std::string id){
        this->id = id;
    }
    void set_size(int size){
        this->size = size;
    }
public:
    ArrIdentifier(std::string id, int size){
        set_size(size);
        set_id(id);
    }
    std::string getID(){
        return this->id;
    }
    int getSize(){
        return this->size;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~ArrIdentifier(){}
};

class MethodDecl : public ASTNode{
private:
    std::string id;
    DataType returnType;
    std::vector<TypeIdentifier *> *arguments;
    BlockStatement *block;
    void set_id(std::string id){
        this->id = id;
    }
    void set_rt(DataType rt){
        this->returnType = rt;
    }
    void set_ag(std::vector<TypeIdentifier *> *ag){
        this->arguments = ag;
    }
    void set_block(BlockStatement *b){
        this->block = b;
    }
public:
    MethodDecl(std::string id, DataType returnType, std::vector<TypeIdentifier *> *arguments, BlockStatement *block){
        set_id(id);
        set_rt(returnType);
        set_ag(arguments);
        set_block(block);
    }
    DataType getReturnType(){
        return this->returnType;
    }
    std::string getID(){
        return this->id;
    }
    BlockStatement *getBlock(){
        return this->block;
    }
    std::vector<TypeIdentifier *> *getArguments(){
        return this->arguments;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~MethodDecl(){}
};

class TypeIdentifier : public ASTNode{
private:
    std::string id;
    DataType type;
    void set_id(std::string id){
        this->id = id;
    }
    void set_t(DataType t){
        this->type = t;
    }
public:
    TypeIdentifier(std::string id, DataType type){
        set_id(id);
        set_t(type);
    }
    DataType getType(){
        return this->type;
    }
    std::string getID(){
        return this->id;
    }
    void accept(Visitor * v){
        v->visit(this);
    }
    ~TypeIdentifier(){}
};

class Statement : public ASTNode{
public:
    Statement(){}
    virtual void accept(Visitor *v) = 0;
    ~Statement(){}
};

class Expression : public ASTNode{
public:
    Expression(){}
    virtual void accept(Visitor *v) = 0;
    ~Expression(){}
};

class BlockStatement : public Statement{
private:
    std::vector<VarDecl *> *id_list;
    std::vector<Statement *> *stmtlist;
    void set_id_lst(std::vector<VarDecl *> *id_lst){
        this->id_list = id_lst;
    }
    void set_stmtlist(std::vector<Statement *> *stmtlst){
        this->stmtlist = stmtlst;
    }
public:
    BlockStatement(std::vector<Statement *> *stmtlist, std::vector<VarDecl *> *id_list) {
        set_id_lst(id_list);
        set_stmtlist(stmtlist);
    }
    std::vector<Statement *> *getStmtlist() {
        return this->stmtlist;
    }
    std::vector<VarDecl *> *getID_list() {
        return this->id_list;
    }
    void accept(Visitor * v) {
        v->visit(this);
    }
    ~BlockStatement(){}
};

class AssignStatement : public Statement{
private:
    AssignOp op;
    Expression *expr;
    Location *location;
    void set_op(AssignOp op){
        this->op = op;
    }
    void set_location(Location *l){
        this->location = l;
    }
    void set_expr(Expression *e){
        this->expr = e;
    }
public:
    AssignStatement(AssignOp op, Location *location, Expression *expr){
        set_op(op);
        set_location(location);
        set_expr(expr);
    }
    Expression *getExpr(){
        return this->expr;
    }
    Location *getLocation(){
        return this->location;
    }
    AssignOp getOp(){
        return this->op;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~AssignStatement(){}
};

class MethodCall : public Expression, public Statement{    // derived from 2 base classes
public:
    MethodCall(){}
    virtual void accept(Visitor *v) = 0;
    ~MethodCall(){}
};

class SmplMethod : public MethodCall{
private:
    std::string id;
    std::vector<Expression *> *arguments;
    void set_id(std::string id){
        this->id = id;
    }
    void set_ag(std::vector<Expression *> *ag){
        this->arguments = ag;
    }
public:
    SmplMethod(std::string id, std::vector<Expression *> *arguments){
        set_id(id);
        set_ag(arguments);
    }
    std::vector<Expression *> *getArguments(){
        return this->arguments;
    }
    std::string getID(){
        return this->id;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~SmplMethod(){}
};

class CalloutMethod : public MethodCall{
private:
    std::string method_name;
    std::vector<CalloutArg *> *arguments;
    void set_mname(std::string name){
        this->method_name = name;
    }
    void set_ag(std::vector<CalloutArg *> *ag){
        this->arguments = ag;
    }
public:
    CalloutMethod(std::string method_name, std::vector<CalloutArg *> *arguments){
        set_mname(method_name);
        set_ag(arguments);
    }
    std::string getMethod_name(){
        return this->method_name;
    }
    std::vector<CalloutArg *> *getArguments(){
        return this->arguments;
    }
    void accept(Visitor *v) {
        v->visit(this);
    }
    ~CalloutMethod(){}
};

class CalloutArg : public Expression{
public:
    CalloutArg(){}
    ~CalloutArg(){}
};

class StringCalloutArg : public CalloutArg{
private:
    std::string argument;
    void set_ag(std::string ag){
        this->argument = ag;
    }
public:
    StringCalloutArg(std::string argument){
        set_ag(argument);
    }
    std::string getArgument(){
        return this->argument;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~StringCalloutArg(){}
};

class ExpressionCalloutArg : public CalloutArg{
private:
    Expression *argument;
    void set_ag(Expression *ag){
        this->argument = ag;
    }
public:
    ExpressionCalloutArg(Expression *argument){
        set_ag(argument);
    }
    Expression *getArgument(){
        return this->argument;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~ExpressionCalloutArg(){}
};

class IfStatement : public Statement{
private:
    Expression *condition;
    BlockStatement *if_block;
    BlockStatement *else_block = NULL;
    void set_cond(Expression *c){
        this->condition = c;
    }
    void set_if(BlockStatement *i){
        this->if_block = i;
    }
    void set_else(BlockStatement *e){
        this->else_block = e;
    }
public:
    IfStatement(Expression *condition, BlockStatement *if_block, BlockStatement *else_block){
        set_cond(condition);
        set_if(if_block);
        set_else(else_block);
    }
    Expression *getCondition(){
        return this->condition;
    }
    BlockStatement *getIf_block(){
        return this->if_block;
    }
    BlockStatement *getElse_block(){
        return this->else_block;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~IfStatement(){}
};

class ForStatement : public Statement{
private:
    std::string id;
    Expression *init_condition;
    Expression *end_condition;
    BlockStatement *block;
    void set_init_cond(Expression *ic){
        this->init_condition = ic;
    }
    void set_end_cond(Expression *ec){
        this->end_condition = ec;
    }
    void set_block(BlockStatement *b){
        this->block = b;
    }
    void set_id(std::string id){
        this->id = id;
    }
public:
    ForStatement(Expression *init_condition, Expression *end_condition, BlockStatement *block, std::string id){
        set_init_cond(init_condition);
        set_end_cond(end_condition);
        set_block(block);
        set_id(id);
    }
    Expression *getInit_condition(){
        return this->init_condition;
    }
    Expression *getEnd_condition(){
        return this->end_condition;
    }
    BlockStatement *getBlock(){
        return this->block;
    }
    std::string getID(){
        return this->id;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~ForStatement(){}
};

class RetStatement : public Statement{
private:
    Expression *return_expr;
    void set_ret_id(Expression *ri){
        this->return_expr = ri;
    }
public:
    RetStatement(Expression *return_expr){
        set_ret_id(return_expr);
    }
    Expression *getExpr(){
        return this->return_expr;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~RetStatement(){}
};

class ConStatement : public Statement{
public:
    ConStatement(){}
    void accept(Visitor *v) {
        v->visit(this);
    }
    ~ConStatement(){}
};

class BreakStatement : public Statement{
public:
    BreakStatement(){}
    void accept(Visitor *v){}
    ~BreakStatement(){}
};

class Location : public Expression{
public:
    Location(){}
    void accept(Visitor *v) = 0;
    ~Location(){}
};

class VarLocation : public Location{
private:
    std::string id;
    void set_id(std::string id){
        this->id = id;
    }
public:
    VarLocation(std::string id){
        set_id(id);
    }
    std::string getID(){
        return this->id;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~VarLocation(){}
};

class ArrayLocation : public Location{
private:
    std::string id;
    Expression *index;
    void set_id(std::string id){
        this->id = id;
    }
    void set_idx(Expression *idx){
        this->index = idx;
    }
public:
    ArrayLocation(std::string id, Expression *index){
        set_id(id);
        set_idx(index);
    }
    std::string getID(){
        return this->id;
    }
    Expression *getIndex(){
        return this->index;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~ArrayLocation(){}
};

class LitExpression : public Expression{
public:
    LitExpression(){}
    virtual void accept(Visitor *v) = 0;
    ~LitExpression(){}
};

class IntLitExpression : public LitExpression{
private:
    int value;
    void set_val(int val){
        this->value = val;
    }
public:
    IntLitExpression(int value){
        set_val(value);
    }
    int getValue(){
        return this->value;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~IntLitExpression(){}
};

class CharacterLitExpression : public LitExpression{
private:
    char value;
    void set_val(char val){
        this->value = val;
    }
public:
    CharacterLitExpression(char value){
        set_val(value);
    }
    char getValue(){
        return this->value;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~CharacterLitExpression(){}
};

class TrueLitExpression : public LitExpression{
public:
    TrueLitExpression(){}
    bool getValue(){
        return true;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~TrueLitExpression(){}
};

class FalseLitExpression : public LitExpression{
public:
    FalseLitExpression(){}
    bool getValue(){
        return false;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~FalseLitExpression(){}
};

class BinOpExpression : public Expression{
private:
    Expression *left;
    Expression *right;
    BinOp op;
    void set_left(Expression *l){
        this->left = l;
    }
    void set_right(Expression *r){
        this->right = r;
    }
    void set_op(BinOp op){
        this->op = op;
    }
public:
    BinOpExpression(Expression *left, Expression *right, BinOp op){
        set_left(left);
        set_right(right);
        set_op(op);
    }
    BinOp getOp(){
        return this->op;
    }
    Expression *getRight(){
        return this->right;
    }
    Expression *getLeft(){
        return this->left;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~BinOpExpression(){}
};

class UnaryOpExpression : public Expression{
private:
    Expression *expr;
    UnOp op;
    void set_op(UnOp op){
        this->op = op;
    }
    void set_expr(Expression *e){
        this->expr = e;
    }
public:
    UnaryOpExpression(Expression *expr, UnOp op){
        set_op(op);
        set_expr(expr);
    }
    Expression *getExpr(){
        return this->expr;
    }
    UnOp getOp(){
        return this->op;
    }
    void accept(Visitor *v){
        v->visit(this);
    }
    ~UnaryOpExpression(){}
};
#endif

