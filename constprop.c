/*
********************************************************************************
  CONSTPROP.C : IMPLEMENT THE DOWNSTREAM CONSTANT PROPOGATION OPTIMIZATION HERE
*********************************************************************************
*/

#include "constprop.h"

refConst *lastNode, *headNode;
/*
*************************************************************************************
   YOUR CODE IS TO BE FILLED IN THE GIVEN TODO BLANKS. YOU CAN CHOOSE TO USE ALL
   UTILITY FUNCTIONS OR NONE. YOU CAN ADD NEW FUNCTIONS. BUT DO NOT FORGET TO
   DECLARE THEM IN THE HEADER FILE.
**************************************************************************************
*/

/*
***********************************************************************
  FUNCTION TO FREE THE CONSTANTS-ASSOCIATED VARIABLES LIST
************************************************************************
*/
void FreeConstList()
{
   refConst* tmp;
   while (headNode != NULL)
    {
       tmp = headNode;
       headNode = headNode->next;
       free(tmp);
    }

}

/*
*************************************************************************
  FUNCTION TO ADD A CONSTANT VALUE AND THE ASSOCIATED VARIABLE TO THE LIST
**************************************************************************
*/
void UpdateConstList(char* name, long val) {
    refConst* node = malloc(sizeof(refConst));
    if (node == NULL) return;
    node->name = name;
    node->val = val;
    node->next = NULL;
    if(headNode == NULL) {
        lastNode = node;
        headNode = node;
    }
    else {
        lastNode->next = node;
        lastNode = node;
    }
}

/*
*****************************************************************************
  FUNCTION TO LOOKUP IF A CONSTANT ASSOCIATED VARIABLE IS ALREADY IN THE LIST
******************************************************************************
*/
refConst* LookupConstList(char* name) {
    refConst *node;
    node = headNode; 
    while(node!=NULL){
        if(!strcmp(name, node->name))
            return node;
        node = node->next;
    }
    return NULL;
}

/*
**********************************************************************************************************************************
 YOU CAN MAKE CHANGES/ADD AUXILLIARY FUNCTIONS BELOW THIS LINE
**********************************************************************************************************************************
*/


/*
************************************************************************************
  THIS FUNCTION IS MEANT TO UPDATE THE CONSTANT LIST WITH THE ASSOCIATED VARIABLE
  AND CONSTANT VALUE WHEN ONE IS SEEN. IT SHOULD ALSO PROPOGATE THE CONSTANTS WHEN 
  WHEN APPLICABLE. YOU CAN ADD A NEW FUNCTION IF YOU WISH TO MODULARIZE BETTER.
*************************************************************************************
*/
void TrackConst(NodeList* statements) {
      Node* node; refConst* temp;
      while(statements != NULL) {
        node = statements->node;
        //TODO : YOUR CODE HERE

      //adding to constant list
      if (node->type == STATEMENT && node->stmtCode == ASSIGN)
      {
          if (node->right->exprCode == CONSTANT)
          {
            temp = LookupConstList(node->name);
            if (temp == NULL) //add variable and constant to list
              UpdateConstList(node->name, node->right->value);
          }
      
          else if (node->right->exprCode == OPERATION && node->right->opCode != FUNCTIONCALL) //for operations i.e. a = b + c
          {
            if(node->right->left->exprCode == VARIABLE)
            {
              temp = LookupConstList(node->right->left->name);
              if (temp != NULL)
              {
                FreeVariable(node->right->left);
                node->right->left = CreateNumber(temp->val);
                madeChange = true;
              }
             
            }
             
            if(node->right->opCode != NEGATE && node->right->right->exprCode == VARIABLE)
            {
              temp = LookupConstList(node->right->right->name);
              if (temp != NULL)
              {
                FreeVariable(node->right->right);
                node->right->right = CreateNumber(temp->val);
                madeChange = true;
              }
              
            }
          }

          else if (node->right->exprCode == OPERATION && node->right->opCode == FUNCTIONCALL) //for functions i.e. a = b(c, d, e);
          {
            NodeList *args = node->right->arguments;
            while(args != NULL)
            {
              if (args->node->exprCode == VARIABLE)
              {
                temp = LookupConstList(args->node->name);
                if (temp != NULL)
                {
                  FreeVariable(args->node);
                  args->node = CreateNumber(temp->val);
                  madeChange = true;
                }
              }
              args = args->next;
            }
          }
      }

      else if (node->type == STATEMENT && node->stmtCode == RETURN) //for return statements i.e. return a
      {
        if(node->left->exprCode == VARIABLE)
        {
          temp = LookupConstList(node->left->name);
          if (temp != NULL)
          {
            FreeVariable(node->left);
            node->left = CreateNumber(temp->val);
            madeChange = true;
          }
        }
      }       
      statements = statements->next;
    }
}


bool ConstProp(NodeList* worklist) {
    while(worklist!=NULL){
      //TODO : YOUR CODE HERE
      
      TrackConst(worklist->node->statements);
      FreeConstList();

      worklist = worklist->next;
    }

    
    return madeChange;
}

/*
**********************************************************************************************************************************
 YOU CAN MAKE CHANGES/ADD AUXILLIARY FUNCTIONS ABOVE THIS LINE
**********************************************************************************************************************************
*/
