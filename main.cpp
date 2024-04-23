#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")


//�E�B���h�E�v���[�W��
LRESULT CALLBACK WindwsProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparm) {
	// ���b�Z�[�W�ɉ����ăQ�[���ŗL�̏������s��
	switch (msg)
	{
		//�E�B���h�E���������ꂽ
	case WM_DESTROY:
			//os�ɑ΂��āA�A�v���̏I����`����
			PostQuitMessage(0);
			return 0;
	}

	//�W�����b�Z�[�W�������s��
	return DefWindowProc(hwnd, msg, wparam, lparm);
}
std::wstring ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}

void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());

	//�ϐ�����^���ێ����Ă����
	//Log(std::format("enemyHp:{}, texturePath:{}\n", SystemPowerStateNotify))
}

void Log(const std::wstring& message) {
	OutputDebugStringA(ConvertString(message).c_str());
}

//Windows�A�v���ł̃G���g���[�|�C���g(main�֐�)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	WNDCLASS wc{};
	//�E�B���h�E�v���[�W��
	wc.lpfnWndProc = WindwsProc;
	//�E�B���h�E�̃N���X��
	wc.lpszClassName = L"CGWindowClass";
		//�C���X�^���g�n���h��
	wc.hInstance = GetModuleHandle(nullptr);
	//�J�[�\��
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	//�E�B���h�E�N���X��o�^����
	RegisterClass(&wc);

	//�N���C�A���g�̈�T�C�Y
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	//�E�B���h�E�T�C�Y��\���\���̂ɃN���C�A���g�̈������
	RECT wrc = { 0, 0, kClientWidth, kClientHeight };

	//�N���C�A���g�̈�����ƂɎ��ۂ̃T�C�Y��wrc��ύX���Ă��炤
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//�E�B���h�E�̐���
	HWND hwnd = CreateWindow(
		wc.lpszClassName,
		L"CG2",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr);

	//�E�B���h�E��\������
	ShowWindow(hwnd, SW_SHOW);

#ifdef DEBUG
	ID3D12Debug* debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))));
	{
		//�f�o�b�O���C���[��L��������
		debugController->EnableDebugLayer();
		//�����GPU���ł��`�F�b�N���s���悤�ɂ���
		debugController->SetEnableGPUBasedValiDation(TRUE);
	}

#endif // DEBUG


	//�o�E�B���h�E�ւ̕����o��
	OutputDebugStringA("Hello,DirectX!\n");

	//DXGI�t�@�N�g���[�̍쐬
	IDXGIFactory7* dxgiFactory = nullptr;
	//HRESULT��Windows�n�̃G���[�R�[�h�ł���֐��������������ǂ�����SUCCEEDED�}�N���Ŕ���ł���
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	//�������̍��{�I�ȕ����ŃG���[���o���ꍇ�̓v���O�������Ԉ���Ă��邩�ǂ����A�ǂ��ɂ��ł��Ȃ��ꍇ�������̂�assert�ɂ��Ă���
	assert(SUCCEEDED(hr));

	//�g�p����A�_�v�^�p�̕ϐ��A�ŏ��Anullptr�����Ă���
	IDXGIAdapter4* useAdapter = nullptr;
	//�ǂ����ɃA�_�v�^��ǂ�
	for (UINT i = 0; i < dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; i++){
		//�A�_�v�^�[�̏����擾����
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));//�擾�ł��Ȃ��͈̂�厖
		//�\�t�g�E�F�A�A�_�v�^�łȂ���΍̗p
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)){
			//�̗p�����A�_�v�^�̏������O�ɏo�́Awstring�̂ق��Ȃ̂Œ���
			Log(std::format(L"use Adapter:{}\n", adapterDesc.Description));
			break;
		}
		useAdapter = nullptr;
	}
	//�K�؂ȃA�_�v�^��������Ȃ������̂ŋN���ł��Ȃ�
	assert(useAdapter != nullptr);

	ID3D12Device* device = nullptr;
	//������x���ƃ��O�o�͗p�̕�����
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };
	//�������ɐ����ł��邩�����Ă���
	for (size_t i = 0; i < _countof(featureLevels); i++){
		//�̗p�����A�_�v�^�[�Ńf�o�C�X�𐶐�
		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
		//�w�肵��������x���Ńf�o�C�X�������ł��������m�F
		if (SUCCEEDED(hr)){
			//�����ł��Ȃ̂Ń��O�o�͂��s���ă��[�v�𔲂���
			Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	//�f�o�C�X�̐��������܂������Ȃ������̂ŋN���ł��Ȃ�
	assert(device != nullptr);
	Log("Comolete create 3D12Device!!!\n");//�����������̃��O���o��

#ifdef DEBUG

	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
	{
		//��΂��G���[���Ɏ~�܂�
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//�G���[���Ɏ~�܂�
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//�x�����Ɏ~�܂�
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		//���
		infoQueue->Release();

		//�}�����郁�b�Z�[�W��ID
		D3D12_MESSAGE_ID denyIds[] = {
			//Windows11�ł�DXGI�f�o�b�O���C���[��DX12�f�o�b�O���C���[�̑��ݍ�p�o�O�ɂ��G���[���b�Z�[�W
			//
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//�}�����郌�x��
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//�w�肵�����b�Z�[�W�̕\����}������
		infoQueue->PushStorageFilter(&filter);
	}
#endif // DEBUG

	//�R�}���h�L���[���쐬����
	ID3D12CommandQueue* commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	//�R�}���h�L���[�̐��������܂������Ȃ������̂ŋN���ł��Ȃ�
	assert(SUCCEEDED(hr));

	//�R�}���h�A���P�[�^���쐬����
	ID3D12CommandAllocator* commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//�R�}���h�A���P�[�^�̐��������܂������Ȃ������̂ŋN���ł��Ȃ�
	assert(SUCCEEDED(hr));

	//�R�}���h���X�g�𐶐�����
	ID3D12GraphicsCommandList* commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	//�R�}���h���X�g�̐��������܂������Ȃ������̂ŋN���ł��Ȃ�
	assert(SUCCEEDED(hr));

	//�X���b�v�`�F�[�����쐬����
	IDXGISwapChain4* swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = kClientWidth;                            //��ʂ̕��B�E�B���h�E�̃N���C�A���g�̈�𓯂����̂ɂ��Ă���
	swapChainDesc.Height = kClientHeight;                          //��ʂ̍����B�E�B���h�E�̃N���C�A���g�ʗǂ��C�𓯂����̂ɂ��Ă���
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;             //�F�̌`��
	swapChainDesc.SampleDesc.Count = 1;                            //�}���`�T���v�����Ȃ�
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;   //�`��^�[�Q�b�g�Ƃ��ė��p����
	swapChainDesc.BufferCount = 2;                                 //�_�u���o�b�t�@
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;      //���j�^�Ɉڂ�����A���g��j��
	//�R�}���h�L���[�A�E�B���h�E�n���h���A�ݒ��n���Đ�������
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain));
	assert(SUCCEEDED(hr));

	//�f�B�X�N���v�^�[�q�[�v�̍쐬
	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NumDescriptors = 2;
	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	//�f�B�X�N���v�^�[�q�[�v�����Ȃ������̂ŋN���ł��Ȃ�
	assert(SUCCEEDED(hr));

	//SwapChain����Resource�����������Ă���
	ID3D12Resource* swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	//���܂��擾�ł��Ȃ���΋N���ł��Ȃ�
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

	//RTV�̐ݒ�
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;        //�o�͌��ʂ�SRGB�ɕϊ����ď�������
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;   //2ed�e�N�X�`���Ƃ��ď�������
	//�f�B�X�N���v�^�̐擪���擾����
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//RTV��2���̂Ńf�B�X�N���v�^��2�p��
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	//�܂�1�ڂ����B1�ڂ͍ŏ��̂Ƃ���ɍ��B���ꏊ��������Ŏw�肵�Ă�����K�v������
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[1]);

	typedef struct D3D12_CPU_DESCRIPTOR_HANDLE {
		SIZE_T ptr;
	}D3D12_CPU_DESCRIPTOR_HANDLE;

	//2�ڂ̃f�B�X�N���v�^�n���h���𓾂�
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//2�ڂ����
	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);


	//�������O��Fance�����
	ID3D12Fence* fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	//Fence��Signal�������߂ɃC�x���g���쐬����
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);

	//�Q�[�����[�v
	MSG msg{};
	//�E�B���h�E�́~�{�^�����������܂Ń��[�v
	while (msg.message != WM_QUIT){
		//Window�Ƀ��b�Z�[�W�����Ă���ŗD��ŏ���������
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//�Q�[���̏���
			//���ꂩ�珑�����ރo�b�N�o�b�t�@�̃C���f�b�N�X���擾
			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();


			//Transition��Barrier�̐ݒ�
			D3D12_RESOURCE_BARRIER barrier{};
			//����̃o���A��Transition
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			//None�ɂ��Ă���
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			//�o���A�𒣂�Ώۂ̃��\�[�X�B���݂̃o�b�N�o�b�t�@�ɑ΂��čs��
			barrier.Transition.pResource = swapChainResources[backBufferIndex];
			//�J�ڑO(����)��RecourceState
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			//�J�ڌ��ResourceState
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			//TransitionBarrier�𒣂�
			commandList->ResourceBarrier(1, &barrier);

			//�`����RTV��ݒ肷��
			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
			//�w�肵���F�ŉ�ʑS�̂��N���A����
			float clearColor[] = { 1.0f, 0.25f, 0.5f, 1.0f }; //���ۂ��F�BRGBA�̏�
			commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);


			//��ʂɕ`�������͂��ׂďI���A��ʂɈڂ��̂ŁA��Ԃ�J��
			//�����RenderTarget����Present�ɂ���
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			//TransitionBarrier�𒣂�
			commandList->ResourceBarrier(1, &barrier);

			//�R�}���h���X�g�̓��e���m�肳����B���ׂẴR�}���h��ς�ł���Close���邱��
			hr = commandList->Close();
			assert(SUCCEEDED(hr));

			//GPU�ɃR�}���h���X�g�̎��s���s�킹��
			ID3D12CommandList* commandLists[] = { commandList };
			commandQueue->ExecuteCommandLists(1, commandLists);
			//GPU��OS�ɉ�ʂ̌������s���悤�ʒm����
			swapChain->Present(1, 0);


			//Fence�̒l���X�V
			fenceValue++;
			//GPU�������܂ł��ǂ蒅�����Ƃ��ɁAFence�̒l���w�肵���l�ɑ������悤��Signal�𑗂�
			commandQueue->Signal(fence, fenceValue);

			//���̃t���[���p�̃R�}���h���X�g������
			hr = commandAllocator->Reset();
			assert(SUCCEEDED(hr));
			hr = commandList->Reset(commandAllocator, nullptr);
			assert(SUCCEEDED(hr));

			

			//Fence�̒l���w�肵��Signal�l�ɂ��ǂ蒅���Ă��邩�m�F����
			//GetCompleteDValue�̏�������Fence�쐬���ɓn���������l
			if (fence->GetCompletedValue() < fenceValue)
			{
				//�w�肵��Signal�ɂ��ǂ蒅���Ă��Ȃ��̂ŁA���ǂ蒅���܂ő҂悤�ɃC�x���g��ݒ肷��
				fence->SetEventOnCompletion(fenceValue, fenceEvent);
				//�C�x���g�҂�
				WaitForSingleObject(fenceEvent, INFINITE);
			}
		}
	}

	return 0;
}




