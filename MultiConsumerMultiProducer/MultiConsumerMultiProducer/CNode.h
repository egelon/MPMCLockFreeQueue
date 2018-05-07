#pragma once
#ifndef CNODE_H
#define CNODE_H

#include "Common.h"
#include <atomic>

template <typename T>
class CNode
{
public:
   CNode(T* pValue) : m_pValue(pValue), m_pNext(nullptr) {};

   T* m_pValue;
   std::atomic<CNode*> m_pNext;
   unsigned char m_ucPadding[CACHE_LINE_SIZE - sizeof(T*) - sizeof(std::atomic<CNode*>)];
};

#endif