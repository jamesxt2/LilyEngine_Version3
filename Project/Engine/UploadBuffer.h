#pragma once

template<UINT BUFFERSIZE>
class UploadBuffer
{
public:
    UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer = false) :
        m_IsConstantBuffer(isConstantBuffer)
    {
        m_BufferSize = BUFFERSIZE;

        // Constant buffer elements need to be multiples of 256 bytes.
        // This is because the hardware can only view constant data 
        // at m*256 byte offsets and of n*256 byte lengths. 
        // typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
        // UINT64 OffsetInBytes; // multiple of 256
        // UINT   SizeInBytes;   // multiple of 256
        // } D3D12_CONSTANT_BUFFER_VIEW_DESC;
        if (isConstantBuffer)
            m_ElementByteSize = CalcConstantBufferByteSize(m_BufferSize);
        else
            m_ElementByteSize = m_BufferSize;

        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC resDesc(CD3DX12_RESOURCE_DESC::Buffer(m_ElementByteSize * elementCount));
        ThrowIfFailed(device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &resDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_UploadBuffer)));

        ThrowIfFailed(m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)));

        // We do not need to unmap until we are done with the resource.  However, we must not write to
        // the resource while it is in use by the GPU (so we must use synchronization techniques).
    }

    UploadBuffer(const UploadBuffer& rhs) = delete;
    UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
    ~UploadBuffer()
    {
        if (m_UploadBuffer != nullptr)
            m_UploadBuffer->Unmap(0, nullptr);

        m_MappedData = nullptr;
    }

    ID3D12Resource* Resource()const
    {
        return m_UploadBuffer.Get();
    }

    void CopyData(int elementIndex, const void* data)
    {
        memcpy(&m_MappedData[elementIndex * m_ElementByteSize], data, m_BufferSize);
    }

private:
	ComPtr<ID3D12Resource>	m_UploadBuffer;
	BYTE* m_MappedData = nullptr;

    UINT m_BufferSize = 0;
	UINT m_ElementByteSize = 0;
	bool m_IsConstantBuffer = false;
};