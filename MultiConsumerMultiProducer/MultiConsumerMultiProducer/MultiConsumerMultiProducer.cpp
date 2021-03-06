// MultiConsumerMultiProducer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Node.h"
#include "CLockFreeQueue.h"

#include <iostream>
#include <thread>


CLockFreeQueue<int> g_myQueue;

void ThreadProduceNode(int iValueToAdd, int iThreadID)
{
   //try to produce 5 times per thread
   for (int i = 0; i < MAX_PRODUCE_ATTEMPTS; i++)
   {
      g_myQueue.ProduceNewNode(iValueToAdd + i, iThreadID);
   }

   g_myQueue.ProduceNewNode(STOP_CONSUMER_VALUE, iThreadID);
}

void ThreadConsumeNode(int iThreadID)
{
   int iHeadValue;
   bool res = true;



   while (res)
   {
      res = g_myQueue.ConsumeHeadNode(iHeadValue, iThreadID);
      if (res)
      {
         //we consumed something
         std::cout << "T_CONSUME" << iThreadID << ":  res: " << res << " value: " << iHeadValue << std::endl;
         if (iHeadValue == STOP_CONSUMER_VALUE)
         {
            res = false;
         }
      }
      else
      {
         //queue was empty, sleep and try again
         auto start = std::chrono::high_resolution_clock::now();
         std::this_thread::sleep_for(THREAD_CONSUMER_RETRY_SLEEP_DELAY_SECONDS);
         res = true;
      }
   }
   std::cout << "T_CONSUME" << iThreadID << " ended" << std::endl;
}

static const int num_producer_threads = 20;
static const int num_consumer_threads = 4;

int main()
{
   std::thread rg_tProducers[num_producer_threads];
   std::thread rg_tConsumers[num_consumer_threads];


   for (unsigned short i = 0; i < num_producer_threads; i++)
   {
      rg_tProducers[i] = std::thread(ThreadProduceNode, i*10, i);
   }

   

   //sleep for a bit, before starting to consume
   std::cout << "T_CONSUME: Starting delay for consumer thread" << std::endl;
   auto start = std::chrono::high_resolution_clock::now();
   std::this_thread::sleep_for(CONSUMER_THREADS_START_TIMEOUT);


   //start consumers
   for (unsigned short i = 0; i < num_consumer_threads; i++)
   {
      rg_tConsumers[i] = std::thread(ThreadConsumeNode, i);
   }



   //Join the producer and consumer threads with the main thread
   for (int i = 0; i < num_producer_threads; ++i)
   {
      rg_tProducers[i].join();
   }

   for (int i = 0; i < num_consumer_threads; ++i)
   {
      rg_tConsumers[i].join();
   }
   return 0;
}

