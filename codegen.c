/*
***********************************************************************
  CODEGEN.C : IMPLEMENT CODE GENERATION HERE
************************************************************************

*/
#include "codegen.h"

int argCounter;
int lastUsedOffset;
char lastOffsetUsed[100];
FILE *fptr;
regInfo *regList, *regHead, *regLast;
varStoreInfo *varList, *varHead, *varLast;

//my function/global declarations//
char* argRegs[6] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"}; //holds argument registers
char* funcCalls[6] = {"", "", "", "", "", ""}; //holds assignments to function calls
int callCount; //count for array above
void InitStackSpace(Node* funcNode); //counts # of args and vars to subtract from stack pointer
void Process(Node* stmtNodeRight); //process based on opcode
bool callLookUp(char* name); //looks to see if current var is one that has value of function call
//------------------------//

/*
*************************************************************************************
   YOUR CODE IS TO BE FILLED IN THE GIVEN TODO BLANKS. YOU CAN CHOOSE TO USE ALL
   UTILITY FUNCTIONS OR NONE. YOU CAN ADD NEW FUNCTIONS. BUT DO NOT FORGET TO
   DECLARE THEM IN THE HEADER FILE.
**************************************************************************************
*/

/*
***********************************************************************
  FUNCTION TO INITIALIZE THE ASSEMBLY FILE WITH FUNCTION DETAILS
************************************************************************
*/
void InitAsm(char* funcName) {
    fprintf(fptr, "\n.globl %s", funcName);
    fprintf(fptr, "\n%s:", funcName); 

    // Init stack and base ptr
    fprintf(fptr, "\npushq %%rbp");  
    fprintf(fptr, "\nmovq %%rsp, %%rbp"); 
}

/*
***************************************************************************
   FUNCTION TO WRITE THE RETURNING CODE OF A FUNCTION IN THE ASSEMBLY FILE
****************************************************************************
*/
void RetAsm() {
    fprintf(fptr,"\npopq  %%rbp");
    fprintf(fptr, "\nretq\n");
} 

/*
***************************************************************************
  FUNCTION TO CONVERT OFFSET FROM LONG TO CHAR STRING 
****************************************************************************
*/
void LongToCharOffset() {
     lastUsedOffset = lastUsedOffset - 8;
     snprintf(lastOffsetUsed, 100,"%d", lastUsedOffset);
     strcat(lastOffsetUsed,"(%rbp)");
}

/*
***************************************************************************
  FUNCTION TO CONVERT CONSTANT VALUE TO CHAR STRING
****************************************************************************
*/
void ProcessConstant(Node* opNode) {
     char value[10];
     LongToCharOffset();
     snprintf(value, 10,"%ld", opNode->value);
     char str[100];
     snprintf(str, 100,"%d", lastUsedOffset);
     strcat(str,"(%rbp)");
     AddVarInfo("", str, opNode->value, true);
     fprintf(fptr, "\nmovq  $%s, %s", value, str);
}

/*
***************************************************************************
  FUNCTION TO SAVE VALUE IN ACCUMULATOR (RAX)
****************************************************************************
*/
void SaveValInRax(char* name) {
    char *tempReg;
    tempReg = GetNextAvailReg(true);
    if(!(strcmp(tempReg, "NoReg"))) {
        LongToCharOffset();
        fprintf(fptr, "\n movq %%rax, %s", lastOffsetUsed);
        UpdateVarInfo(name, lastOffsetUsed, INVAL, false);
        UpdateRegInfo("%rax", 1);
    }
    else {
        fprintf(fptr, "\nmovq %%rax, %s", tempReg);
        UpdateRegInfo(tempReg, 0);
        UpdateVarInfo(name, tempReg, INVAL, false);
        UpdateRegInfo("%rax", 1);
    }
}



/*
***********************************************************************
  FUNCTION TO ADD VARIABLE INFORMATION TO THE VARIABLE INFO LIST
************************************************************************
*/
void AddVarInfo(char* varName, char* location, long val, bool isConst) {
   varStoreInfo* node = malloc(sizeof(varStoreInfo));
   node->varName = varName;
   node->value = val;
   strcpy(node->location,location);
   node->isConst = isConst;
   node->next = NULL;
   node->prev = varLast;
   if(varHead==NULL) {
       varHead = node;
       varLast = node;;
       varList = node;
   } else {
       //node->prev = varLast;
       varLast->next = node;
       varLast = varLast->next;
   }
   varList = varHead;
}

/*
***********************************************************************
  FUNCTION TO FREE THE VARIABLE INFORMATION LIST
************************************************************************
*/
void FreeVarList()
{  
   varStoreInfo* tmp;
   while (varHead != NULL)
    {  
       tmp = varHead;
       varHead = varHead->next;
       free(tmp);
    }

}

/*
***********************************************************************
  FUNCTION TO LOOKUP VARIABLE INFORMATION FROM THE VARINFO LIST
************************************************************************
*/
char* LookUpVarInfo(char* name, long val) {
    varList = varLast;
    if(varList == NULL) printf("NULL varlist");
    while(varList!=NULL) {
        if(varList->isConst == true) {
            if(varList->value == val) return varList->location;
        }
        else {
            if(!strcmp(name,varList->varName)) return varList->location;
        }
        varList = varList->prev;
    }
    varList = varHead;
    return "";
}

/*
***********************************************************************
  FUNCTION TO UPDATE VARIABLE INFORMATION 
************************************************************************
*/
void UpdateVarInfo(char* varName, char* location, long val, bool isConst) {
  
   if(!(strcmp(LookUpVarInfo(varName, val), ""))) {
       AddVarInfo(varName, location, val, isConst);
   }
   else {
       varList = varHead;
       if(varList == NULL) printf("NULL varlist");
       while(varList!=NULL) {
           if(!strcmp(varList->varName,varName)) {
               varList->value = val;
               strcpy(varList->location,location);
               varList->isConst = isConst;
               break;
        }
        varList = varList->next;
       }
    }
    varList = varHead;
}

/*
***********************************************************************
  FUNCTION TO PRINT THE VARIABLE INFORMATION LIST
************************************************************************
*/
void PrintVarListInfo() {
    varList = varHead;
    if(varList == NULL) printf("NULL varlist");
    while(varList!=NULL) {
        if(!varList->isConst) {
            printf("\t %s : %s", varList->varName, varList->location);
        }
        else {
            printf("\t %ld : %s", varList->value, varList->location);
        }
        varList = varList->next;
    }
    varList = varHead;
}

/*
***********************************************************************
  FUNCTION TO ADD NEW REGISTER INFORMATION TO THE REGISTER INFO LIST
************************************************************************
*/
void AddRegInfo(char* name, int avail) {

   regInfo* node = malloc(sizeof(regInfo));
   node->regName = name;
   node->avail = avail;
   node->next = NULL; 

   if(regHead==NULL) {
       regHead = node;
       regList = node;
       regLast = node;
   } else {
       regLast->next = node;
       regLast = node;
   }
   regList = regHead;
}

/*
***********************************************************************
  FUNCTION TO FREE REGISTER INFORMATION LIST
************************************************************************
*/
void FreeRegList()
{  
   regInfo* tmp;
   while (regHead != NULL)
    {  
       tmp = regHead;
       regHead = regHead->next;
       free(tmp);
    }

}

/*
***********************************************************************
  FUNCTION TO UPDATE THE AVAILIBILITY OF REGISTERS IN THE REG INFO LIST
************************************************************************
*/
void UpdateRegInfo(char* regName, int avail) {
    while(regList!=NULL) {
        if(regName == regList->regName) {
            regList->avail = avail;
        }
        regList = regList->next;
    }
    regList = regHead;
}

/*
***********************************************************************
  FUNCTION TO RETURN THE NEXT AVAILABLE REGISTER
************************************************************************
*/
char* GetNextAvailReg(bool noAcc) {
    regList = regHead;
    if(regList == NULL) printf("NULL reglist");
    while(regList!=NULL) {
        if(regList->avail == 1) {
            if(!noAcc) return regList->regName;
            // if not rax and dont return accumulator set to true, return the other reg
            // if rax and noAcc == true, skip to next avail
            if(noAcc && strcmp(regList->regName, "%rax")) { 
                return regList->regName;
            }
        }
        regList = regList->next;
    }
    regList = regHead;
    return "NoReg";
}

/*
***********************************************************************
  FUNCTION TO DETERMINE IF ANY REGISTER APART FROM OR INCLUDING 
  THE ACCUMULATOR(RAX) IS AVAILABLE
************************************************************************
*/
int IfAvailReg(bool noAcc) {
    regList = regHead;
    if(regList == NULL) printf("NULL reglist");
    while(regList!=NULL) {
        if(regList->avail == 1) {
            // registers available
            if(!noAcc) return 1;
            if(noAcc && strcmp(regList->regName, "%rax")) {
                return 1;
            }
        }
        regList = regList->next;
    }
    regList = regHead;
    return 0;
}

/*
***********************************************************************
  FUNCTION TO DETERMINE IF A SPECIFIC REGISTER IS AVAILABLE
************************************************************************
*/
bool IsAvailReg(char* name) {
    regList = regHead;
    if(regList == NULL) printf("NULL reglist");
    while(regList!=NULL) {
        if(!strcmp(regList->regName, name)) {
           if(regList->avail == 1) {
               return true;
           } 
        }
        regList = regList->next;
    }
    regList = regHead;
    return false;
}

/*
***********************************************************************
  FUNCTION TO PRINT THE REGISTER INFORMATION
************************************************************************
*/
void PrintRegListInfo() {
    regList = regHead;
    if(regList == NULL) printf("NULL reglist");
    while(regList!=NULL) {
        printf("\t %s : %d", regList->regName, regList->avail);
        regList = regList->next;
    }
    regList = regHead;
}

/*
***********************************************************************
  FUNCTION TO CREATE THE REGISTER LIST
************************************************************************
*/
void CreateRegList() {
    // Create the initial reglist which can be used to store variables.
    // 4 general purpose registers : AX, BX, CX, DX
    // 4 special purpose : SP, BP, SI , DI. 
    // Other registers: r8, r9
    // You need to decide which registers you will add in the register list 
    // use. Can you use all of the above registers?
    /*
     ****************************************
              TODO : YOUR CODE HERE
     ***************************************
    */

   AddRegInfo("%rax", 1);
   AddRegInfo("%rbx", 1);
   AddRegInfo("%rcx", 1);
   AddRegInfo("%rdx", 1);
   AddRegInfo("%rsp", 1);
   AddRegInfo("%rbp", 1);
   AddRegInfo("%rsi", 1);
   AddRegInfo("%rdi", 1);
   AddRegInfo("%r8", 1);
   AddRegInfo("%r9", 1);
}



/*
***********************************************************************
  THIS FUNCTION IS MEANT TO PUT THE FUNCTION ARGUMENTS ON STACK
************************************************************************
*/
int PushArgOnStack(NodeList* arguments) {
    /*
     ****************************************
              TODO : YOUR CODE HERE
     ****************************************
    */    
    int arrIndex = 0;

    while(arguments!=NULL) {
    /*
        ***********************************************************************
                TODO : YOUR CODE HERE
        THINK ABOUT WHERE EACH ARGUMENT COMES FROM. EXAMPLE WHERE IS THE 
        FIRST ARGUMENT OF A FUNCTION STORED.
        ************************************************************************
        */  
        if (arguments->node->exprCode == PARAMETER)
        {
            LongToCharOffset();
            AddVarInfo(arguments->node->name, lastOffsetUsed, arguments->node->value, false);
            fprintf(fptr,"movq %s, %s\n", argRegs[arrIndex], lastOffsetUsed);
            arrIndex++;
        }
       
        arguments = arguments->next;
    }
    return argCounter;
}


/*
*************************************************************************
  THIS FUNCTION IS MEANT TO GET THE FUNCTION ARGUMENTS FROM THE  STACK
**************************************************************************
*/
void PopArgFromStack(NodeList* arguments) {
    /*
     ****************************************
              TODO : YOUR CODE HERE
     ****************************************
    */
    while(arguments!=NULL) {
    /*
     ***********************************************************************
              TODO : YOUR CODE HERE
      THINK ABOUT WHERE EACH ARGUMENT COMES FROM. EXAMPLE WHERE IS THE
      FIRST ARGUMENT OF A FUNCTION STORED AND WHERE SHOULD IT BE EXTRACTED
      FROM AND STORED TO..
     ************************************************************************
     */
        arguments = arguments->next;
    }
}

/*
 ***********************************************************************
  THIS FUNCTION IS MEANT TO PROCESS EACH CODE STATEMENT AND GENERATE 
  ASSEMBLY FOR IT. 
  TIP: YOU CAN MODULARIZE BETTER AND ADD NEW SMALLER FUNCTIONS IF YOU 
  WANT THAT CAN BE CALLED FROM HERE.
 ************************************************************************
 */  
void ProcessStatements(NodeList* statements) {
    bool funcOccured = false; bool rightFlag = false;
    callCount = 0;

    while(statements != NULL) {
        if (statements->node->stmtCode == ASSIGN)
        {
            LongToCharOffset(); //-8 for offset every time we assign something
            if (statements->node->right->exprCode == CONSTANT) //if constant, then just put the literal onto stack
                fprintf(fptr, "movq $%d, %s\n",statements->node->right->value, lastOffsetUsed);

            //else add to varlist and then add that to stack
            else if (statements->node->right->exprCode == VARIABLE || statements->node->right->exprCode == PARAMETER)
            {
                //LongToCharOffset();
                char *location = LookUpVarInfo(statements->node->right->name, INVAL);
                if (location != NULL)
                {
                    AddVarInfo(statements->node->name, lastOffsetUsed, statements->node->right->value, false);
                    fprintf(fptr, "movq %s, %%rax\n",location);
                    fprintf(fptr, "movq %rax, %s\n",lastOffsetUsed);
                }
            }
            //operation case
            else if (statements->node->right->exprCode == OPERATION)
            {
                Node *leftNode = statements->node->right->left;
                Node *rightNode = statements->node->right->right;

                AddVarInfo(statements->node->name, lastOffsetUsed, INVAL, false); //add to varlist

                if (statements->node->right->opCode == NEGATE) //negate case i.e. only left and no right = 1 register
                {
                    if (leftNode->exprCode == CONSTANT)
                        fprintf(fptr, "movq $%d, %%rax\n", leftNode->value);
                    else if (leftNode->exprCode == VARIABLE || leftNode->exprCode == PARAMETER)
                    {
                        char *location = LookUpVarInfo(leftNode->name, INVAL);
                        if (location != NULL)
                            fprintf(fptr, "movq %s, %%rax\n", location);
                    }

                    fprintf(fptr, "negq %%rax\n");  
                    fprintf(fptr, "movq %%rax, %s\n", lastOffsetUsed);

                }
                else if (statements->node->right->opCode == FUNCTIONCALL) //functioncall case
                {
                    NodeList* args = statements->node->right->arguments;
                    int argCount = 0;
                    
                    //loads arguments onto the stack
                    while (args != NULL){
                        if (args->node->exprCode == CONSTANT)
                            fprintf(fptr, "movq $%d, %s\n", args->node->value, argRegs[argCount]);
                        else if (args->node->exprCode == VARIABLE || args->node->exprCode == PARAMETER)
                        {
                            char *location = LookUpVarInfo(args->node->name, INVAL);
                            if (location != NULL){
                                fprintf(fptr, "movq %s, %s\n", location, argRegs[argCount]);
                                argCount++;
                            }
                        }
                       
                        args = args->next;
                    }

                    fprintf(fptr, "callq %s\n", statements->node->right->left->name);

                    //set arbitrary threshold of 6 function calls
                    if (callCount < 6)
                        funcCalls[callCount] = statements->node->name;
                    callCount++;
                    funcOccured = true;
                    
                }
                else
                {
                    //3 cases: (1)var + var (2) var + constant (3) constant + var
                    //left operand
                    if (leftNode->exprCode == CONSTANT)
                        fprintf(fptr, "movq $%d, %%rax\n", leftNode->value);
                    else if (leftNode->exprCode == VARIABLE || leftNode->exprCode == PARAMETER)
                    {
                        char *location = LookUpVarInfo(leftNode->name, INVAL);
                        if (location != NULL)
                        {
                            if (callLookUp(leftNode->name) && funcOccured) //if variable held a function call
                            {
                                fprintf(fptr, "movq %%rax, %s\n", location);
                                fprintf(fptr, "movq %s, %%rax\n", location);
                            }
                            else 
                            {
                                //checks to see if right side is function call, then have to declare in diff order
                                if (rightNode->exprCode == VARIABLE && callLookUp(rightNode->name))
                                {
                                     fprintf(fptr, "movq %%rax, %%rcx\n");
                                     rightFlag = true;
                                }
                                    
                                fprintf(fptr, "movq %s, %%rax\n", location);
                            }
                          funcOccured = false;    
                        }      
                    }
                    //right operand
                    if (rightNode->exprCode == CONSTANT)
                        fprintf(fptr, "movq $%d, %%rcx\n", rightNode->value);
                    else if (rightNode->exprCode == VARIABLE || rightNode->exprCode == PARAMETER)
                    {
                        char *location = LookUpVarInfo(rightNode->name, INVAL);
                        if (!rightFlag && location != NULL) //if we didn't already output the right side (from code above)
                        {
                            if (callLookUp(rightNode->name) && funcOccured) //if variable held a function call
                                fprintf(fptr, "movq %%rax, %%rcx\n");
                            else 
                                fprintf(fptr, "movq %s, %%rcx\n", location);
    
                            funcOccured = false;
                        }
                        else
                            rightFlag = false;    
                    }
                    
                    // //switch statement calling on operation
                    Process(statements->node->right);

                    // //output the movq from rax to new place on stack
                    fprintf(fptr, "movq %%rax, %s\n", lastOffsetUsed);
                }

            }     
        }

        else if (statements->node->stmtCode == RETURN)
        {
            if (statements->node->left->exprCode == CONSTANT)
                fprintf(fptr, "movq $%d, %%rax\n", statements->node->left->value);
            else if (statements->node->left->exprCode == VARIABLE || statements->node->left->exprCode == PARAMETER)
            {
                char *location = LookUpVarInfo(statements->node->left->name, INVAL);
                if (location != NULL)
                {
                    if (funcOccured == true) //if function called right before return
                        fprintf(fptr, "movq %%rax, %s\n", location);

                    fprintf(fptr, "movq %s, %%rax\n", location);  
                }  
            }
        }
        statements = statements->next;
    }

    fprintf(fptr, "addq $%d, %%rsp", argCounter*8);
}

/*
 ***********************************************************************
  THIS FUNCTION IS MEANT TO DO CODEGEN FOR ALL THE FUNCTIONS IN THE FILE
 ************************************************************************
*/
void Codegen(NodeList* worklist) {
    fptr = fopen("assembly.s", "w+");
    /*
     ****************************************
              TODO : YOUR CODE HERE
     ****************************************
    */
    if(fptr == NULL) {
        printf("\n Could not create assembly file");
        return; 
    }
    while(worklist != NULL) {
        InitAsm(worklist->node->name);
        InitStackSpace(worklist->node);
        PushArgOnStack(worklist->node->arguments);
        ProcessStatements(worklist->node->statements);
        RetAsm();
        FreeVarList();

        worklist = worklist->next; 
    }
    fclose(fptr);
}

/*
**********************************************************************************************************************************
 YOU CAN MAKE ADD AUXILLIARY FUNCTIONS BELOW THIS LINE. DO NOT FORGET TO DECLARE THEM IN THE HEADER
**********************************************************************************************************************************
*/

//Finds space needed to subtract from top of stack
void InitStackSpace(Node* funcNode)
{
    NodeList* arguments = funcNode->arguments;
    NodeList* statements = funcNode->statements;
    argCounter = 0; lastUsedOffset = 0;

    fprintf(fptr, "\n"); 
    while (arguments != NULL){
        if (arguments->node->exprCode == PARAMETER)
            argCounter++;
        arguments = arguments->next;
    }

    while (statements != NULL){
        if (statements->node->stmtCode == ASSIGN)
           argCounter++;
        statements = statements->next;
    }

    //move stack pointer down to hold all vars
    fprintf(fptr, "subq $%d, %%rsp\n", argCounter*8); 

}

void Process(Node* stmtNodeRight)
{
    switch(stmtNodeRight->opCode)
    {
        case ADD:   
            fprintf(fptr, "addq %%rcx, %%rax\n");  
            break;

        case SUBTRACT:
            fprintf(fptr, "subq %%rcx, %%rax\n");
            break;
        
        case MULTIPLY:
            fprintf(fptr, "imulq %%rcx, %%rax\n");
            break;

        case DIVIDE:
            fprintf(fptr, "cqto\n");
            fprintf(fptr, "idivq %%rcx\n");
            break;
        
        case BOR:
            fprintf(fptr, "orq %%rcx, %%rax\n");
            break;
        
        case BAND:
            fprintf(fptr, "andq %%rcx, %%rax\n");
            break;

        case BXOR:
            fprintf(fptr, "xorq %%rcx, %%rax\n");
            break;

        case BSHR:
            fprintf(fptr, "sarq %%rcx, %%rax\n");
            break;
        
        case BSHL:
            fprintf(fptr, "salq %%rcx, %%rax\n");
            break;
    }
}

bool callLookUp(char* name)
{
    for (int i = 0; i < callCount; i++)
        if (strcmp(funcCalls[i], name) == 0)
            return true;

    return false;
}

/*
**********************************************************************************************************************************
 YOU CAN MAKE ADD AUXILLIARY FUNCTIONS ABOVE THIS LINE. DO NOT FORGET TO DECLARE THEM IN THE HEADER
**********************************************************************************************************************************
*/
