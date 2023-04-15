// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "CopyDialogWindow.xaml.h"
#if __has_include("CopyDialogWindow.g.cpp")
#include "CopyDialogWindow.g.cpp"
#endif

#include "WindowHelper.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::FastCopy::implementation
{
    CopyDialogWindow::CopyDialogWindow()
    {
        InitializeComponent();

        CenterWindow(*this, { 450, 140 });
        auto appWindow = GetAppWindow(*this);
        auto presenter = appWindow.Presenter().as<winrt::Microsoft::UI::Windowing::OverlappedPresenter>();
        presenter.IsResizable(false);
        presenter.IsMaximizable(false);
        presenter.IsMinimizable(true);

        appWindow.TitleBar().ExtendsContentIntoTitleBar(true);
        appWindow.TitleBar().ButtonBackgroundColor(winrt::Windows::UI::Colors::Transparent());
        appWindow.TitleBar().ButtonInactiveBackgroundColor(winrt::Windows::UI::Colors::Transparent());
    }
}