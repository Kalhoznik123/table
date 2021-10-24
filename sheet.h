#pragma once

#include "cell.h"
#include "common.h"
#include <map>
#include <functional>

class Sheet : public SheetInterface {
    using Row = std::vector<std::unique_ptr<CellInterface>>;
    using Table = std::vector<Row>;
public:

    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

	// Можете дополнить ваш класс нужными полями и методами


private:
    std::string GetCellValue(Position pos) const;

    void ResizeTable(Size new_size);

    // контейнер ячеек
    Table table_;
    // печатная область
    Size size_;

    // фиксируем здесь сколько ячеек в каждом столбце или строке
    std::map<int, int> row_to_cells_count_;
    std::map<int, int> col_to_cells_count_;
};
