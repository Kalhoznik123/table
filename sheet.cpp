#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <cmath>
#include <sstream>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
    // обновляем размер печатной области
    ++row_to_cells_count_[pos.row];
    ++col_to_cells_count_[pos.col];
    // обновляем размер таблицы
    ResizeTable({(--row_to_cells_count_.end())->first + 1, (--col_to_cells_count_.end())->first + 1});
    // присваиваем значениеi
    if (!table_.at(pos.row).at(pos.col)) {
        table_.at(pos.row).at(pos.col) = std::make_unique<Cell>(*this);
    }

    auto* cell = dynamic_cast<Cell*>(table_.at(pos.row).at(pos.col).get());
    if(cell){
        cell->Set(std::move(text));
    }

}

const CellInterface* Sheet::GetCell(Position pos) const {


    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
    if (pos.row >= size_.rows || pos.col >= size_.cols || table_.empty()) {
        return nullptr;
    }
    return table_.at(pos.row).at(pos.col).get();
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
    if (pos.row >= size_.rows || pos.col >= size_.cols || table_.empty()) {
        return nullptr;
    }
    return table_.at(pos.row).at(pos.col).get();
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
    if (pos.row > size_.rows || pos.col > size_.cols || table_.empty()) {
        return;
    }
    // что-то обновляем только если ячейка существует
    if (table_.at(pos.row).at(pos.col)) {
        table_.at(pos.row).at(pos.col) = nullptr;
        --row_to_cells_count_.at(pos.row);
        --col_to_cells_count_.at(pos.col);
        // обновляем размер печатной боласти
        if (row_to_cells_count_.at(pos.row) == 0) {
            row_to_cells_count_.erase(row_to_cells_count_.find(pos.row));
        }
        if (col_to_cells_count_.at(pos.col) == 0) {
            col_to_cells_count_.erase(col_to_cells_count_.find(pos.col));
        }
        if (row_to_cells_count_.empty() || col_to_cells_count_.empty()) {
            size_ = {0, 0};
            //table_ = Table();
        } else {
            // в данной ветке гарантировано контейнеры содержат как минимум 1 элемент, а значит с чистой душой --
            ResizeTable({(--row_to_cells_count_.end())->first + 1, (--col_to_cells_count_.end())->first + 1});
        }
    }
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::PrintValues(std::ostream& output) const {    
    for (int i = 0; i < size_.rows; ++i) {
        for (int j = 0; j < size_.cols; ++j) {
            if (table_.at(i).at(j)) {
                Cell::Value value = table_.at(i).at(j)->GetValue();
                if(std::holds_alternative<double>(value)){
                    output << std::get<double>(value);
                }else if(std::holds_alternative<FormulaError>(value)){
                    output << std::get<FormulaError>(value);
                }else{
                    output << std::get<std::string>(value);
                }

            }
            if (j < size_.cols - 1) {
                output << '\t';
            }
        }

        output << '\n';
    }

}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i) {
        for (int j = 0; j < size_.cols; ++j) {
            if (table_.at(i).at(j)) {
                output << table_.at(i).at(j)->GetText();
            }
            if (j < size_.cols - 1) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

std::string Sheet::GetCellValue(Position pos) const {
    Cell::Value value = table_.at(pos.row).at(pos.col)->GetValue();
    if (std::holds_alternative<double>(value)) {

        return std::to_string(static_cast<int>(std::round(std::get<double>(value))));

        double result = std::get<double>(value);
        if (std::abs(result - static_cast<int>(result)) < 0.0001) {
            // чтобы не было кучи нулей
            return std::to_string(static_cast<int>(std::get<double>(value)));
        } else {
            //return std::to_string(static_cast<int>(std::get<double>(value)));
            return std::to_string(std::get<double>(value));
        }
    } else if (std::holds_alternative<FormulaError>(value)) {
        std::stringstream ss;
        ss << std::get<FormulaError>(value);
        return ss.str();
    } else if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    }
    return ""s;
}

void Sheet::ResizeTable(Size new_size) {
    if (table_.size() < static_cast<size_t>(new_size.rows)) {
        for (int i = size_.rows; i < new_size.rows; ++i) {
            table_.push_back(Row(size_.cols));
        }      
    }
    size_.rows = new_size.rows;
    if (!table_.empty() && table_.at(0).size() < static_cast<size_t>(new_size.cols)) {
        for (int i = 0; i < size_.rows; ++i) {
            table_.at(i).resize(new_size.cols);
        }
    }
    size_.cols = new_size.cols;
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
