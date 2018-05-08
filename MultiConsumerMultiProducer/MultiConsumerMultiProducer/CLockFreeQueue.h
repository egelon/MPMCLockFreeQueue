#pragma once
#ifndef CLOCKFREEQUEUE_H
#define CLOCKFREEQUEUE_H

#include "Node.h"

#include <iostream>
#include <chrono>
#include <thread>

using namespace std::literals::chrono_literals;

template <typename T>
class CLockFreeQueue
{
public:
   CLockFreeQueue()
   {
      m_pFirst = new Node<T>(nullptr);
      m_pLast = m_pFirst;
      m_bProducerLock = false;
      m_bConsumerLock = false;
   };

   ~CLockFreeQueue()
   {
      while (m_pFirst != nullptr)
      {
         Node<T>* tmp = m_pFirst;
         m_pFirst = m_pFirst->m_pNext;
         delete tmp->m_pValue;
         delete tmp;
      }
   };

   void ProduceNewNode(const T& pValueToAdd, int iThreadID )
   {
      //make a new node locally
      Node<T>* pNewNode = new Node<T>(new T(pValueToAdd));

      //do some fake work
      std::cout << "T_PRODUCE" << iThreadID << ": Starting to produce item" << std::endl;
      auto start = std::chrono::high_resolution_clock::now();
      std::this_thread::sleep_for(THREAD_PRODUCER_SLEEP_TIME_SECONDS);

      while (m_bProducerLock.exchange(true)) { } //acquire exclusivity (essentially "obtain a critical section lock")
      //we are now in our "critical section"
      
      m_pLast->m_pNext = pNewNode;  //attach the new node to the queue
      m_pLast = pNewNode;           //the new node is now the last node
      
      //and now we leave the "critical section"
      m_bProducerLock = false; //release exclusivity ("release the lock")

      //might as well see how long it took
      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> elapsed = end - start;
      std::cout << "T_PRODUCE" << iThreadID << ": Item "<< pValueToAdd <<" produced in " << elapsed.count() << " ms\n";
   }

   bool ConsumeHeadNode(T& result, int iThreadID)
   {
      //start a timer
      std::cout << "T_CONSUME" << iThreadID << ": Starting to handle first item" << std::endl;
      auto start = std::chrono::high_resolution_clock::now();
      


      //first, attempt to "obtain a lock"
      while (m_bConsumerLock.exchange(true)) { }  //acquire exclusivity
      //we are now in our "critical section"

      Node<T>* pOldDummyHead = m_pFirst; //get a pointer to the empty "dummy head" node
      Node<T>* pFirstValueNode = m_pFirst->m_pNext; //...and to its "next" node, which is the first "actual" node with a value (hopefully)

      //check if the "value" node is empty
      if (pFirstValueNode == nullptr)
      {
         //if it is empty, then we only had our "dummy head" node, so our queue was in fact empty
         
         //leave the "critical section"
         m_bConsumerLock = false;


         //might as well see how long it took
         auto end = std::chrono::high_resolution_clock::now();
         std::chrono::duration<double, std::milli> elapsed = end - start;
         std::cout << "T_CONSUME" << iThreadID << ": Queue was empty! Item handled in " << elapsed.count() << " ms\n";
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
      std::this_thread::sleep_for(THREAD_CONSUMER_SLEEP_TIME_SECONDS);

      result = *pNodeValue;    //copy the node's value to the output param
      delete pNodeValue; // clean up the value
      delete pOldDummyHead; //and the old dummy

      //might as well see how long it took
      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> elapsed = end - start;
      std::cout << "T_CONSUME" << iThreadID << ": Item handled in " << elapsed.count() << " ms\n";

      return true;
   }

private:

   unsigned char m_ucPadding_0[CACHE_LINE_SIZE];
   // for one consumer at a time
   Node<T>* m_pFirst;
   unsigned char m_ucPadding_1[CACHE_LINE_SIZE - sizeof(Node<T>*)];

   // shared among consumers
   std::atomic<bool> m_bConsumerLock;
   unsigned char m_ucPadding_2[CACHE_LINE_SIZE - sizeof(std::atomic<bool>)];

   // for one producer at a time
   Node<T>* m_pLast;
   unsigned char m_ucPadding_3[CACHE_LINE_SIZE - sizeof(Node<T>*)];

   // shared among producers
   std::atomic<bool> m_bProducerLock;
   unsigned char m_ucPadding_4[CACHE_LINE_SIZE - sizeof(std::atomic<bool>)];
};

#endif