#pragma once

#include <wrl.h>
using namespace Microsoft::WRL;

#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <dxgi1_4.h>
#include <DirectX/d3dx12.h>

#pragma comment(lib,"d3dcompiler")
#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")


#include <string>
#include <exception>
#include <comdef.h>

#include "d3dUtils.h"
#include "define.h"
#include "ptr.h"