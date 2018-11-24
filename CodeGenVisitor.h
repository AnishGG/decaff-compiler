#ifndef CODE_GENERATION
#define CODE_GENERATION

#include <llvm/IR/Verifier.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LegacyPassManagers.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/AsmParser/Parser.h> 
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Pass.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/MathExtras.h>
#include <iostream>
#include "BlockTable.h"
#include "ast.h"
#include "Visitor.h"

class CodeGenVisitor : public Visitor
{
    private:
        llvm::Module* module;
        ASTProgram* start;
        llvm::Function* mainFunction;                   // To create a function, initial function type of main is known
        std::list<BlockVariables> blockTable;           // List to store all the blocks
        void initiateModule(ASTProgram* start){         // Initiating top level container for all other LLVM IR objects
            std::string module_name = start->getID();
            llvm::LLVMContext &context = llvm::getGlobalContext();
            this->module = new llvm::Module(module_name, context);
            this->start = start;
        }
        void set_architecture(){
            this->module->setTargetTriple("x86_64-pc-linux-gnu");   // target architecture for which the code is generated
        }
    public:
        /**************************************************************/
        // These are all the block handling functions
        std::unordered_map<std::string, llvm::Value *> getLocalVariables(){
            return blockTable.front().localVariables;
        }
        void setLocalVariables(std::unordered_map<std::string, llvm::Value *> variables){
            blockTable.front().localVariables.insert(variables.begin(), variables.end());
        }
        bool lookupLocalVariables(std::string name){
            std::unordered_map<std::string, llvm::Value *> it; 
            it = getLocalVariables();
            bool ret = it.find(name) != it.end();
            return ret;
        }
        void declareLocalVariables(std::string name, llvm::Value *value){
            if(lookupLocalVariables(name) == false){
                std::pair<std::string, llvm::Value *> inp = std::pair<std::string, llvm::Value *>(name, value);
                blockTable.front().localVariables.insert(inp);
            }
            else{
                std::cerr<<"Variable "<<name<<" already declared";
                exit(0);
            }
        }
        bool lookupGlobalVariables(std::string name){
            bool ret = (returnLocalVariables(name) == NULL);
            return !ret;
        }
        llvm::Value *returnLocalVariables(std::string name){
            std::list<BlockVariables>::iterator it;
            for(it = blockTable.begin(); it != blockTable.end(); it++){
                std::unordered_map<std::string, llvm::Value *>::iterator found_or_end;
                found_or_end = it->localVariables.find(name);
                if(found_or_end != it->localVariables.end()){
                    llvm::Value *ret = found_or_end->second;
                    return ret;
                }
            }
            return NULL;
        }
        void pushBlock(llvm::BasicBlock *block){
            blockTable.push_front(BlockVariables(block));
        }
        void popBlock(){
            blockTable.pop_front();
        }
        llvm::BasicBlock *topBlock(){
            std::list<BlockVariables>::iterator it;
            for(it = blockTable.begin(); it != blockTable.end(); it++){
                if(it->block != NULL){  // some scopes might be useless
                    return it->block;
                }
            }
            return blockTable.front().block;
        }
        llvm::BasicBlock *bottomBlock(){
            return blockTable.back().block;
        }
        /*************************************************************/
        CodeGenVisitor(ASTProgram* start){
            initiateModule(start);
            set_architecture();
            llvm::FunctionType *ftype = llvm::FunctionType::get(llvm::Type::getVoidTy(llvm::getGlobalContext()), false);    // Initialising the return type of the main function i.e. main
            mainFunction = llvm::Function::Create(ftype, llvm::GlobalValue::ExternalLinkage, "main", module);
        }
        ~CodeGenVisitor(){}
        void handleBlock(llvm::BasicBlock *block){
            block = topBlock();
            popBlock();
        }
        void buildPass(llvm::BasicBlock *block){
            llvm::legacy::PassManager PM;       // Manages what all passes do we want to run on our module
            llvm::ReturnInst::Create(llvm::getGlobalContext(), block);
            llvm::verifyModule(*this->module);        // Performs various checks like to determine if our compiler is doing everything right
            PM.add(llvm::createPrintModulePass(llvm::outs()));  // To generate the IR from the ast we gave
            PM.run(*this->module);                    // Executes the scheduled pass
        }
        void codeGen() {
            llvm::BasicBlock *block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", this->mainFunction, 0);   // 0 to insert at the end of the function
            pushBlock(block);
            this->visit(start);
            handleBlock(block);
            buildPass(block);
        }
        void *visit(ASTProgram *node){
            std::vector<FieldDecl* > *lst = node->getFdl();
            if (lst != NULL){
                std::vector<FieldDecl* >::iterator it;
                for(it = lst->begin() ; it != lst->end(); it++){    // Visiting all the field declarations
                    this->visit(*it);
                }               
            }
            llvm::Function *iterator = NULL, *userMain = NULL;
            std::vector<MethodDecl *> *lst2 = node->getMdl();
            if (lst2 != NULL){
                std::vector<MethodDecl *>::iterator it2;
                for(it2 = lst2->begin() ; it2 != lst2->end(); it2++){   // Visiting all the methods in the function
                    iterator = static_cast<llvm::Function *>(this->visit(*it2));    // To convert the llvm function from void* to function* safely during compile time itselves
                    std::string m_name = (*it2)->getID();
                    if (m_name == "main" && userMain == NULL){
                        userMain = iterator;        // saving the main block
                    }
                }               
            }
            if(userMain != NULL)
                llvm::CallInst::Create(userMain, "", topBlock());   // Calling the function main with it's local variables
            else{
                std::cerr << "No main Found" <<std::endl;
                exit(0);
            }
            return NULL;
        }
        void *visit(FieldDecl * node){
            std::vector<VarIdentifier *> *var_list = node->getVar_id_list();
            if (var_list != NULL){
                std::vector<VarIdentifier *>::iterator it;
                for(it = var_list->begin() ; it != var_list->end(); it++){  // Iterating over all the variables(global)
                    this->visit(*it);            
                }   
            }
            std::vector<ArrIdentifier *> *array_list = node->getArray_id_list();
            if (array_list != NULL){
                std::vector<ArrIdentifier *>::iterator it2;
                for(it2 = array_list->begin() ; it2 != array_list->end(); it2++){   // Iterating over all the arrays(global)
                    this->visit(*it2);
                }
            }
            return NULL;
        }
        void *visit(VarDecl* node){
            std::vector<VarIdentifier *> *id_list = node->getID_list();
            if(id_list != NULL){
                std::vector<VarIdentifier *>::iterator it;
                for(it = id_list->begin(); it != id_list->end(); it++){    // Iterating over all the variables in a single variable list
                    this->visit(*it);
                }
            }
            return NULL;
        }

        void decl_variables_globally(std::string var_name, llvm::GlobalVariable *gI){   // Inserting the variable in the global scope
            declareLocalVariables(var_name, gI);
        }

        void decl_variables_locally(std::string var_name, llvm::AllocaInst *aI){        // Inserting the variable in the local scope
            declareLocalVariables(var_name, aI);
        }

        void *visit(VarIdentifier *node) {
            if(topBlock() != bottomBlock()){
                /* have to allocate memory on the stack in the function's frame*/
                llvm::AllocaInst *allocaInst = new llvm::AllocaInst(llvm::Type::getInt64Ty(llvm::getGlobalContext()), node->getID()/*name*/, topBlock()/*for which basic block*/); // creating space in stack with alloca call
                new llvm::StoreInst(llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), 0, true), allocaInst, false, topBlock());  // Storing the space in stack/memory
                decl_variables_locally(node->getID(), allocaInst);
                return allocaInst;
            }
            else{   // For global scope
                llvm::GlobalVariable *globalInteger = new llvm::GlobalVariable(*module, llvm::Type::getInt64Ty(llvm::getGlobalContext()), false/*this is not a constant*/, llvm::GlobalValue::CommonLinkage /*Linkage for normal variables*/, NULL/* No global scope before this */, node->getID());
                globalInteger->setInitializer(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(64/*NumBits*/, llvm::StringRef("0"), 10/*base 10*/)/*convert string to int*/));
                decl_variables_globally(node->getID(), globalInteger);       
                return globalInteger;
            }
        }
        void *visit(ArrIdentifier *node){
            int sz = node->getSize();
            if(sz > 1){
                /* Only doing in global scope */
                llvm::GlobalVariable* variable = new llvm::GlobalVariable(*module, llvm::ArrayType::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), node->getSize()), false, llvm::GlobalValue::CommonLinkage, NULL, node->getID());
                sz = node->getSize();
                /* Initializing space for the array */
                variable->setInitializer(llvm::ConstantAggregateZero::get(llvm::ArrayType::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), node->getSize())));
                assert(node->getSize() > 1);
                decl_variables_globally(node->getID(), variable);
                return variable;
            }
            else{
                std::cerr << "Invalid Array Size" <<std::endl;
                exit(0);
            }
        void *visit(MethodDecl *node){
            llvm::Type* x;
            std::vector<llvm::Type*> argTypes;
            std::vector<TypeIdentifier *> *args = node->getArguments();
            if(args != NULL){
                std::vector<TypeIdentifier *>::iterator it;
                for (it = args->begin(); it != args->end(); it++){
                    if((*it)->getType() == DataType::int_literal){
                        x = llvm::Type::getInt64Ty(llvm::getGlobalContext());
                    }
                    else if((*it)->getType() == DataType::void_literal){
                        x = llvm::Type::getVoidTy(llvm::getGlobalContext());
                    }
                    else if((*it)->getType() == DataType::bool_literal){
                        x = llvm::Type::getInt64Ty(llvm::getGlobalContext());
                    }
                    argTypes.push_back(x);
                }
            } 
            std::string m_name = node->getID();
            if(node->getReturnType() == DataType::int_literal)
                x = llvm::Type::getInt64Ty(llvm::getGlobalContext());
            else if(node->getReturnType() == DataType::void_literal)
                x = llvm::Type::getVoidTy(llvm::getGlobalContext());
            else if(node->getReturnType() == DataType::bool_literal)
                x = llvm::Type::getInt64Ty(llvm::getGlobalContext());
            llvm::FunctionType *ftype = llvm::FunctionType::get(x, llvm::makeArrayRef(argTypes), false); // Setting return type of the function
            llvm::Function *function = llvm::Function::Create(ftype, llvm::GlobalValue::InternalLinkage, m_name, module);        // Internal Linkage used for the functions
            std::string entry_point = "point";
            llvm::BasicBlock *block = llvm::BasicBlock::Create(llvm::getGlobalContext(), entry_point, function, 0);             // Creating the basic block for the new method
            pushBlock(block);   // New functions block pushed
            if(args != NULL){
                llvm::Function::arg_iterator it2 = function->arg_begin();
                std::vector<TypeIdentifier *>::iterator it;
                for(it = args->begin(); it != args->end(); it++){
                    llvm::Value *arg = &(*it2);
                    std::string param_name = (*it)->getID();
                    arg->setName(param_name);
                    llvm::AllocaInst * allocaInst = new llvm::AllocaInst(llvm::Type::getInt64Ty(llvm::getGlobalContext()), (*it)->getID(), topBlock());    // Initializing space for each parameter on stack
                    bool cond = false;
                    new llvm::StoreInst(arg, allocaInst, cond, topBlock());    // saving the parameter on the stack along with the reference to the block
                    declareLocalVariables(param_name, allocaInst);
                    it2++;
                }
            }
            this->visit(node->getBlock());
            if(topBlock()->getTerminator() == NULL){    // Check if the block terminated or not
                if(node->getReturnType() != DataType::void_literal) // Put this block in the globalcontext
                    llvm::ReturnInst::Create(llvm::getGlobalContext(), llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), 0, true), topBlock());
                else 
                    llvm::ReturnInst::Create(llvm::getGlobalContext(), topBlock());
            }
            popBlock();
            return function;
        }
        void *visit(TypeIdentifier * node) {
            return NULL; 
        }
        void *visit(Statement *node){ 
            /* This will return NULL if the block is well formed */
            if (topBlock()->getTerminator() != NULL) {   // Removing the extra semicolon's, if any, present in the decaff program
                llvm::Instruction *terminator = topBlock()->getTerminator();
                terminator->eraseFromParent();
            }
            if(dynamic_cast<AssignStatement *>(node) != NULL)
                return this->visit(dynamic_cast<AssignStatement *>(node));  // conversion from base class to derived class
            else if(dynamic_cast<MethodCall *>(node) != NULL)
                return this->visit(dynamic_cast<MethodCall *>(node));       // base class to derived methodcall class
            else if(dynamic_cast<IfStatement *>(node) != NULL)
                return this->visit(dynamic_cast<IfStatement *>(node));      // base class to derived ifstatement class
            else if(dynamic_cast<ForStatement *>(node) != NULL)
                return this->visit(dynamic_cast<ForStatement *>(node));     // base class to derived forstatement class
            else if(dynamic_cast<RetStatement *>(node) != NULL)
                return this->visit(dynamic_cast<RetStatement *>(node));     // base class to derived retstatement class
            else if(dynamic_cast<ConStatement *>(node) != NULL)
                return this->visit(dynamic_cast<ConStatement *>(node));     // base class to derived continuestatement class
            else if(dynamic_cast<BreakStatement *>(node) != NULL)
                return this->visit(dynamic_cast<BreakStatement *>(node));   // base class to derived breakstatement class
            else if(dynamic_cast<BlockStatement *>(node) != NULL){
                pushBlock(NULL);    // creating an empty block for this block statement
                this->visit(dynamic_cast<BlockStatement *>(node));          // base class to derived blockStatement class
                popBlock();         // This block is done for now
                return NULL;
            }
            std::cerr << "No such statement found" <<std::endl;
            exit(0);
        }

        void *visit(Expression *node){
            if(dynamic_cast<BinOpExpression *>(node) != NULL) 
                return this->visit(dynamic_cast<BinOpExpression *>(node));  // base class to derived binoperationexpression class
            else if(dynamic_cast<LitExpression *>(node) != NULL) 
                return this->visit(dynamic_cast<LitExpression *>(node));    // base class to derived literalExpression class
            else if(dynamic_cast<MethodCall *>(node) != NULL)
                return this->visit(dynamic_cast<MethodCall *>(node));       // base class to derived method call class
            else if(dynamic_cast<UnaryOpExpression *>(node) != NULL)
                return this->visit(dynamic_cast<UnaryOpExpression *>(node));// base class to derived unaryopExpression class
            else if(dynamic_cast<Location *>(node) != NULL)
                return this->visit(dynamic_cast<Location *>(node));         // base class to derived location class
            else if(dynamic_cast<CalloutArg *>(node) != NULL)
                return this->visit(dynamic_cast<CalloutArg *>(node));
            std::cerr << "No such expression found" <<std::endl;
            exit(0);
        }
        void *visit(BlockStatement *node){
            std::vector<VarDecl *> *id_list = node->getID_list();
            std::vector<Statement *> *stmtlist = node->getStmtlist();
            if(id_list != NULL){
                std::vector<VarDecl *>::iterator it;
                for(it = id_list->begin(); it != id_list->end(); it++) {
                    this->visit(*it);
                }
            }
            if(stmtlist != NULL){
                std::vector<Statement *>::iterator it;
                for(it = stmtlist->begin(); it != stmtlist->end(); it++) {
                    this->visit(*it);
                    if(dynamic_cast<RetStatement *>(*it))  // If scope enters this part, then other instructions won't get executed 
                        break;                
                }
            }
            return NULL;
        }
        void *visit(AssignStatement *node){
            llvm::Value *location = NULL;
            Location *loc = node->getLocation();
            if(dynamic_cast<ArrayLocation *>(loc) != NULL){ // If this is array's address
                std::string arr_name = dynamic_cast<ArrayLocation *>(loc)->getID();
                if(lookupGlobalVariables(arr_name) == false){
                    std::cerr << "Variable Not Declared" <<std::endl;
                    exit(0);
                }
                std::vector<llvm::Value *> index;
                index.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(64, llvm::StringRef("0"), 10)));   // Store the value in decimal form
                Expression *idx = dynamic_cast<ArrayLocation *>(loc)->getIndex();
                llvm::Value *set_value = static_cast<llvm::Value *>(this->visit(idx));  // converting from void * to llvm::Value * 
                index.push_back(set_value);        
                llvm::Value *val = returnLocalVariables(arr_name);
                location = llvm::GetElementPtrInst::CreateInBounds(val, index, "tmp", topBlock()); // instruction to safely access the array and struct values, insert at the end of the basic block
            }
            if(dynamic_cast<VarLocation *>(loc) != NULL){
                std::string v_name = dynamic_cast<VarLocation *>(loc)->getID();
                if(lookupGlobalVariables(v_name) == false){
                    std::cerr << "Variable Not Declared" <<std::endl;
                    exit(0);
                }
                location = returnLocalVariables(v_name);
            }
            Expression *epr = node->getExpr();
            llvm::Value *expr = static_cast<llvm::Value *>(this->visit(epr));      // converting from void * to llvm::value *
            AssignOp op = node->getOp();
            if(op == AssignOp::sub_assign){
                expr = llvm::BinaryOperator::Create(llvm::Instruction::Sub, new llvm::LoadInst(location, "load", topBlock())/*current value at the address*/, expr, "tmp", topBlock());
            }
            else if(op == AssignOp::add_assign){
                expr = llvm::BinaryOperator::Create(llvm::Instruction::Add, new llvm::LoadInst(location, "load", topBlock()), expr, "tmp", topBlock());
            }
            return new llvm::StoreInst(expr, location, false, topBlock());  // storing the result
        }
        void *visit(MethodCall *node){
            if(dynamic_cast<CalloutMethod *>(node) != NULL) 
                return this->visit(dynamic_cast<CalloutMethod *>(node));
            if(dynamic_cast<SmplMethod *>(node) != NULL) 
                return this->visit(dynamic_cast<SmplMethod *>(node));
            std::cerr << "No such method call found" <<std::endl;
            exit(0);
        }
        void *visit(SmplMethod *node){
            std::vector<llvm::Value *> args;
            std::string m_name = node->getID();
            llvm::Function *function = (llvm::Function *)module->getFunction(m_name);
            if(function == NULL){
                std::cerr << "No Function defined" <<std::endl;
                exit(0);
            }
            std::vector<Expression *> *arguments = node->getArguments();
            if(!function->isVarArg()/*taking constant number of arguments*/ && arguments != NULL && (function->getArgumentList().size() != arguments->size())){
                std::cerr << "Invalid Number of Arguments" <<std::endl;
                exit(0);
            }
            if(arguments != NULL){
                std::vector<Expression *>::iterator it;
                for(it = arguments->begin(); it != arguments->end(); it++){
                    Expression *arg_expr = *it;
                    llvm::Value *evaluated_expr = static_cast<llvm::Value *>(this->visit(arg_expr));
                    args.push_back(evaluated_expr);
                }
            }

            llvm::Type *ret_type = function->getReturnType();
            if(ret_type->isVoidTy() == true)   // if the function is of type void
                return llvm::CallInst::Create(function, llvm::makeArrayRef(args), "", topBlock());    
            else                               // If there is a return type of the function
                return llvm::CallInst::Create(function, llvm::makeArrayRef(args), node->getID(), topBlock());
        }

        void *visit(CalloutMethod *node){
            std::string m_name = node->getMethod_name();
            llvm::Function *function = module->getFunction(m_name);
            std::vector<llvm::Value *> args;
            if(function == NULL){   // If the function is not defined yet throught callout
                llvm::IntegerType *t = llvm::Type::getInt64Ty(llvm::getGlobalContext());
                llvm::FunctionType *ftype = llvm::FunctionType::get(t, true);
                function = llvm::Function::Create(ftype, llvm::GlobalValue::ExternalLinkage, m_name, module);   // This function will be externally visible
            }
            std::vector<CalloutArg *> *arguments = node->getArguments();
            if(arguments != NULL){
                std::vector<CalloutArg *>::iterator it;
                for(it = arguments->begin(); it != arguments->end(); it++){
                    CalloutArg *arg_expr = *it;
                    llvm::Value *evaluated_expr = static_cast<llvm::Value *>(this->visit(arg_expr));
                    args.push_back(evaluated_expr);
                }
            }
            return llvm::CallInst::Create(function, llvm::makeArrayRef(args), m_name, topBlock());
        }
        void *visit(CalloutArg *node){ 
            if(dynamic_cast<StringCalloutArg *>(node) != NULL) 
                return this->visit(dynamic_cast<StringCalloutArg *>(node));
            if(dynamic_cast<ExpressionCalloutArg *>(node) != NULL)
                return this->visit(dynamic_cast<ExpressionCalloutArg *>(node));
            std::cerr << "No such callout argument" <<std::endl;
            exit(0);
        }
        void *visit(StringCalloutArg *node){
            std::string argument = node->getArgument();
            llvm::ArrayType *a_t = llvm::ArrayType::get(llvm::IntegerType::get(llvm::getGlobalContext(), 8/* number of bits */), argument.size() + 1);
            llvm::GlobalVariable* variable = new llvm::GlobalVariable(*module, a_t, true/*is_constant?*/, llvm::GlobalValue::InternalLinkage, NULL, "string");
            bool req = true;
            llvm::Constant *init = llvm::ConstantDataArray::getString(llvm::getGlobalContext(), argument, req);
            variable->setInitializer(init);
            return variable;
        }

        void *visit(ExpressionCalloutArg *node){
            Expression *args = node->getArgument();
            return this->visit(args);
        }

        void *visit(IfStatement *node){
            llvm::BasicBlock *entryBlock = topBlock(), *returnedBlock = NULL;
            llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "mergeBlock", entryBlock->getParent()); // return the enclosing method of this entryBlock
            llvm::Function *pr = entryBlock->getParent();
            llvm::BasicBlock *ifBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "ifBlock", pr); // return the enclosing method of this entry block
            Expression *expr = node->getCondition();
            llvm::Value *condition = static_cast<llvm::Value *>(this->visit(expr));
            llvm::Constant *argument_inside = llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), 0, true);    // creating a constant zero
            llvm::ICmpInst *comparison = new llvm::ICmpInst(*entryBlock, llvm::ICmpInst::ICMP_NE, condition, argument_inside/*signed int */, "tmp");    // comparing whether the condition is true or not

            pushBlock(ifBlock);
            BlockStatement *if_block = node->getIf_block();
            this->visit(if_block);
            returnedBlock = topBlock();
            popBlock();
            if(returnedBlock->getTerminator() == NULL){
                llvm::BranchInst::Create(mergeBlock, returnedBlock);    // (If we found a return statement)
            }
            std::unordered_map<std::string, llvm::Value *> localVariables;
            if(node->getElse_block() != NULL){
                /* only needed if there is a else block present */
                llvm::Function *par = entryBlock->getParent();
                llvm::BasicBlock * elseBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "elseBlock", par);    // return the enclosing method of the entry block
                pushBlock(elseBlock);
                BlockStatement *else_block;
                else_block = node->getElse_block();
                this->visit(else_block);
                returnedBlock = topBlock();
                popBlock();
                if(returnedBlock->getTerminator() == NULL)             // If we found a return statement in the else block
                    llvm::BranchInst::Create(mergeBlock, returnedBlock);
                llvm::BranchInst::Create(ifBlock, elseBlock, comparison, entryBlock);   // IfTrue, IfFalse, condition, insert at the end of this block
                localVariables = getLocalVariables();
            } 
            else{
                llvm::BranchInst::Create(ifBlock, mergeBlock, comparison, entryBlock);
                localVariables = getLocalVariables();
            }
            popBlock(), pushBlock(mergeBlock), setLocalVariables(localVariables);
            return NULL;
        }


};
#endif