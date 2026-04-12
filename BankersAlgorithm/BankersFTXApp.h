#pragma once

#include "state.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

class BankersFTXApp {
public:
    explicit BankersFTXApp(state& sim_state)
        : sim_state_(sim_state),
        screen_(ftxui::ScreenInteractive::Fullscreen()) {
        BuildCountScreen();
        BuildEmptyMatrixScreen();
        BuildEmptyResultScreen();
        BuildRoot();
    }

    void Run() {
        screen_.Loop(root_);
    }

private:
    struct CellInput {
        std::string value = "0";
        bool invalid = false;
    };

    struct Snapshot {
        std::vector<int> avail;
        std::vector<std::vector<int>> claim;
        std::vector<std::vector<int>> alloc;
        std::vector<std::vector<int>> need;
    };

    enum class View {
        Counts,
        Matrices,
        Results
    };

    class DynamicRoot : public ftxui::ComponentBase {
    public:
        explicit DynamicRoot(BankersFTXApp* app) : app_(app) {}

        ftxui::Element OnRender() override {
            auto active = app_->GetActiveScreen();
            if (!active) {
                return ftxui::text("No active screen.");
            }
            return active->Render();
        }

        bool OnEvent(ftxui::Event event) override {
            auto active = app_->GetActiveScreen();
            if (!active) {
                return false;
            }
            return active->OnEvent(event);
        }

        ftxui::Component ActiveChild() override {
            return app_->GetActiveScreen();
        }

        bool Focusable() const override {
            return true;
        }

    private:
        BankersFTXApp* app_;
    };

private:
    state& sim_state_;
    ftxui::ScreenInteractive screen_;
    ftxui::Component root_;

    ftxui::Component count_screen_;
    ftxui::Component matrix_screen_;
    ftxui::Component result_screen_;

    View current_view_ = View::Counts;

    std::string process_count_str_ = "3";
    std::string resource_count_str_ = "3";
    std::string message_;

    std::vector<CellInput> resource_inputs_;
    std::vector<std::vector<CellInput>> claim_inputs_;
    std::vector<std::vector<CellInput>> alloc_inputs_;

    std::vector<int> base_resource_;
    std::vector<std::vector<int>> base_claim_;
    std::vector<std::vector<int>> base_alloc_;

    std::vector<int> execution_path_;
    std::vector<Snapshot> snapshots_;
    int current_step_ = 0;

private:
    ftxui::Component GetActiveScreen() {
        switch (current_view_) {
        case View::Counts:
            return count_screen_;
        case View::Matrices:
            return matrix_screen_;
        case View::Results:
            return result_screen_;
        default:
            return count_screen_;
        }
    }

    static int ToInt(const std::string& s) {
        try {
            if (s.empty()) {
                return 0;
            }
            return std::stoi(s);
        }
        catch (...) {
            return 0;
        }
    }

    static std::string MakeLabel(const std::string& prefix, int row, int col) {
        return prefix + "[" + std::to_string(row) + "][" + std::to_string(col) + "]";
    }

    static std::string MakeVectorLabel(const std::string& prefix, int index) {
        return prefix + "[" + std::to_string(index) + "]";
    }

    bool BlockNonDigits(ftxui::Event e) {
        if (e.is_character()) {
            const std::string ch = e.character();
            if (ch.size() == 1 && (ch[0] < '0' || ch[0] > '9')) {
                return true;
            }
        }
        return false;
    }

    void BuildRoot() {
        root_ = std::make_shared<DynamicRoot>(this);
    }

    void ClearInvalidFlags() {
        for (auto& cell : resource_inputs_) {
            cell.invalid = false;
        }

        for (auto& row : claim_inputs_) {
            for (auto& cell : row) {
                cell.invalid = false;
            }
        }

        for (auto& row : alloc_inputs_) {
            for (auto& cell : row) {
                cell.invalid = false;
            }
        }
    }

    void InitializeMatrixInputs() {
        const int rcount = sim_state_.get_rcount();
        const int pcount = sim_state_.get_pcount();

        resource_inputs_.assign(rcount, CellInput{});
        claim_inputs_.assign(pcount, std::vector<CellInput>(rcount));
        alloc_inputs_.assign(pcount, std::vector<CellInput>(rcount));

        for (int j = 0; j < rcount; ++j) {
            resource_inputs_[j].value = "0";
            resource_inputs_[j].invalid = false;
        }

        for (int i = 0; i < pcount; ++i) {
            for (int j = 0; j < rcount; ++j) {
                claim_inputs_[i][j].value = "0";
                claim_inputs_[i][j].invalid = false;

                alloc_inputs_[i][j].value = "0";
                alloc_inputs_[i][j].invalid = false;
            }
        }
    }

    void LoadUIIntoStateAndCache() {
        const int rcount = sim_state_.get_rcount();
        const int pcount = sim_state_.get_pcount();

        base_resource_.assign(rcount, 0);
        base_claim_.assign(pcount, std::vector<int>(rcount, 0));
        base_alloc_.assign(pcount, std::vector<int>(rcount, 0));

        for (int j = 0; j < rcount; ++j) {
            base_resource_[j] = ToInt(resource_inputs_[j].value);
        }

        for (int i = 0; i < pcount; ++i) {
            for (int j = 0; j < rcount; ++j) {
                base_claim_[i][j] = ToInt(claim_inputs_[i][j].value);
                base_alloc_[i][j] = ToInt(alloc_inputs_[i][j].value);
            }
        }

        sim_state_.set_resource(base_resource_);
        sim_state_.set_claim(base_claim_);
        sim_state_.set_alloc(base_alloc_);
    }

    std::vector<int> ComputeInitialAvail(
        const std::vector<int>& resource,
        const std::vector<std::vector<int>>& alloc) {
        const int pcount = static_cast<int>(alloc.size());
        const int rcount = static_cast<int>(resource.size());

        std::vector<int> avail(rcount, 0);

        for (int r = 0; r < rcount; ++r) {
            int sum = 0;
            for (int p = 0; p < pcount; ++p) {
                sum += alloc[p][r];
            }
            avail[r] = resource[r] - sum;
        }

        return avail;
    }

    std::vector<std::vector<int>> ComputeNeed(
        const std::vector<std::vector<int>>& claim,
        const std::vector<std::vector<int>>& alloc) {
        const int pcount = static_cast<int>(claim.size());
        const int rcount = pcount > 0 ? static_cast<int>(claim[0].size()) : 0;

        std::vector<std::vector<int>> need(pcount, std::vector<int>(rcount, 0));

        for (int p = 0; p < pcount; ++p) {
            for (int r = 0; r < rcount; ++r) {
                need[p][r] = claim[p][r] - alloc[p][r];
            }
        }

        return need;
    }

    void BuildSnapshots() {
        snapshots_.clear();
        current_step_ = 0;

        if (base_claim_.empty() || base_alloc_.empty() || base_resource_.empty()) {
            return;
        }

        auto claim = base_claim_;
        auto alloc = base_alloc_;
        auto avail = ComputeInitialAvail(base_resource_, alloc);

        Snapshot initial;
        initial.claim = claim;
        initial.alloc = alloc;
        initial.need = ComputeNeed(claim, alloc);
        initial.avail = avail;
        snapshots_.push_back(initial);

        for (int process : execution_path_) {
            if (process < 0 || process >= static_cast<int>(alloc.size())) {
                break;
            }

            for (int r = 0; r < static_cast<int>(avail.size()); ++r) {
                avail[r] += alloc[process][r];
                alloc[process][r] = 0;
                claim[process][r] = 0;
            }

            Snapshot step;
            step.claim = claim;
            step.alloc = alloc;
            step.need = ComputeNeed(claim, alloc);
            step.avail = avail;
            snapshots_.push_back(step);
        }
    }

    std::string BuildExecutionOrderText() const {
        if (execution_path_.empty()) {
            return "No safe execution order was produced.";
        }

        std::string order;
        for (int i = 0; i < static_cast<int>(execution_path_.size()); ++i) {
            order += "P" + std::to_string(execution_path_[i] + 1);
            if (i + 1 != static_cast<int>(execution_path_.size())) {
                order += " -> ";
            }
        }
        return order;
    }

    ftxui::Element BuildHeaderRow() const {
        using namespace ftxui;

        Elements headers;
        headers.push_back(text("") | size(WIDTH, EQUAL, 6));

        for (int j = 0; j < sim_state_.get_rcount(); ++j) {
            headers.push_back(
                text("R" + std::to_string(j)) |
                bold |
                center |
                size(WIDTH, EQUAL, 6)
            );
        }

        return hbox(std::move(headers));
    }

    ftxui::Element RenderMatrixBox(
        const std::string& title,
        const std::vector<std::vector<int>>& matrix) const {
        using namespace ftxui;

        Elements rows;
        rows.push_back(text(title) | bold | center);
        rows.push_back(separator());
        rows.push_back(BuildHeaderRow());

        for (int p = 0; p < static_cast<int>(matrix.size()); ++p) {
            Elements row;
            row.push_back(text("P" + std::to_string(p)) | size(WIDTH, EQUAL, 6));

            for (int r = 0; r < static_cast<int>(matrix[p].size()); ++r) {
                row.push_back(
                    text(std::to_string(matrix[p][r])) |
                    size(WIDTH, EQUAL, 6) |
                    center
                );
            }

            rows.push_back(hbox(std::move(row)));
        }

        return vbox(std::move(rows)) | border | flex;
    }

    ftxui::Element RenderVectorBox(
        const std::string& title,
        const std::vector<int>& vec,
        const std::string& left_label) const {
        using namespace ftxui;

        Elements row;
        row.push_back(text(left_label) | size(WIDTH, EQUAL, 6));

        for (int r = 0; r < static_cast<int>(vec.size()); ++r) {
            row.push_back(
                text(std::to_string(vec[r])) |
                size(WIDTH, EQUAL, 6) |
                center
            );
        }

        return vbox({
                   text(title) | bold,
                   separator(),
                   BuildHeaderRow(),
                   hbox(std::move(row)),
            }) |
            border;
    }

    void BuildCountScreen() {
        using namespace ftxui;

        auto process_input = Input(&process_count_str_, "# Processes");
        auto resource_input = Input(&resource_count_str_, "# Resources");

        process_input |= CatchEvent([this](Event e) {
            return BlockNonDigits(e);
            });

        resource_input |= CatchEvent([this](Event e) {
            return BlockNonDigits(e);
            });

        auto submit_button = Button("Create Input Matrices", [this] {
            const int pcount = ToInt(process_count_str_);
            const int rcount = ToInt(resource_count_str_);

            if (pcount <= 0 || rcount <= 0) {
                message_ = "Process count and resource count must both be greater than 0.";
                screen_.PostEvent(ftxui::Event::Custom);
                return;
            }

            sim_state_.set_dimensions(rcount, pcount);
            InitializeMatrixInputs();
            BuildMatrixScreen();

            message_.clear();
            current_view_ = View::Matrices;
            screen_.PostEvent(ftxui::Event::Custom);
            });

        auto container = Container::Vertical({
            process_input,
            resource_input,
            submit_button
            });

        count_screen_ = Renderer(
            container,
            [this, process_input, resource_input, submit_button] {
                using namespace ftxui;

                return vbox({
                    text("Banker's Algorithm Setup") | bold | center,
                    separator(),
                    text("Enter the number of processes and resources."),
                    separator(),
                    hbox({
                        text("Processes: ") | size(WIDTH, EQUAL, 14),
                        process_input->Render() | size(WIDTH, EQUAL, 12),
                    }),
                    hbox({
                        text("Resources: ") | size(WIDTH, EQUAL, 14),
                        resource_input->Render() | size(WIDTH, EQUAL, 12),
                    }),
                    separator(),
                    submit_button->Render(),
                    separator(),
                    paragraph(message_) | border,
                    }) | border | size(WIDTH, GREATER_THAN, 60);
            }
        );
    }

    void BuildEmptyMatrixScreen() {
        using namespace ftxui;

        auto back_button = Button("Back", [this] {
            current_view_ = View::Counts;
            screen_.PostEvent(ftxui::Event::Custom);
            });

        auto container = Container::Vertical({
            back_button
            });

        matrix_screen_ = Renderer(
            container,
            [back_button] {
                using namespace ftxui;

                return vbox({
                    text("Banker's Algorithm Input") | bold | center,
                    separator(),
                    text("Enter the counts first."),
                    separator(),
                    back_button->Render(),
                    }) | border;
            }
        );
    }

    void BuildEmptyResultScreen() {
        using namespace ftxui;

        auto back_button = Button("Back", [this] {
            current_view_ = View::Matrices;
            screen_.PostEvent(ftxui::Event::Custom);
            });

        auto container = Container::Vertical({
            back_button
            });

        result_screen_ = Renderer(
            container,
            [back_button] {
                using namespace ftxui;

                return vbox({
                    text("Banker's Algorithm Results") | bold | center,
                    separator(),
                    text("Run the simulation first."),
                    separator(),
                    back_button->Render(),
                    }) | border;
            }
        );
    }

    void BuildMatrixScreen() {
        using namespace ftxui;

        const int rcount = sim_state_.get_rcount();
        const int pcount = sim_state_.get_pcount();

        Components resource_input_components;
        for (int j = 0; j < rcount; ++j) {
            auto input = Input(&resource_inputs_[j].value, MakeVectorLabel("Res", j));
            input |= CatchEvent([this](Event e) {
                return BlockNonDigits(e);
                });
            resource_input_components.push_back(input);
        }

        auto resource_row_container = Container::Horizontal(resource_input_components);

        auto resource_section = Renderer(
            resource_row_container,
            [this, resource_input_components] {
                using namespace ftxui;

                Elements row;
                row.push_back(text("Total") | size(WIDTH, EQUAL, 6));

                for (int j = 0; j < sim_state_.get_rcount(); ++j) {
                    row.push_back(
                        resource_input_components[j]->Render() | size(WIDTH, EQUAL, 8)
                    );
                }

                return vbox({
                    text("Resource Vector") | bold,
                    separator(),
                    BuildHeaderRow(),
                    hbox(std::move(row)),
                    }) | border;
            }
        );

        Components claim_row_components;
        for (int i = 0; i < pcount; ++i) {
            Components row_inputs;

            for (int j = 0; j < rcount; ++j) {
                auto input = Input(&claim_inputs_[i][j].value, MakeLabel("C", i, j));
                input |= CatchEvent([this](Event e) {
                    return BlockNonDigits(e);
                    });
                row_inputs.push_back(input);
            }

            auto row_container = Container::Horizontal(row_inputs);

            auto row_renderer = Renderer(
                row_container,
                [this, i, row_inputs] {
                    using namespace ftxui;

                    Elements row;
                    row.push_back(text("P" + std::to_string(i)) | size(WIDTH, EQUAL, 6));

                    for (int j = 0; j < sim_state_.get_rcount(); ++j) {
                        row.push_back(
                            row_inputs[j]->Render() | size(WIDTH, EQUAL, 8)
                        );
                    }

                    return hbox(std::move(row));
                }
            );

            claim_row_components.push_back(row_renderer);
        }

        auto claim_table_container = Container::Vertical(claim_row_components);

        auto claim_section = Renderer(
            claim_table_container,
            [this, claim_row_components] {
                using namespace ftxui;

                Elements rows;
                rows.push_back(text("Claim Matrix") | bold);
                rows.push_back(separator());
                rows.push_back(BuildHeaderRow());

                for (auto& row : claim_row_components) {
                    rows.push_back(row->Render());
                }

                return vbox(std::move(rows)) | border;
            }
        );

        Components alloc_row_components;
        for (int i = 0; i < pcount; ++i) {
            Components row_inputs;

            for (int j = 0; j < rcount; ++j) {
                auto input = Input(&alloc_inputs_[i][j].value, MakeLabel("A", i, j));
                input |= CatchEvent([this](Event e) {
                    return BlockNonDigits(e);
                    });
                row_inputs.push_back(input);
            }

            auto row_container = Container::Horizontal(row_inputs);

            auto row_renderer = Renderer(
                row_container,
                [this, i, row_inputs] {
                    using namespace ftxui;

                    Elements row;
                    row.push_back(text("P" + std::to_string(i)) | size(WIDTH, EQUAL, 6));

                    for (int j = 0; j < sim_state_.get_rcount(); ++j) {
                        auto cell = row_inputs[j]->Render() | size(WIDTH, EQUAL, 8);

                        if (alloc_inputs_[i][j].invalid) {
                            cell = cell | bgcolor(Color::Red) | color(Color::White);
                        }

                        row.push_back(cell);
                    }

                    return hbox(std::move(row));
                }
            );

            alloc_row_components.push_back(row_renderer);
        }

        auto alloc_table_container = Container::Vertical(alloc_row_components);

        auto alloc_section = Renderer(
            alloc_table_container,
            [this, alloc_row_components] {
                using namespace ftxui;

                Elements rows;
                rows.push_back(text("Allocation Matrix") | bold);
                rows.push_back(separator());
                rows.push_back(BuildHeaderRow());

                for (auto& row : alloc_row_components) {
                    rows.push_back(row->Render());
                }

                return vbox(std::move(rows)) | border;
            }
        );

        auto back_button = Button("Back", [this] {
            message_.clear();
            current_view_ = View::Counts;
            screen_.PostEvent(ftxui::Event::Custom);
            });

        auto validate_button = Button("Validate / Store", [this] {
            ClearInvalidFlags();
            LoadUIIntoStateAndCache();

            std::vector<std::pair<int, int>> bad_cells;
            const bool valid = sim_state_.validate_alloc_le_claim(bad_cells);

            if (!valid) {
                for (const auto& bad : bad_cells) {
                    const int r = bad.first;
                    const int c = bad.second;

                    if (r >= 0 &&
                        r < static_cast<int>(alloc_inputs_.size()) &&
                        c >= 0 &&
                        c < static_cast<int>(alloc_inputs_[r].size())) {
                        alloc_inputs_[r][c].invalid = true;
                    }
                }

                message_ = "Allocation cannot be greater than claim. Invalid allocation cells are highlighted.";
                screen_.PostEvent(ftxui::Event::Custom);
                return;
            }

            message_ = "Input stored in state. You can now run the simulation.";
            screen_.PostEvent(ftxui::Event::Custom);
            });

        auto run_button = Button("Run Simulation", [this] {
            ClearInvalidFlags();
            LoadUIIntoStateAndCache();

            std::vector<std::pair<int, int>> bad_cells;
            const bool valid = sim_state_.validate_alloc_le_claim(bad_cells);

            if (!valid) {
                for (const auto& bad : bad_cells) {
                    const int r = bad.first;
                    const int c = bad.second;

                    if (r >= 0 &&
                        r < static_cast<int>(alloc_inputs_.size()) &&
                        c >= 0 &&
                        c < static_cast<int>(alloc_inputs_[r].size())) {
                        alloc_inputs_[r][c].invalid = true;
                    }
                }

                message_ = "Allocation cannot be greater than claim. Invalid allocation cells are highlighted.";
                screen_.PostEvent(ftxui::Event::Custom);
                return;
            }

            sim_state_.prepare_for_simulation();
            sim_state_.sim();

            execution_path_ = sim_state_.get_path();
            BuildSnapshots();
            BuildResultScreen();

            current_view_ = View::Results;
            screen_.PostEvent(ftxui::Event::Custom);
            });

        auto button_row = Container::Horizontal({
            back_button,
            validate_button,
            run_button
            });

        auto main_container = Container::Vertical({
            resource_section,
            claim_section,
            alloc_section,
            button_row
            });

        matrix_screen_ = Renderer(
            main_container,
            [this, resource_section, claim_section, alloc_section, back_button, validate_button, run_button] {
                using namespace ftxui;

                return vbox({
                    text("Banker's Algorithm Input") | bold | center,
                    separator(),
                    hbox({
                        text("Processes: " + std::to_string(sim_state_.get_pcount())),
                        separator(),
                        text("Resources: " + std::to_string(sim_state_.get_rcount())),
                    }),
                    separator(),
                    resource_section->Render(),
                    separator(),
                    claim_section->Render(),
                    separator(),
                    alloc_section->Render(),
                    separator(),
                    hbox({
                        back_button->Render() | flex,
                        validate_button->Render() | flex,
                        run_button->Render() | flex,
                    }),
                    separator(),
                    paragraph(message_) | border,
                    }) | border | size(WIDTH, GREATER_THAN, 100);
            }
        );
    }

    void BuildResultScreen() {
        using namespace ftxui;

        auto prev_button = Button("Previous Step", [this] {
            if (current_step_ > 0) {
                --current_step_;
            }
            screen_.PostEvent(ftxui::Event::Custom);
            });

        auto next_button = Button("Next Step", [this] {
            if (current_step_ + 1 < static_cast<int>(snapshots_.size())) {
                ++current_step_;
            }
            screen_.PostEvent(ftxui::Event::Custom);
            });

        auto back_button = Button("Back to Input", [this] {
            current_view_ = View::Matrices;
            screen_.PostEvent(ftxui::Event::Custom);
            });

        auto container = Container::Vertical({
            Container::Horizontal({
                prev_button,
                next_button,
                back_button
            })
            });

        result_screen_ = Renderer(
            container,
            [this, prev_button, next_button, back_button] {
                using namespace ftxui;

                if (snapshots_.empty()) {
                    return vbox({
                        text("Banker's Algorithm Results") | bold | center,
                        separator(),
                        text("No simulation results are available."),
                        separator(),
                        back_button->Render(),
                        }) | border;
                }

                const Snapshot& snap = snapshots_[current_step_];

                std::string step_text =
                    "Step " + std::to_string(current_step_) + " of " +
                    std::to_string(static_cast<int>(snapshots_.size()) - 1);

                std::string process_text;
                if (static_cast<int>(execution_path_.size()) == 0)
                {
                    process_text = "Deadlock, no processes can complete.";
                }
                else if (current_step_ < static_cast<int>(execution_path_.size())) {
                    process_text = "Process completed at this step: P" + std::to_string(execution_path_[current_step_] + 1);
                }
                else {
                    process_text = "All listed processes have completed.";
                }

                std::string order_text = BuildExecutionOrderText();

                return vbox({
                    text("Banker's Algorithm Results") | bold | center,
                    separator(),
                    text("Execution Order: " + order_text),
                    text(step_text),
                    text(process_text),
                    separator(),
                    RenderVectorBox("Available Vector", snap.avail, "Avail"),
                    separator(),
                    hbox({
                        RenderMatrixBox("Claim Matrix", snap.claim),
                        RenderMatrixBox("Allocation Matrix", snap.alloc),
                        RenderMatrixBox("Need Matrix", snap.need),
                    }),
                    separator(),
                    hbox({
                        prev_button->Render() | flex,
                        next_button->Render() | flex,
                        back_button->Render() | flex,
                    }),
                    }) | border | size(WIDTH, GREATER_THAN, 120);
            }
        );
    }
};