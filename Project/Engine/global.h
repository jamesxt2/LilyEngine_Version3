#pragma once

#include <wrl.h>
using namespace Microsoft::WRL;

#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <dxgi1_4.h>
#include <DirectX/d3dx12.h>
#include <DirectX/DirectXColors.h>
#include <DirectX/SimpleMath.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

#pragma comment(lib,"d3dcompiler")
#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")


#include <string>
#include <exception>
#include <comdef.h>
#include <vector>
#include <map>
#include <memory>
#include <ppl.h>
#include <fstream>

#include "d3dUtils.h"
#include "define.h"
#include "ptr.h"
#include "enum.h"
#include "struct.h"
#include "UploadBuffer.h"
#include "FrameResource.h"