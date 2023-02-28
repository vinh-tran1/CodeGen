#include "optimizer.h"


void Optimizer(NodeList *funcdecls) {
/*
*************************************
     TODO: YOUR CODE HERE
*************************************
*/

     bool constfold = false; bool constprop = false; bool deadassign = false;

     while(funcdecls != NULL)
     {
          do
          {
               constfold = ConstantFolding(funcdecls);
               constprop = ConstProp(funcdecls);
               deadassign = DeadAssign(funcdecls);    

          }while(constfold || constprop || deadassign);

          funcdecls = funcdecls->next;
}
}
