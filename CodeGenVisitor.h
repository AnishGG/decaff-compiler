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
        }

};
#endif