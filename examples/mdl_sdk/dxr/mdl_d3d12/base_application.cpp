/******************************************************************************
 * Copyright (c) 2019-2025, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include "base_application.h"
#include "command_queue.h"
#include "descriptor_heap.h"
#include "dxgidebug.h"
#include "mdl_sdk.h"
#include "window.h"
#include "window_image_file.h"
#include "window_win32.h"
#include <memory>

#include <errhandlingapi.h>
#include <strsafe.h>
#include <dbghelp.h>
#include <d3d12.h>
#include<Windows.h>  

typedef LONG NTSTATUS, *PNTSTATUS;
typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

RTL_OSVERSIONINFOW GetOSVersion() {
    HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
    if (hMod) {
        RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
        if (fxPtr != nullptr) {
            RTL_OSVERSIONINFOW rovi = { 0 };
            rovi.dwOSVersionInfoSize = sizeof(rovi);
            if (0 == fxPtr(&rovi)) {
                return rovi;
            }
        }
    }
    RTL_OSVERSIONINFOW rovi = { 0 };
    return rovi;
}

namespace mi { namespace examples { namespace mdl_d3d12
{

Base_application_message_interface::Base_application_message_interface(
    Base_application* app,
    HINSTANCE instance)
    : m_app(app)
    , m_instance(instance)
{
}

// ------------------------------------------------------------------------------------------------

void Base_application_message_interface::key_down(uint8_t key)
{
    m_app->key_down(key);
}

void Base_application_message_interface::key_up(uint8_t key)
{
    m_app->key_up(key);
}

// ------------------------------------------------------------------------------------------------

void Base_application_message_interface::paint()
{
    m_app->update();
    m_app->render();
}

// ------------------------------------------------------------------------------------------------

void Base_application_message_interface::resize(size_t width, size_t height, double dpi)
{
    m_app->flush_command_queues();
    m_app->m_window->resize(width, height, dpi);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

Base_application::Base_application()
    : m_options(nullptr)
    , m_window(nullptr)
    , m_resource_descriptor_heap(nullptr)
    , m_render_target_descriptor_heap(nullptr)
    , m_mdl_sdk(nullptr)
    , m_scene_is_updating_next(false)
{
    m_update_args.frame_number = 0;
    m_update_args.elapsed_time = 0.0;
    m_update_args.total_time = 0.0;

    m_render_args.frame_number = 0;
    m_render_args.back_buffer = nullptr;
    m_render_args.back_buffer_rtv = {0};
}

// ------------------------------------------------------------------------------------------------

Base_application::~Base_application()
{
    log_set_file_path(nullptr); // close the file if there is one
}

// ------------------------------------------------------------------------------------------------

namespace // anonymous
{
LONG top_level_exection_filter(PEXCEPTION_POINTERS exception_information)
{
    BOOL bMiniDumpSuccessful;
    WCHAR szPath[MAX_PATH];
    WCHAR szFileName[MAX_PATH];
    DWORD dwBufferSize = MAX_PATH;
    HANDLE hDumpFile;
    SYSTEMTIME stLocalTime;
    MINIDUMP_EXCEPTION_INFORMATION ExpParam;

    // path is the executable directory
    std::string exec_path = mi::examples::io::get_working_directory();
    std::wstring exec_path_w = mi::examples::strings::str_to_wstr(exec_path);
    wmemcpy_s(szPath, dwBufferSize, exec_path_w.c_str(), exec_path_w.size());

    // filename based on time and date
    GetLocalTime(&stLocalTime);
    wsprintfW(szFileName, L"%04d-%02d-%02d_%02d-%02d-%02d_%ld_%ld.dmp",
        stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
        stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond,
        GetCurrentProcessId(), GetCurrentThreadId());

    std::wstring full_path = exec_path_w + L"/" + szFileName;
    hDumpFile = CreateFile(full_path.c_str(), GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

    ExpParam.ThreadId = GetCurrentThreadId();
    ExpParam.ExceptionPointers = exception_information;
    ExpParam.ClientPointers = TRUE;

    bMiniDumpSuccessful = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
        hDumpFile, MiniDumpWithFullMemory, &ExpParam, NULL, NULL);

    mdl_d3d12::log_error("Application crashed. Writing dump file in the executable folder: " +
        mi::examples::strings::wstr_to_str(szFileName));

    mdl_d3d12::flush_loggers();
    return EXCEPTION_EXECUTE_HANDLER;
}
}

// ------------------------------------------------------------------------------------------------

int Base_application::run(
    Base_options* options,
    Base_dynamic_options* dynamic_options,
    HINSTANCE hInstance,
    int nCmdShow)
{
    // setup mini-dumps
    SetUnhandledExceptionFilter(top_level_exection_filter);

    // create graphics context, load MDL SDK, ...
    if (!initialize_internal(options, dynamic_options))
    {
        // give the user time to read the reason of failure
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        Diagnostics::list_loaded_libraries(Log_level::Info);
        return -1;
    }

    // create the window
    Base_application_message_interface message_interface(this, hInstance);
    if (options->no_gui)
        m_window = new Window_image_file(
            message_interface, m_options->output_file, m_options->iterations);
    else
        m_window = new Window_win32(message_interface);

    m_render_args.backbuffer_width = m_window->get_width();
    m_render_args.backbuffer_height = m_window->get_height();

    int return_code = 0;
    // load the applications content and then run the main loop
    if (load())
    {
        // show the window and run the message loop
        return_code = m_window->show(nCmdShow);
        if (return_code != 0)
            log_warning("Applications main loop stopped with issues.", SRC);

        // complete the current work load
        flush_command_queues();
    }
    else
    {
        log_error("Loading Applications failed. Freeing already loaded content.", SRC);
        // give the user time to read the reason of failure 
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }

    // unload the application
    if (!unload())
    {
        log_error("Unloading Applications failed.", SRC);
        // give the user time to read the reason of failure 
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }

    // before unloading list every library that is in use
    Diagnostics::list_loaded_libraries(return_code == 0 ? Log_level::Verbose : Log_level::Info);

    // release base application resources
    for (auto&& queue : m_command_queues)
        delete queue.second;

    delete m_window;
    delete m_mdl_sdk;
    delete m_resource_descriptor_heap;
    delete m_render_target_descriptor_heap;
    set_dred_device(nullptr);
    m_device = nullptr;
    m_factory = nullptr;

    if (options->gpu_debug)
    {
        ComPtr<IDXGIDebug1> debugController;
        if (!log_on_failure(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debugController)), 
            "Failed to get query DXGI Debug Infterface"))
        {
            debugController->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
        }
    }

    return return_code;
}

// ------------------------------------------------------------------------------------------------

Command_queue* Base_application::get_command_queue(D3D12_COMMAND_LIST_TYPE type)
{
    auto found = m_command_queues.find(type);
    if (found != m_command_queues.end())
        return found->second;

    Command_queue* new_queue = new Command_queue(this, type);
    m_command_queues[type] = new_queue;
    return new_queue;
}

// ------------------------------------------------------------------------------------------------

void Base_application::flush_command_queues()
{
    for (auto&& it : m_command_queues)
        it.second->flush();
}

// ------------------------------------------------------------------------------------------------

Descriptor_heap* Base_application::get_resource_descriptor_heap()
{
    { return m_resource_descriptor_heap; }
}

// ------------------------------------------------------------------------------------------------

Descriptor_heap* Base_application::get_render_target_descriptor_heap()
{
    { return m_render_target_descriptor_heap; }
}

// ------------------------------------------------------------------------------------------------

bool Base_application::initialize_internal(
    Base_options* options,
    Base_dynamic_options* dynamic_options)
{
    if (!initialize(options))
        return false;

    m_options = options;
    m_dynamic_options = dynamic_options;

    // log version number for potential repros
    RTL_OSVERSIONINFOW os_version = GetOSVersion();
    log_info("OS Version: Windows " + std::to_string(os_version.dwMajorVersion) + "." +
        std::to_string(os_version.dwMinorVersion) + " build " + std::to_string(os_version.dwBuildNumber));

    #ifdef AGILITY_SDK_ENABLED
        const std::string agility_sdk_enabled = " enabled: YES";
    #else
        const std::string agility_sdk_enabled = " enabled: NO";
    #endif
    #ifdef D3D12_PREVIEW_SDK_VERSION
        log_info("Microsoft Agility SDK Version: " +
            std::to_string(D3D12_PREVIEW_SDK_VERSION) + agility_sdk_enabled);
    #else
        log_info("Microsoft Agility SDK Version: None" + agility_sdk_enabled);
    #endif

    UINT dxgi_factory_flags = 0;
    D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_12_1;

    if(options->gpu_debug)
    {
        ComPtr<ID3D12Debug3> debugController;
        if (!log_on_failure(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)),
            "Failed to get query DXGI Debug Infterface"))
        {
            debugController->EnableDebugLayer();
            //debugController->SetEnableGPUBasedValidation(true);
            //debugController->SetGPUBasedValidationFlags(
            //    D3D12_GPU_BASED_VALIDATION_FLAGS_NONE);
            //debugController->SetEnableSynchronizedCommandQueueValidation(true);
        }

        ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
        if (!log_on_failure(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue)),
            "Failed to get query DXGI Debug Infterface"))
        {
            dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
            dxgiInfoQueue->SetBreakOnSeverity(
                DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
            dxgiInfoQueue->SetBreakOnSeverity(
                DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, false);
            dxgiInfoQueue->SetBreakOnSeverity(
                DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, false);
            dxgiInfoQueue->SetBreakOnSeverity(
                DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO, false);
            dxgiInfoQueue->SetBreakOnSeverity(
                DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE, false);
        }

        // Turn on auto-breadcrumbs and page fault reporting.
        // available from 10.0.18362.0
        #if WDK_NTDDI_VERSION > NTDDI_WIN10_RS5
            ComPtr<ID3D12DeviceRemovedExtendedDataSettings> dredSettings;
            if (!log_on_failure(D3D12GetDebugInterface(IID_PPV_ARGS(&dredSettings)),
                "Failed to get query DXGI Debug Infterface"))
            {
                dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            }
        #endif
    }

    if (log_on_failure(CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&m_factory)),
        "Failed to create DXGI Factory.", SRC))
        return false;


    // collect non-software adapters
    bool found_adapter = false;
    struct adapter_pair
    {
        ComPtr<IDXGIAdapter1> adapter;
        DXGI_ADAPTER_DESC1 desc;
    };

    std::deque<adapter_pair> available_adapters;

    ComPtr<IDXGIAdapter1> adapter;
    for (UINT a = 0; DXGI_ERROR_NOT_FOUND != m_factory->EnumAdapters1(a, &adapter); ++a)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        // check if a device fits the requirements
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            continue;

        available_adapters.push_back({adapter, desc});
    }

    // sort by dedicated memory, assuming that is a good heuristic
    std::sort(available_adapters.begin(), available_adapters.end(),
        [](const adapter_pair& a, const adapter_pair& b)
        {
            return a.desc.DedicatedVideoMemory > b.desc.DedicatedVideoMemory;
        });

    // allow the user to select a certain GPU
    if (available_adapters.size() > 1)
    {
        std::string msg =
            "Multiple GPUs detected, run with option '--gpu <num>' to select a specific one."
            "\n                      Default is the first one (from the top) that supports RTX:";

        for (size_t i = 0; i < available_adapters.size(); ++i)
        {
            msg += "\n                      - [" + std::to_string(i) + "] " +
                mi::examples::strings::wstr_to_str(available_adapters[i].desc.Description);
        }

        log_info(msg);
    }

    // if the user picked one, move that to the top
    if (options->gpu >= 0 && options->gpu < available_adapters.size())
    {
        adapter_pair selected = available_adapters[options->gpu];
        available_adapters.erase(available_adapters.begin() + options->gpu);
        available_adapters.push_front(selected);
    }

    // iterate over available devices and use the first that fits the requirements
    for (adapter_pair& pair : available_adapters)
    {
        std::string name = mi::examples::strings::wstr_to_str(pair.desc.Description);

        // create the device context
        if (!log_on_failure(D3D12CreateDevice(
            pair.adapter.Get(), feature_level, _uuidof(D3DDevice), &m_device),
            "Failed to create D3D Device: " + name))
        {
            // check ray tracing support and other features
            D3D12_FEATURE_DATA_D3D12_OPTIONS data;
            D3D12_FEATURE_DATA_D3D12_OPTIONS5 data5;
            m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &data, sizeof(data));
            m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &data5, sizeof(data5));
            if (data5.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
            {
                log_info("D3D Device does not support RTX: " + name);
                m_device = nullptr;
                continue;
            }

            // found a device that supports RTX
            log_info("Created context for D3D Device: " + name);

            // get device feature level
            {
                D3D12_FEATURE_DATA_FEATURE_LEVELS dLevels;
                D3D_FEATURE_LEVEL aLevels[] =
                {
                    //D3D_FEATURE_LEVEL_12_2,
                    D3D_FEATURE_LEVEL_12_1,
                    D3D_FEATURE_LEVEL_12_0,
                };
                dLevels.NumFeatureLevels = sizeof(aLevels) / sizeof(D3D_FEATURE_LEVEL);
                dLevels.pFeatureLevelsRequested = aLevels;
                m_device->CheckFeatureSupport(
                    D3D12_FEATURE_FEATURE_LEVELS,
                    &dLevels,
                    sizeof(D3D12_FEATURE_DATA_FEATURE_LEVELS)
                );

                std::string level = "";
                {
                    switch (dLevels.MaxSupportedFeatureLevel)
                    {
                    case D3D_FEATURE_LEVEL_12_1:
                        level = "D3D_FEATURE_LEVEL_12_1 (or higher)";
                        break;
                    case D3D_FEATURE_LEVEL_12_0:
                        level = "D3D_FEATURE_LEVEL_12_0";
                        break;
                    default:
                        level = "D3D_FEATURE_LEVEL_11_1 (or lower)";
                        break;
                    }
                }
                log_info("Supporting Feature Level: " + level);
            }

            // get device ray tracing tier
            {
                std::string tier = "";
                {
                    switch (data5.RaytracingTier)
                    {
                    case D3D12_RAYTRACING_TIER_1_1:
                        tier = "D3D12_RAYTRACING_TIER_1_1 (or higher)";
                        break;
                    case D3D12_RAYTRACING_TIER_1_0:
                        tier = "D3D12_RAYTRACING_TIER_1_0";
                        break;
                    default:
                        tier = "NOT_SUPPORTED";
                        break;
                    }
                }
                log_info("Supporting Raytracing Tier: " + tier);
            }

            // get shader model level
            {
                D3D12_FEATURE_DATA_SHADER_MODEL data_shader_model;
                D3D_SHADER_MODEL all_model_versions[] =
                {
                    D3D_SHADER_MODEL_6_6,
                    D3D_SHADER_MODEL_6_5,
                    D3D_SHADER_MODEL_6_4,
                    D3D_SHADER_MODEL_6_3
                };
                size_t num_versions = sizeof(all_model_versions) / sizeof(D3D_SHADER_MODEL);
                for (size_t i = 0; i < num_versions; i++)
                {
                    data_shader_model.HighestShaderModel = all_model_versions[i];
                    HRESULT hres = m_device->CheckFeatureSupport(
                        D3D12_FEATURE_SHADER_MODEL, &data_shader_model, sizeof(D3D12_FEATURE_DATA_SHADER_MODEL));
                    if (hres != E_INVALIDARG)
                    {
                        if (FAILED(hres))
                        {
                            data_shader_model.HighestShaderModel = static_cast<D3D_SHADER_MODEL>(0);
                        }
                        break;
                    }
                }
                std::string model = "";
                {
                    switch (data_shader_model.HighestShaderModel)
                    {
                    case D3D_SHADER_MODEL_6_6:
                        model = "D3D_SHADER_MODEL_6_6 (or higher)";
                        options->features.HLSL_dynamic_resources = true;
                        break;
                    case D3D_SHADER_MODEL_6_5:
                        model = "D3D_SHADER_MODEL_6_5";
                        break;
                    case D3D_SHADER_MODEL_6_4:
                        model = "D3D_SHADER_MODEL_6_4";
                        break;
                    case D3D_SHADER_MODEL_6_3:
                        model = "D3D_SHADER_MODEL_6_3";
                        break;
                    default:
                        model = "NOT_SUPPORTED";
                        break;
                    }
                }
                log_info("Supporting Shader Model: " + model);
            }

            // get device resource binding tier
            {
                std::string tier = "";
                {
                    switch (data.ResourceBindingTier)
                    {
                    case D3D12_RESOURCE_BINDING_TIER_3:
                        tier = "D3D12_RESOURCE_BINDING_TIER_3 (or higher)";
                        break;
                    case D3D12_RESOURCE_BINDING_TIER_2:
                        tier = "D3D12_RESOURCE_BINDING_TIER_2";
                        options->features.HLSL_dynamic_resources = false;
                        break;
                    case D3D12_RESOURCE_BINDING_TIER_1:
                        tier = "D3D12_RESOURCE_BINDING_TIER_1";
                        options->features.HLSL_dynamic_resources = false;
                        break;
                    default:
                        tier = "NOT_SUPPORTED";
                        break;
                    }
                }
                log_info("Supporting Resource Binding Tier: " + tier);
            }

            found_adapter = true;
            break;
        }
 
    }

    if (!found_adapter)
    {
        log_error("No D3D device found that fits the requirements.");
        return false;
    }

    if (options->gpu_debug)
    {
        ComPtr<ID3D12InfoQueue> pInfoQueue;
        if (!log_on_failure(m_device.As(&pInfoQueue), "Failed to query Info Queue"))
        {
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

            D3D12_MESSAGE_SEVERITY Severities[] =
            {
                D3D12_MESSAGE_SEVERITY_INFO
            };

            D3D12_INFO_QUEUE_FILTER NewFilter = {};
            NewFilter.DenyList.NumSeverities = _countof(Severities);
            NewFilter.DenyList.pSeverityList = Severities;

            if (log_on_failure(pInfoQueue->PushStorageFilter(&NewFilter),
                "Failed to setup D3D debug messages", SRC))
                return false;
        }

        set_dred_device(m_device.Get());
    }

    log_info("Supporting HLSL Dynamic Resources: " + 
        std::string(options->features.HLSL_dynamic_resources ? "YES" : "NO"));

    // check if the device context is still valid
    if (log_on_failure(m_device->GetDeviceRemovedReason(),
       "Created device is in invalid state.", SRC))
       return false;

    // create a heap for all resources
    m_resource_descriptor_heap = new Descriptor_heap(
        this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096 /* hard coded */, "ResourceHeap");

    m_render_target_descriptor_heap = new Descriptor_heap(
        this, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 8 /* hard coded */, "RenderTargetHeap");

    // load the MDL SDK and check for success
    m_mdl_sdk = new Mdl_sdk(this);
    if (!m_mdl_sdk->is_valid())
    {
        log_error("MDL SDK not initialized properly.");
        return false;
    }
    return true;
}

// ------------------------------------------------------------------------------------------------

Base_dynamic_options::Base_dynamic_options(const Base_options* options) 
    : m_restart_progressive_rendering(true)
    , m_active_lpe(options->lpe.empty() ? "beauty" : options->lpe[0])
    , m_available_aovs({})
    , m_active_aov(0)
{
}

bool Base_dynamic_options::get_restart_progressive_rendering()
{
    bool ret = m_restart_progressive_rendering;
    m_restart_progressive_rendering = false;
    return ret;
}

// ------------------------------------------------------------------------------------------------

void Base_application::update()
{
    m_update_args.scene_is_updating = m_scene_is_updating_next;
    m_render_args.scene_is_updating = m_scene_is_updating_next;

    // allow the application to adapt to new resolutions
    if ((m_window->get_width() != m_render_args.backbuffer_width) ||
        (m_window->get_height() != m_render_args.backbuffer_height))
    {
        m_render_args.backbuffer_width = m_window->get_width();
        m_render_args.backbuffer_height = m_window->get_height();
        on_resize(m_window->get_width(), m_window->get_height());
        flush_command_queues();
    }

    // compute elapsed time
    if (m_update_args.frame_number == 0)
    {
        m_mainloop_start_time = std::chrono::steady_clock::now();
    }
    else
    {
        auto now = std::chrono::steady_clock::now();
        double new_total_time = std::chrono::duration<double>(now - m_mainloop_start_time).count();

        m_update_args.elapsed_time = new_total_time - m_update_args.total_time;
        m_update_args.total_time = new_total_time;
    }

    // update the application
    update(m_update_args);
}

// ------------------------------------------------------------------------------------------------

void Base_application::render()
{
    m_render_args.back_buffer = m_window->get_back_buffer();
    m_render_args.back_buffer_rtv = m_window->get_back_buffer_rtv();
    render(m_render_args);
    m_window->present_back_buffer();

    m_update_args.frame_number++;
    m_render_args.frame_number++;
}

}}} // mi::examples::mdl_d3d12
