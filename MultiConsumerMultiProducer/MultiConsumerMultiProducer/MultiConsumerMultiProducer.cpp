// MultiConsumerMultiProducer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Node.h"

int main()
{
   int* myVal1 = new int(5);


   Node<int>* pHead = new Node<int>(myVal1);



   Node<int>* tmp = pHead;
   pHead = pHead->m_pNext;
   delete tmp->m_pValue;
   delete tmp;
   return 0;
}

