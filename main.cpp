﻿#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dxgidebug.h>
#include <dxcapi.h>
#include "Vector4.h"
#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_wndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")



//ウィンドウプロージャ
LRESULT CALLBACK WindwsProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (ImGui_ImplWin32_wndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
	// メッセージに応じてゲーム固有の処理を行う
	switch (msg)
	{
		//ウィンドウが放棄された
	case WM_DESTROY:
			//osに対して、アプリの終了を伝える
			PostQuitMessage(0);
			return 0;
	}

	//標準メッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
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

	//変数から型を維持してくれる
	//Log(std::format("enemyHp:{}, texturePath:{}\n", SystemPowerStateNotify))
}

void Log(const std::wstring& message) {
	OutputDebugStringA(ConvertString(message).c_str());
}

ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;  // UploadHeapを使う

	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};

	// バッファリソース。テクスチャの場合はまた別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;              // リソースのサイズ。今回はVector4を3頂点分

	// バッファの場合はこれらは1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;

	// バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	
	// 実際に頂点リソースを作る
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

ID3D12DescriptorHeap* CreateDiscriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
	//ディスクリプターヒープの作成
	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc{};
	DescriptorHeapDesc.Type = heapType;
	DescriptorHeapDesc.NumDescriptors = numDescriptors;
	DescriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	//ディスクリプターヒープが作れなかったので起動できない
	assert(SUCCEEDED(hr));
	return descriptorHeap;

	/*//ディスクリプターヒープの作成
	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NumDescriptors = 2;
	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	//ディスクリプターヒープが作れなかったので起動できない
	assert(SUCCEEDED(hr));

	//SwapChainからResourceを引っ張ってくる
	ID3D12Resource* swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	//うまく取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));*/
}

IDxcBlob* CompileShader
(
	// CompilerするShaderファイルへのパス
	const std::wstring& filePath,
	// Compilerに使用するProfile
	const wchar_t* profile,
	// 初期化で生成したものを3つ
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler
)
{
	// これからシェーダーをコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin CompilerShader, path:{}, profile:{}\n", filePath, profile)));
	// hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	// 読めなかったら止める
	assert(SUCCEEDED(hr));
	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8; // UTF8の文字コードであることを通知

	// 2,Compileする
	LPCWSTR arguments[] = {
		filePath.c_str(), // コンパイル対象のhlslファイル名
		L"-E", L"main", //エントリーポイントの指定。基本的にmain以外にはしない
		L"-T", profile, // ShaderProfileの設定
		L"-Zi", L"-Qembed_debug", // デバッグ用の情報を埋め込む
		L"-Od",              // 適用化を外しておく
		L"-Zpr",             //メモリレイアウトは行優先
	};
	// 実際にShaderをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,   // 読み込んだファイル
		arguments,             // コンパイルオプション
		_countof(arguments),   // コンパイルオプションの数
		includeHandler,        // includeが含まれた諸々
		IID_PPV_ARGS(&shaderResult) // コンパイル結果
	);
	//コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));

	//警告・エラーが出てたらログに出して止める
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0){
		Log(shaderError->GetStringPointer());
		//警告・エラーダメゼッタイ
		assert(false);
	}

	//コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	//成功したログを出す
	Log(ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));
	// もう使わないリソースを解放
	shaderSource->Release();
	shaderResult->Release();
	//実行用バイナリを辺ky九
	return shaderBlob;
}

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	WNDCLASS wc{};
	//ウィンドウプロージャ
	wc.lpfnWndProc = WindwsProc;
	//ウィンドウのクラス名
	wc.lpszClassName = L"CGWindowClass";
		//インスタントハンドル
	wc.hInstance = GetModuleHandle(nullptr);
	//カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	//ウィンドウクラスを登録する
	RegisterClass(&wc);

	//クライアント領域サイズ
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	//ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0, 0, kClientWidth, kClientHeight };

	//クライアント領域をもとに実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウの生成
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

	//ウィンドウを表示する
	ShowWindow(hwnd, SW_SHOW);

#ifdef _DEBUG
	ID3D12Debug1* debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))));
	{
		//デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();
		//さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}

#endif // DEBUG


	//出ウィンドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");

	//DXGIファクトリーの作成
	IDXGIFactory7* dxgiFactory = nullptr;
	//HRESULTはWindows系のエラーコードであり関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	//初期化の根本的な部分でエラーが出た場合はプログラムが間違っているかどうか、どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));

	//使用するアダプタ用の変数、最初、nullptrを入れておく
	IDXGIAdapter4* useAdapter = nullptr;
	//良い順にアダプタを読む
	for (UINT i = 0; i < dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; i++){
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));//取得できないのは一大事
		//ソフトウェアアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)){
			//採用したアダプタの情報をログに出力、wstringのほうなので注意
			Log(std::format(L"use Adapter:{}\n", adapterDesc.Description));
			break;
		}
		useAdapter = nullptr;
	}
	//適切なアダプタが見つからなかったので起動できない
	assert(useAdapter != nullptr);

	ID3D12Device* device = nullptr;
	//昨日レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };
	//高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); i++){
		//採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
		//指定した昨日レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr)){
			//生成できなのでログ出力を行ってループを抜ける
			Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	//デバイスの生成がうまくいかなかったので起動できない
	assert(device != nullptr);
	Log("Comolete create 3D12Device!!!\n");//初期化完了のログを出す

#ifdef _DEBUG

	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
	{
		//やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		//抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			//Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
			//
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);
		//解放
		infoQueue->Release();
	}
#endif // DEBUG

	//コマンドキューを作成する
	ID3D12CommandQueue* commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	//コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドアロケータを作成する
	ID3D12CommandAllocator* commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//コマンドアロケータの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドリストを生成する
	ID3D12GraphicsCommandList* commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	//コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//スワップチェーンを作成する
	IDXGISwapChain4* swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = kClientWidth;                            //画面の幅。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Height = kClientHeight;                          //画面の高さ。ウィンドウのクライアント量良い気を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;             //色の形式
	swapChainDesc.SampleDesc.Count = 1;                            //マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;   //描画ターゲットとして利用する
	swapChainDesc.BufferCount = 2;                                 //ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;      //モニタに移したら、中身を破棄
	//コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain));
	assert(SUCCEEDED(hr));

	// RTV様のヒープでディスクリプタの数は2。RTVはShader内で触るものではないので、ShaderVisibleはfalse
	ID3D12DescriptorHeap* rtvDescriptorHeap = CreateDiscriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

	// SRV様のヒープでディスクリプタの数は128。SRVはShader内で触るものなので、shaderVisibleはtrue
	ID3D12DescriptorHeap* srvDescriptorHeap = CreateDiscriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

	//SwapChainからResourceを引っ張ってくる
	ID3D12Resource* swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	//うまく取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;        //出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;   //2edテクスチャとして書き込む
	//ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	//まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);


	//2つ目のディスクリプタハンドルを得る
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//2つ目を作る
	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);


	//初期化０でFanceを作る
	ID3D12Fence* fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	//FenceのSignalを持つためにイベントを作成する
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);

	// dxcCompilerを初期化
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	// 現時点でincludeはしないが、includeに対応するために設定を行っておく
	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // UploadHeapを使う
	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	// バッファリソース。テクスチャの場合また別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeof(Vector4) * 3;
	// バッファの場合はこれらは1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際に頂点リソースを作る
	ID3D12Resource* vertexResource = nullptr;
	hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	

	// RootSinature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// RootParameter作成。複数設定できるので配列。今回は結果1つだけの長さ1の配列
	D3D12_ROOT_PARAMETER rootParameter[1] = {};
	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;     // CBVを使う
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;  // PixelShaderで使う
	rootParameter[0].Descriptor.ShaderRegister = 0;                     // レジスタ番号0とバインド
	descriptionRootSignature.pParameters = rootParameter;               // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameter);   // 配列の長さ

	// マテリアル用のリソースを作る。 今回はcolor1つ分のサイズを用意する
	ID3D12Resource* materialResouce = CreateBufferResource(device, sizeof(Vector4));
	// マテリアルにデータを書き込む
	Vector4* materialData = nullptr;
	// 書き込むためのアドレスを取得
	materialResouce->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// 今回は赤を書き込んでみる
	*materialData = Vector4(1.0f, 0.0f, 0.0f, 1.0f);


	//シリアライズしてバイナリにする
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリをもとに作成
	ID3D12RootSignature* rootSignature = nullptr;
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
	// InputLayout 
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);
	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	// RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	// Shaderをコンパイルする
	IDxcBlob* vertexShaderBlob = CompileShader(L"Object3D.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(vertexShaderBlob != nullptr);
	IDxcBlob* pixelShaderBlob = CompileShader(L"Object3D.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(pixelShaderBlob != nullptr);
	// PSOを作成する
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature; // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;  // InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer() , vertexShaderBlob->GetBufferSize() }; // VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };    // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc; // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ(形状)のタイプ、三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定(気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 実際に生成
	ID3D12PipelineState* graphicsPilelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPilelineState));
	assert(SUCCEEDED(hr));
	
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(Vector4) * 3;
	// 1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(Vector4);

	// 頂点リソースにデータを書き込む
	Vector4* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 左下
	vertexData[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
	// 上
	vertexData[1] = { 0.0f, 0.5f, 0.0f, 1.0f };
	// 右下
	vertexData[2] = { 0.5f, -0.5f, 0.0f, 1.0f };

	// ビューポート
	D3D12_VIEWPORT viewPort{};
	// クライアント領域の合図と一緒にして画面全体に表示
	viewPort.Width = kClientWidth;
	viewPort.Height = kClientHeight;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	// シザー矩形
	D3D12_RECT scissorRect{};
	// 基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;
	scissorRect.right = kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = kClientHeight;

	// ImGuiの初期化。詳細はさして重要ではないので開設は省略する
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(device, swapChainDesc.BufferCount, rtvDesc.Format, srvDescriptorHeap, srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	//ゲームループ
	MSG msg{};
	//ウィンドウの×ボタンが押されるまでループ
	while (msg.message != WM_QUIT){
		//Windowにメッセージが来てたら最優先で処理させる
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			//ゲームの処理

			// 開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に書き換える
			ImGui::ShowDemoWindow();

			// ImGuiの内部コマンドを生成する
			ImGui::Render();


			// これから書き込むバックバッファのインデックスを取得
			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();


			// TransitionのBarrierの設定
			D3D12_RESOURCE_BARRIER barrier{};
			// 今回のバリアはTransition
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			// Noneにしておく
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			// バリアを張る対象のリソース。現在のバックバッファに対して行う
			barrier.Transition.pResource = swapChainResources[backBufferIndex];
			// 遷移前(現在)のRecourceState
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			// 遷移後のResourceState
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			// TransitionBarrierを張る
			commandList->ResourceBarrier(1, &barrier);

			// 描画先のRTVを設定する
			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
			// 指定した色で画面全体をクリアする
			float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f }; //青っぽい色。RGBAの順
			commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

			// 描画用のDescriptorHeapの設定
			ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap };
			commandList->SetDescriptorHeaps(1, descriptorHeaps);

			commandList->RSSetViewports(1, &viewPort);  // Viewportを設定
			commandList->RSSetScissorRects(1, &scissorRect);   // Scirssorを設定
			// RootSignatureを設定。PSOに設定しているけど別途設定が必要
			commandList->SetGraphicsRootSignature(rootSignature);
			commandList->SetPipelineState(graphicsPilelineState);   // PSOを設定
			commandList->IASetVertexBuffers(0, 1, &vertexBufferView);  // VBVを設定
			// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			// マテリアルCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(0, materialResouce->GetGPUVirtualAddress());
			// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
			commandList->DrawInstanced(3, 1, 0, 0);
			 
			// 実際のcommandListのImGuiの描画コマンドを積む
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
			
			// 画面に描く処理はすべて終わり、画面に移すので、状態を遷移
			// 今回はRenderTargetからPresentにする
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			// TransitionBarrierを張る
			commandList->ResourceBarrier(1, &barrier);

			// コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
			hr = commandList->Close();
			assert(SUCCEEDED(hr));
 
			// GPUにコマンドリストの実行を行わせる
			ID3D12CommandList* commandLists[] = { commandList };
			commandQueue->ExecuteCommandLists(1, commandLists);
			// GPUとOSに画面の交換を行うよう通知する
			swapChain->Present(1, 0);


			// Fenceの値を更新
			fenceValue++;
			// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
			commandQueue->Signal(fence, fenceValue);

			

			

			// Fenceの値が指定したSignal値にたどり着いているか確認する
			// GetCompleteDValueの初期化はFence作成時に渡した初期値
			if (fence->GetCompletedValue() < fenceValue)
			{
				// 指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
				fence->SetEventOnCompletion(fenceValue, fenceEvent);
				// イベント待つ
				WaitForSingleObject(fenceEvent, INFINITE);
			}
			// 次のフレーム用のコマンドリストを準備
			hr = commandAllocator->Reset();
			assert(SUCCEEDED(hr));
			hr = commandList->Reset(commandAllocator, nullptr);
			assert(SUCCEEDED(hr));
		}
	}
	// ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
			

	CloseHandle(fenceEvent);
	fence->Release();
	rtvDescriptorHeap->Release();
	swapChainResources[0]->Release();
	swapChainResources[1]->Release();
	swapChain->Release();
	commandList->Release();
	commandAllocator->Release();
	commandQueue->Release();
	device->Release();
	useAdapter->Release();
	dxgiFactory->Release();
	vertexResource->Release();
	graphicsPilelineState->Release();
	signatureBlob->Release();
	if (errorBlob)
	{
		errorBlob->Release();
	}
	rootSignature->Release();
	pixelShaderBlob->Release();
	vertexShaderBlob->Release();
	materialResouce->Release();
	
#ifdef DEBUG
	debugController->Release();
#endif // DEBUG
	CloseWindow(hwnd);


	//リソースリークチェック
	IDXGIDebug1* debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}
	{

	}

	return 0;
}




