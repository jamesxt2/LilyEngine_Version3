#include "pch.h"
#include "CStructuredBuffer.h"

CStructuredBuffer::CStructuredBuffer(UINT elementByteSize, UINT elementCount)
	: m_ElementByteSize(elementByteSize)
{
}

CStructuredBuffer::~CStructuredBuffer()
{
}
