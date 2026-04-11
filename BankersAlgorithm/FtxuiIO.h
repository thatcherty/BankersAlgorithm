#pragma once
#include <iostream>
#include <string>
#include <deque>

// FTXUI
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

using namespace std;
using namespace ftxui;

class FtxuiIO
{
public:
    FtxuiIO() = default;

    // Keep only the most recent N lines so output doesn't "never clear".
    void PrintLine(const string& s);

    void ClearOutput();

    string PromptLine(const string& prompt, const string& placeholder = "");

    char PromptChar(const string& prompt);

    int PromptInt(const string& prompt);

    bool PromptYesNo(const string& prompt);

    // full screen display
    void ShowMessage(const string& title, const string& body);

private:
    void RunTextPrompt(const string& prompt, const string& placeholder, string& out);

    string JoinLines() const;

    static constexpr size_t kMaxLines = 8;
    deque<string> lines_;
};
