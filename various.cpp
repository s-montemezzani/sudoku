#include "various.hpp"
#include <vector>
#include <stdio.h>
#include <iostream>

//row of the cell
//if transpose==true, we return the row of the cell in the transposed sudoku, i.e. the column
int Cell::row(bool const& transpose) const {
    int trueindex = transpose ? transpose_cell_index(index) : index;
    return trueindex/9;

                //Note: equivalent to
                //    if (transpose) return column();
                //    else return index/9;
}

//column of the cell
//if transpose==true, we return the column of the cell in the transposed sudoku, i.e. the row
int Cell::column(bool const& transpose) const {
    int trueindex = transpose ? transpose_cell_index(index) : index;
    return trueindex%9;
}

//box of the cell
//if transpose==true, we return the box of the cell in the transposed sudoku
int Cell::box(bool const& transpose) const {
    int trueindex = transpose ? transpose_cell_index(index) : index;
    return (trueindex / 27) * 3 + (trueindex % 9) / 3;
}

int Cell::number_of_candidates() const {
    int count=0;
    for (int i=0;i<9;++i) if (candidates[i]) ++count;
    return count;
}
std::vector<int> Cell::candidate_list() const {
    std::vector<int> list;
    for (int i=0;i<9;++i) if (candidates[i]) list.push_back(i);
    return list;
}

//return vector containing index of the not found cells in the row
//if transpose==true, it considers the corresponding row in the transposed sudoku, i.e. the column
std::vector<Cell*> Sudoku::empty_cells_row(int const& row,bool const& transpose) {

    std::vector<Cell*> result;
    for (int i=0;i<9;i++) {
        int index=cell_xy(row,i);
        int trueindex = transpose ? transpose_cell_index(index) : index;

        if (!cell[trueindex].found) result.push_back(&cell[trueindex]);
    }
    return result;

}
//return vector containing index of the not found cells in the column
//if transpose==true, it considers the corresponding column in the transposed sudoku, i.e. the row
std::vector<Cell*> Sudoku::empty_cells_column(int const& column,bool const& transpose) {
    std::vector<Cell*> result;
    for (int i=0;i<9;i++) {
        int index=cell_xy(i,column);
        int trueindex = transpose ? transpose_cell_index(index) : index;
        if (!cell[trueindex].found) result.push_back(&cell[trueindex]);
    }
    return result;
}
//return vector containing index of the not found cells in the box
//if transpose==true, it considers the corresponding box in the transposed sudoku
    std::vector<Cell*> Sudoku::empty_cells_box(int const& box, bool const& transpose) {
    //if transpose=true, it means we want the box of the transpose sudoku
    //i.e. boxes look like     0 3 6
    //                         1 4 7
    //                         2 5 8

    std::vector<Cell*> result;
    for (int i=0;i<9;i++) {
        int index=cell2_xy(box,i);
        int trueindex = transpose ? transpose_cell_index(index) : index;
        if (!cell[trueindex].found) result.push_back(&cell[trueindex]);
    }
    return result;
}



//index of the corresponding cell in the transposed sudoku
int transpose_cell_index(int const& cell) {
    return cell % 9 * 9 + cell / 9;
}

//index of cell depending on row and column
//i.e.
// 0 1  2  3  ... 8
// 9 10 11 12 ... 17
// ...
int cell_xy(int const&row,int const&column) {
     return row*9+column;
}

//index of cell depending on box and "in box"-index
//i.e. inside the box indexes look like
// 0 1 2
// 3 4 5
// 6 7 8
int cell2_xy(int const& box,int const&cell) {
    return (box/3)*27+(box%3)*3+(cell/3)*9+cell%3;
}


bool is_subset(std::vector<int> const& a,std::vector<int> const& b) {
    if (a.size()>b.size()) return false;
    for (auto e : a) {
        bool found=false;
        for (auto f:b) {
            if (e==f) {
                found=true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}


std::vector<int> candidates_in_cells(std::vector<Cell*> const& cells) {
    std::vector<int> list;
    for (int i=0;i<9;i++) {
        for (auto cell : cells) {
            if (cell->candidates[i]) {
                list.push_back(i);

                //avoids adding the same number more than once
                break;
            }
        }
    }
    return list;
}
std::vector<int> intersection(std::vector<int> const& cells1, std::vector<int> const& cells2) {
    std::vector<int> list;
    for (auto cell:cells1) {
        if (is_member(cell,cells2)) list.push_back(cell);
    }
    return list;
}

void remove_candidate(Cell* const& cell, int const& candidate) {
    cell->candidates[candidate]=false;
}
void assign(Cell* const & cell, int const& value) {
	cell->value=value;
	cell->found=true;

    //TODO: add const in pointers whenever possible
	Sudoku* const sudoku= cell->sudoku;

	//remove other candidates from cell
	for (int i=0;i<9;i++) if (i != value) {
            remove_candidate(cell,i);
    }
	//remove candidate from row
	for (int i=0;i<9;i++) {
            int int_temp_cell=cell_xy(cell->row(),i);
            //Cell* temp_cell=
            if (int_temp_cell != cell->index)
                remove_candidate(&sudoku->cell[int_temp_cell],value);
	}

	//remove candidate from column
    for (int i=0;i<9;i++) {
            int int_temp_cell=cell_xy(i,cell->column());
            if (int_temp_cell != cell->index)
                remove_candidate(&sudoku->cell[int_temp_cell],value);
	}

	//remove candidate from box
    for (int i=0;i<9;i++) {
            int int_temp_cell=cell2_xy(cell->box(),i);
            if (int_temp_cell != cell->index)
                remove_candidate(&sudoku->cell[int_temp_cell],value);
	}

}

void import_from_file(Sudoku& arg_sudoku) {
    FILE * pFile;
    char c;
    int count=0;

    pFile = fopen ("sudoku.txt" , "r");
    if (pFile == NULL) perror ("Error opening file");
    else
    {
        while (!feof(pFile))
        {
            if ((c=fgetc(pFile)) == EOF) break;
            if (c!='\n') {
                if (c!='.') {

                    //since c is char, (int) c - '0' turns into the corresponding integer. -1 because we use indexes 0 to 8
                    assign(&arg_sudoku.cell[count],(int) c - '0' - 1);
                }
                count++;
            }
        }
        fclose (pFile);
    }
}

void print(Sudoku const& arg_sudoku) {
	for (int i=0;i<81;i++) {
		if (arg_sudoku.cell[i].found) std::cout << arg_sudoku.cell[i].value+1;
		else std::cout << ".";
        if (i%9==8) std::cout << "\n";
		else if (i%3==2) std::cout << " ";
		if (i==26||i==53) std::cout << "\n";
	}
}
void print_full(Sudoku const& arg_sudoku) {
	for (int i=0;i<81;i++) {
		for (int j=0;j<9;j++) {
			if (arg_sudoku.cell[i].candidates[j]) std::cout << j+1;
			else std::cout << " ";
        }
		std::cout << " ";

		if (i%9==8) std::cout << "\n";
		else if (i%3==2) std::cout << "| ";
		if (i==26||i==53) std::cout << "------------------------------+-------------------------------+------------------------------\n";
	}
}

//returns subsets of size k of v
std::vector<std::vector<int>> subsets(int k,std::vector<int> const& v,int skip) {
    //note: skip=0 by default (see various.hpp)
    std::vector<std::vector<int>> list;
    if (k == 0 || ((v.end()-v.begin()) - skip)<k ) return {{}};
    int first=v[skip];
    skip++;
    for (std::vector<int> e:subsets(k-1,v,skip)) {
        e.insert(e.begin(),first);
        list.push_back(e);
    }
    for (std::vector<int> e:subsets(k,v,skip)) {
        if (!e.empty()) list.push_back(e);
    }

    return list;
}
