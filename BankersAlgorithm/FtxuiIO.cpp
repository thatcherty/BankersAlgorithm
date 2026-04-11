#include "FtxuiIO.h"

// Keep only the most recent N lines so output doesn't "never clear".
void FtxuiIO::PrintLine(const string& s)
{
    lines_.push_back(s);
    while (lines_.size() > kMaxLines) lines_.pop_front();
}

void FtxuiIO::ClearOutput() { lines_.clear(); }

string FtxuiIO::PromptLine(const string& prompt, const string& placeholder)
{
    string out;
    RunTextPrompt(prompt, placeholder, out);
    return out;
}

char FtxuiIO::PromptChar(const string& prompt)
{
    while (true)
    {
        string s = PromptLine(prompt, "one character");
        if (s.size() == 1) return s[0];
        PrintLine("Please enter exactly one character.");
    }
}

int FtxuiIO::PromptInt(const string& prompt)
{
    while (true)
    {
        string s = PromptLine(prompt);
        stringstream ss(s);
        int x;
        if (ss >> x && ss.eof()) return x;
        PrintLine("Enter a valid number.");
    }
}

bool FtxuiIO::PromptYesNo(const string& prompt)
{
    // Local screen per prompt prevents "random characters" from appearing
    // due to leftover buffered input/events.
    ScreenInteractive screen = ScreenInteractive::Fullscreen();

    bool result = false;
    bool decided = false;

    auto yes_btn = Button("Yes", [&] { result = true; decided = true; screen.Exit(); });
    auto no_btn = Button("No", [&] { result = false; decided = true; screen.Exit(); });

    auto container = Container::Horizontal({ yes_btn, no_btn });

    auto renderer = Renderer(container, [&] {
        return vbox({
            text("Banker's Algorithm") | bold,
            separator(),
            paragraph(prompt),
            separator(),
            hbox({
                yes_btn->Render(),
                text("  "),
                no_btn->Render(),
            }),
            separator()
            }) | border | size(WIDTH, GREATER_THAN, 80);
        });

    screen.Loop(renderer);

    // If user closed terminal unexpectedly, default false.
    if (!decided) return false;
    return result;
}

// full screen display
void FtxuiIO::ShowMessage(const string& title, const string& body)
{
    ScreenInteractive screen = ScreenInteractive::Fullscreen();

    auto ok_btn = Button("OK", [&] { screen.Exit(); });
    auto container = Container::Vertical({ ok_btn });

    auto renderer = Renderer(container, [&] {
        return vbox({
            text(title) | bold,
            separator(),
            paragraph(body) | border | size(HEIGHT, GREATER_THAN, 10),
            separator(),
            ok_btn->Render()
            }) | border | size(WIDTH, GREATER_THAN, 90);
        });

screen.Loop(renderer);
}

void FtxuiIO::RunTextPrompt(const string& prompt, const string& placeholder, string& out)
{
    // Local screen per prompt prevents stray buffered input from showing up.
    ScreenInteractive screen = ScreenInteractive::Fullscreen();

    string input; // starts empty

    auto input_component = Input(&input, placeholder);
    auto ok_button = Button("OK", [&] { screen.Exit(); });
    auto clear_button = Button("Clear Output", [&] { ClearOutput(); });

    auto container = Container::Vertical({
        input_component,
        Container::Horizontal({ ok_button, clear_button }),
        });

    auto renderer = Renderer(container, [&] {
        return vbox({
            text("Banker's Algorithm") | bold,
            separator(),
            paragraph(prompt),
            input_component->Render() | border,
            separator(),
            hbox({
                ok_button->Render(),
                text("  "),
                clear_button->Render(),
            }),
            separator(),
            text("Output:"),
            paragraph(JoinLines()) | border | size(HEIGHT, LESS_THAN, 10),
            }) | border | size(WIDTH, GREATER_THAN, 80);
        });

    screen.Loop(renderer);
    out = input;
}

string FtxuiIO::JoinLines() const
{
    string out;

    for (const auto& l : lines_)
    {
        out += l + "\n";
    }

    if (out.empty())
    {
        out = "(no output)";
    }

    return out;
}