// MultiConsumerMultiProducer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CNode.h"

int main()
{
   int* myVal1 = new int(5);


   CNode<int>* pHead = new CNode<int>(myVal1);



   CNode<int>* tmp = pHead;
   pHead = pHead->m_pNext;
   delete tmp->m_pValue;
   delete tmp;
   return 0;
}

