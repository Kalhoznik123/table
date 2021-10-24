#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include "sheet.h"

Cell::Cell(Sheet& sheet)
    :sheet_(sheet)
    ,impl_(std::make_unique<EmptyImpl>()){}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    using namespace std::literals;
    if (text.empty()) {
        UpdateRef();
        InvalidateCach();
        impl_ = std::make_unique<EmptyImpl>();
    } else if (text[0] == '=' && text.size() > 1) {
        std::unique_ptr<Impl> impl;
        try {
            impl = std::make_unique<FormulaImpl>(text.substr(1, text.size() - 1), sheet_);
        }  catch (std::exception&) {
            throw FormulaException("ERROR:incorrect formula syntaxis"s);
        }
        // чекаем на кольцевые ссылки
        CheckLoopReferences(dynamic_cast<FormulaImpl*>(impl.get())->GetReferencedCells(), this);
        UpdateRef();
        InvalidateCach();
        impl_.swap(impl);
    }
    else {
        // тут конструирум текст
        UpdateRef();
        InvalidateCach();
        impl_ = std::make_unique<TextImpl>(std::move(text));
    }

}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    if(!prev_value)
        prev_value = impl_->GetValue();

    return *prev_value;
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const{

    std::vector<Position> referenced_cells;
    if (const auto* cell = dynamic_cast<FormulaImpl*>(impl_.get())) {
        referenced_cells  =cell->GetReferencedCells();
    }
    return referenced_cells;
}

bool Cell::IsReferenced() const{
    return !parent_nodes_.empty();
}


void Cell::UpdateRef(){

    for (CellInterface* child : child_nodes_) {
        Cell* child_cell = dynamic_cast<Cell*>(child);
        child_cell->child_nodes_.erase(this);

    }
    //отчищаем свои зависимости
    child_nodes_.clear();

    std::vector<Position> child_positions = GetReferencedCells();
    // обновляем связи в графе
    for (const Position& child_position : child_positions) {
        // вставляем в список указатель на ребёнка
        const auto it = child_nodes_.insert(sheet_.GetCell(child_position)).first;
        // добавляем в ребёнка себя
        dynamic_cast<Cell*>(*it)->parent_nodes_.insert(this);
    }
}

void Cell::InvalidateCach(){
    //отчищаем свой кэш
    if(prev_value){
        prev_value = std::nullopt;
    }
    //если нет родителей т.е тех кто от нас зависит делаем return - выход из рекурсии
    if(!IsReferenced()){
        return;
    }
    //запускаем рекурсию по своим родителям с целью инвалидировать кэш у них
    for(CellInterface* cell_interface: parent_nodes_){

        Cell* cell  = dynamic_cast<Cell*>(cell_interface);
        if(cell){
            cell->InvalidateCach();
        }
    }


}

void Cell::CheckLoopReferences(const std::vector<Position>& leaves, const CellInterface* root) {
    using namespace std::literals;
    for (const Position& leaflet : leaves) {
        const auto fd = sheet_.GetCell(leaflet);
        if (!fd) {
            sheet_.SetCell(leaflet, "");
        }
        if (fd == root) {
            throw CircularDependencyException("ERROR: loop reference detected"s);
        }
        // рекурсия есть рекурсия!
        CheckLoopReferences(sheet_.GetCell(leaflet)->GetReferencedCells(), root);
    }
}
