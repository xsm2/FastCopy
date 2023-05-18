// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "pch.h"

#include "App.xaml.h"
#include "CopyDialogWindow.xaml.h"
#include "MainWindow.xaml.h"
#include "ViewModelLocator.h"
#include "FastCopyCommandParser.h"
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include "CommandLine.h"
#include "DebugLogger.h"
#include <winrt/Microsoft.Windows.AppLifecycle.h>
#include <winrt/Windows.System.h>
#include <filesystem>


using namespace winrt;
using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;
using namespace FastCopy;
using namespace FastCopy::implementation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

/// <summary>
/// Initializes the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App()
{
    InitializeComponent();

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
    UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e)
        {
            if (IsDebuggerPresent())
            {
                auto errorMessage = e.Message();
                __debugbreak();
            }
        });
#endif
}


winrt::Windows::Foundation::IAsyncAction GetFromClipboard()
{
    //get data from copy paste
    auto packageView = winrt::Windows::ApplicationModel::DataTransfer::Clipboard::GetContent();
    if (!packageView.Contains(winrt::Windows::ApplicationModel::DataTransfer::StandardDataFormats::StorageItems()))
        co_return;

    auto sources = ViewModelLocator::GetInstance().XCopyViewModel().Sources();
    auto storageItems = co_await packageView.GetStorageItemsAsync();
    for (auto storageItem : storageItems)
    {
        if (storageItem.IsOfType(winrt::Windows::Storage::StorageItemTypes::File))
        {
            sources.Append(winrt::FastCopy::ExplorerItem{ storageItem.as<winrt::Windows::Storage::StorageFile>() });
        }
        else if (storageItem.IsOfType(winrt::Windows::Storage::StorageItemTypes::Folder))
        {
            sources.Append(winrt::FastCopy::ExplorerItem{ storageItem.as<winrt::Windows::Storage::StorageFolder>(), true, 3 });
        }
    }
}



static std::pair<std::wstring_view, std::wstring_view> ParseToastArgument(std::wstring_view argument)
{
    auto const index = argument.find(L'=');
    return { argument.substr(0, index), argument.substr(index + 1) };
}


void App::OnLaunched(LaunchActivatedEventArgs const&)
{
    if (auto arg = isLaunchByToastNotification(); arg)
        return launchByToastNotification(*arg);

    if (isLaunchSettings())
        return launchSettings();

    normalLaunch();
}

std::optional<winrt::Microsoft::Windows::AppLifecycle::AppActivationArguments> App::isLaunchByToastNotification()
{
    if (auto activateArg = winrt::Microsoft::Windows::AppLifecycle::AppInstance::GetCurrent().GetActivatedEventArgs();
        activateArg && activateArg.Kind() == winrt::Microsoft::Windows::AppLifecycle::ExtendedActivationKind::ToastNotification)
    {
        return activateArg;
    }
    return {};
}

void App::launchByToastNotification(winrt::Microsoft::Windows::AppLifecycle::AppActivationArguments const& args)
{
    auto argument = args
        .Data()
        .as<winrt::Windows::ApplicationModel::Activation::ToastNotificationActivatedEventArgs>()
        .Argument();

    if (argument.empty())
        exit(-1); //dismiss

    auto const [action, arg] = ParseToastArgument(argument);
    if (action == L"open")
    {
        winrt::Windows::System::Launcher::LaunchUriAsync(winrt::Windows::Foundation::Uri{ arg });
    }

    exit(0);
}

bool App::isLaunchSettings()
{
    return Command::Get().Size() == 1;
}

void App::launchSettings()
{
    setting = make<MainWindow>();
    setting.Activate();
}

void App::normalLaunch()
{
    auto const destination = Command::Get().GetDestination();
    if (destination.empty())
        return;

    auto const recordFile = Command::Get().RecordFile();
    auto viewModel = ViewModelLocator::GetInstance().RobocopyViewModel();

#ifndef _DEBUG
    if (recordFile.empty())
        return;
#endif // !_DEBUG


    viewModel.Destination(destination);
    viewModel.RecordFile(recordFile);
    viewModel.Finished([recordFile](auto, FinishState state) {
#ifndef _DEBUG
        if (state == FinishState::Success)
            std::filesystem::remove(recordFile.data());
#endif
    });

    LOGI(L"App started, record file: {}", recordFile.data());

    window = make<CopyDialogWindow>();
    window.Activate();
    m_helper.emplace(window);
    viewModel.Start();
}
