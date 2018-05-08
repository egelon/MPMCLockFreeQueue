#pragma once
#ifndef NODE_H
#define NODE_H

#include "Common.h"
#include <atomic>

template <typename T>
struct Node
{
   Node(T* pValue) : m_pValue(pValue), m_pNext(nullptr) {};


   T* m_pValue;
   std::atomic<Node*> m_pNext;
   unsigned char m_ucPadding[CACHE_LINE_SIZE - sizeof(T*) - sizeof(std::atomic<Node*>)];
};


#endif