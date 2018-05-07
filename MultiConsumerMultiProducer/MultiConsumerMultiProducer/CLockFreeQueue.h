#pragma once
#ifndef CLOCKFREEQUEUE_H
#define CLOCKFREEQUEUE_H

#include "CNode.h"

#include <iostream>
#include <chrono>
#include <thread>

template <typename T>
class CLockFreeQueue
{
public:
   CLockFreeQueue()
   {
      m_pFirst = new CNode<T>(nullptr);
      m_pLast = m_pFirst;
      m_bProducerLock = false;
      m_bConsumerLock = false;
   };

   ~CLockFreeQueue()
   {
      while (m_pFirst != nullptr)
      {
         CNode<T>* tmp = m_pFirst;
         m_pFirst = m_pFirst->m_pNext;
         delete tmp->m_pValue;
         delete tmp;
      }
   };

   void ProduceNewNode(const T& pValueToAdd)
   {
      //make a new node locally
      CNode<T>* pNewNode = new CNode<T>(new T(pValueToAdd));

      //do some fake work
      std::cout << "Starting to produce item" << std::endl;
      auto start = std::chrono::high_resolution_clock::now();
      std::this_thread::sleep_for(THREAD_SLEEP_TIME_SECONDS);

      while (m_bProducerLock.exchange(true)) { } //acquire exclusivity (essentially "obtain a critical section lock")
      //we are now in our "critical section"
      
      m_pLast->m_pNext = pNewNode;  //attach the new node to the queue
      m_pLast = pNewNode;           //the new node is now the last node
      
      //and now we leave the "critical section"
      m_bProducerLock = false; //release exclusivity ("release the lock")

      //might as well see how long it took
      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> elapsed = end - start;
      std::cout << "Item produced in " << elapsed.count() << " ms\n";
   }

   bool ConsumeHeadNode(T& result)
   {
      //start a timer
      std::cout << "Starting to handle first item" << std::endl;
      auto start = std::chrono::high_resolution_clock::now();
      


      //first, attempt to "obtain a lock"
      while (m_bConsumerLock.exchange(true)) { }  //acquire exclusivity
      //we are now in our "critical section"

      CNode<T>* pOldDummyHead = m_pFirst; //get a pointer to the empty "dummy head" node
      CNode<T>* pFirstValueNode = m_pFirst->m_pNext; //...and to its "next" node, which is the first "actual" node with a value (hopefully)

      //check if the "value" node is empty
      if (pFirstValueNode == nullptr)
      {
         //if it is empty, then we only had our "dummy head" node, so our queue was in fact empty
         
         //leave the "critical section"
         m_bConsumerLock = false;


         //might as well see how long it took
         auto end = std::chrono::high_resolution_clock::now();
         std::chrono::duration<double, std::milli> elapsed = end - start;
         std::cout << "Queue was empty! Item handled in " << elapsed.count() << " ms\n";
         return false;
      }

      //now, our "value" node actually has something in it
      T* pNodeValue = pFirstValueNode->m_pValue; //take the value out of the node
      pFirstValueNode->m_pValue = nullptr; //and delete it from the node
      m_pFirst = pFirstValueNode; //move the queue's "dummy head" pointer forward. Our "value" node will become the new "dummy head"

      //we have a local copy of the value of the first "value node" after the "dummy head"
      //we've also removed the value from the "value node", so it is now empty
      //we've also moved the head pointer to point to our now-empty "value node"
      //we've prepared our queue for processing, we can leave the "critical section"

      m_bConsumerLock = false; //release exclusivity

      //do some fake work
      std::this_thread::sleep_for(THREAD_SLEEP_TIME_SECONDS);

      result = *pNodeValue;    //copy the node's value to the output param
      delete valpNodeValue; // clean up the value
      delete pOldDummyHead; //and the old dummy

      //might as well see how long it took
      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> elapsed = end - start;
      std::cout << "Item handled in " << elapsed.count() << " ms\n";

      return true;
   }

private:

   unsigned char m_ucPadding_0[CACHE_LINE_SIZE];
   // for one consumer at a time
   CNode<T>* m_pFirst;
   unsigned char m_ucPadding_1[CACHE_LINE_SIZE - sizeof(CNode<T>*)];

   // shared among consumers
   std::atomic<bool> m_bConsumerLock;
   unsigned char m_ucPadding_2[CACHE_LINE_SIZE - sizeof(std::atomic<bool>)];

   // for one producer at a time
   CNode<T>* m_pLast;
   unsigned char m_ucPadding_3[CACHE_LINE_SIZE - sizeof(CNode<T>*)];

   // shared among producers
   std::atomic<bool> m_bProducerLock;
   unsigned char m_ucPadding_4[CACHE_LINE_SIZE - sizeof(std::atomic<bool>)];
};

#endif