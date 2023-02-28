/*
***********************************************************************
  DEADASSIGN.C : IMPLEMENT THE DEAD CODE ELIMINATION OPTIMIZATION HERE
************************************************************************
*/

#include "deadassign.h"

int change;
refVar *last, *head;

/*
*************************************************************************************
   YOUR CODE IS TO BE FILLED IN THE GIVEN TODO BLANKS. YOU CAN CHOOSE TO USE ALL
   UTILITY FUNCTIONS OR NONE. YOU CAN ADD NEW FUNCTIONS. BUT DO NOT FORGET TO
   DECLARE THEM IN THE HEADER FILE.
**************************************************************************************
*/

/*
***********************************************************************
  FUNCTION TO INITIALIZE HEAD AND LAST POINTERS FOR THE REFERENCED 
  VARIABLE LIST.
************************************************************************
*/

void init()
{ 
    head = NULL;
    last = head;
}

/*
***********************************************************************
  FUNCTION TO FREE THE REFERENCED VARIABLE LIST
************************************************************************
*/

void FreeList()
{
   refVar* tmp;
   while (head != NULL)
    {
       tmp = head;
       head = head->next;
       free(tmp);
    }

}

/*
***********************************************************************
  FUNCTION TO IDENTIFY IF A VARIABLE'S REFERENCE IS ALREADY TRACKED
************************************************************************
*/
bool VarExists(char* name) {
   refVar *node;
   node = head;
   while(node != NULL) {
       if(!strcmp(name, node->name)) {
           return true;
       }
       node = node->next;
    }
    return false;
}

/*
***********************************************************************
  FUNCTION TO ADD A REFERENCE TO THE REFERENCE LIST
************************************************************************
*/
void UpdateRefVarList(char* name) {
    refVar* node = malloc(sizeof(refVar));
    if (node == NULL) return;
    node->name = name;
    node->next = NULL;
    if(head == NULL) {
        last = node;
        head = node;
    }
    else {
        last->next = node;
        last = node;
    }
}

/*
****************************************************************************
  FUNCTION TO PRINT OUT THE LIST TO SEE ALL VARIABLES THAT ARE USED/REFERRED
  AFTER THEIR ASSIGNMENT. YOU CAN USE THIS FOR DEBUGGING PURPOSES OR TO CHECK
  IF YOUR LIST IS GETTING UPDATED CORRECTLY
******************************************************************************
*/
void PrintRefVarList() {
    refVar *node;
    node = head;
    if(node==NULL) {
        printf("\nList is empty"); 
        return;
    }
    while(node != NULL) {
        printf("\t %s", node->name);
        node = node->next;
    }
}

/*
***********************************************************************
  FUNCTION TO UPDATE THE REFERENCE LIST WHEN A VARIABLE IS REFERENCED 
  IF NOT DONE SO ALREADY.
************************************************************************
*/
void UpdateRef(Node* node) {
      if(node->right != NULL && node->right->exprCode == VARIABLE) {
          if(!VarExists(node->right->name)) {
              UpdateRefVarList(node->right->name);
          }
      }
      if(node->left != NULL && node->left->exprCode == VARIABLE) {
          if(!VarExists(node->left->name)) {
              UpdateRefVarList(node->left->name);
          }
      }
}

/*
**********************************************************************************************************************************
 YOU CAN MAKE CHANGES/ADD AUXILLIARY FUNCTIONS BELOW THIS LINE
**********************************************************************************************************************************
*/


/*
********************************************************************
  THIS FUNCTION IS MEANT TO TRACK THE REFERENCES OF EACH VARIABLE
  TO HELP DETERMINE IF IT WAS USED OR NOT LATER
********************************************************************
*/

void TrackRef(Node* funcNode) {
     NodeList* statements = funcNode->statements;
     Node *node;

     while(statements != NULL) {
        /*****************************************
              TODO : YOUR CODE HERE
        *****************************************/       
      
      node = statements->node;
       //loop thru, if var on right, add to ref
      if (node->type == STATEMENT)
      {
        if (node->stmtCode == ASSIGN)
        {
          if (node->right->exprCode == OPERATION && node->right->opCode != FUNCTIONCALL)
            UpdateRef(node->right);
            
          else if(node->right->exprCode == VARIABLE){
            if (!VarExists(node->right->name))
              UpdateRefVarList(node->right->name);
          }

          else if (node->right->opCode == FUNCTIONCALL){
            NodeList *args = node->right->arguments;
            while(args != NULL)
            {
              if (args->node->exprCode == VARIABLE && !VarExists(args->node->name))
                UpdateRefVarList(args->node->name);
               
              args = args->next;
            }
          }
        }

        else if (node->stmtCode == RETURN){
          if(node->left->exprCode == VARIABLE)
            if (!VarExists(node->left->name))
              UpdateRefVarList(node->left->name);
        }
      }

	  statements = statements->next;
    }
}

/*
***************************************************************
  THIS FUNCTION IS MEANT TO DO THE ACTUAL DEADCODE REMOVAL
  BASED ON THE INFORMATION OF TRACKED REFERENCES
****************************************************************
*/
NodeList* RemoveDead(NodeList* statements) {
    refVar* varNode;
    NodeList *prev, *tmp, *first;
        /*
         ****************************************
              TODO : YOUR CODE HERE
         ****************************************
        */
     
    prev = NULL; tmp = NULL; first = statements;

    while(statements != NULL) {
      /*
        ****************************************
            TODO : YOUR CODE HERE
        ****************************************
      */
     
      if (statements->node->type == STATEMENT && statements->node->stmtCode == RETURN)
      {
        prev = statements;
        statements = statements->next;
      }
        
      else if (statements->node->type == STATEMENT && statements->node->stmtCode == ASSIGN)
      {
        if (!VarExists(statements->node->name))
        {
          if (prev == NULL) //if first node needs to be removed
          {
            tmp = first;
            first = statements->next;
            statements = statements->next;
            
            FreeStatement(tmp->node);
            free(tmp);

            change = 1;
          }

          else
          {
            tmp = statements;
            prev->next = statements->next;
            statements = statements->next;

            FreeStatement(tmp->node);
            free(tmp);
            change = 1;
          }
        }

        else
        {
          prev = statements;
          statements = statements->next;
        }  
      }

      else
      {
        prev = statements;
        statements = statements->next;
      }
  }

   return first;
}

/*
********************************************************************
  THIS FUNCTION SHOULD ENSURE THAT THE DEAD CODE REMOVAL PROCESS
  OCCURS CORRECTLY FOR ALL THE FUNCTIONS IN THE PROGRAM
********************************************************************
*/
bool DeadAssign(NodeList* worklist) {
   bool madeChange = false; change = 0;
   while(worklist != NULL) {
            /*
             ****************************************
              TODO : YOUR CODE HERE
             ****************************************
             */
        TrackRef(worklist->node);
        worklist->node->statements = RemoveDead(worklist->node->statements);
        FreeList();

        worklist = worklist->next;
    }

    
    if(change==1) madeChange=true;
    return madeChange;
}

/*
**********************************************************************************************************************************
 YOU CAN MAKE CHANGES/ADD AUXILLIARY FUNCTIONS ABOVE THIS LINE
**********************************************************************************************************************************
*/
 
