#include "various.hpp"
#include <vector>
#include <stdio.h>
#include <iostream>
int Cell::number_of_candidates() {
    int count=0;
    for (int i=0;i<9;++i) if (candidates[i]) ++count;
    return count;
}
std::vector<int> Cell::candidate_list() {
    std::vector<int> list;
    for (int i=0;i<9;++i) if (candidates[i]) list.push_back(i);
    return list;
}
std::vector<int> Sudoku::empty_cells_row(int const& row) {
    std::vector<int> result;
    for (int i=0;i<9;i++) if (!cell[cell_xy(row,i)].found) result.push_back(cell_xy(row,i));
    return result;
}

std::vector<int> Sudoku::empty_cells_column(int const& column) {
    std::vector<int> result;
    for (int i=0;i<9;i++) if (!cell[cell_xy(i,column)].found) result.push_back(cell_xy(i,column));
    return result;
}
std::vector<int> Sudoku::empty_cells_box(int const& box) {
    std::vector<int> result;
    for (int i=0;i<9;i++) if (!cell[cell2_xy(box,i)].found) result.push_back(cell2_xy(box,i));
    return result;
}



//index of the corresponding cell in the transposed sudoku
int transpose_cell_index(int const& cell) {
    return cell % 9 * 9 + cell / 9;
}
//index of the corresponding box in the transposed sudoku
int transpose_box_index(int const& box) {
    return box % 3 * 3 + box / 3;
}
//index of cell depending on row and column
int cell_xy(int const&row,int const&column) {
    // 0 1  2  3 ... 8
    // 9 10 11 ... 17
    // ...
     return row*9+column;
}
//index of cell depending on box, "in box"-row and "in box"-column
int cell2_xy(int const& box,int const&cell) {
    // 0 1 2
    // 3 4 5
    // 6 7 8
    return (box/3)*27+(box%3)*3+(cell/3)*9+cell%3;
}



//returns subsets of size k of v
typedef std::vector<int> intvec;
std::vector<intvec> subsets(int k,intvec const& v,int skip) {
    //note: skip=0 by default (see various.hpp)
    std::vector<intvec> list;
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
bool is_member(int const& a,std::vector<int> const& b) {
    for (auto e:b)
        if (a==e) return true;
    return false;
}

std::vector<int> candidates_in_cells(std::vector<int> const& cells,Sudoku const& arg_sudoku) {
    std::vector<int> list;
    for (int i=0;i<9;i++) {
        for (auto e : cells) {
            if (arg_sudoku.cell[e].candidates[i]) {
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
void remove_candidate(int const& cell, int const& candidate,Sudoku& arg_sudoku) {
        arg_sudoku.cell[cell].candidates[candidate]=false;

        int cell_transpose=transpose_cell_index(cell);
        arg_sudoku.transpose->cell[cell_transpose].candidates[candidate]=false;

}
void assign(int const& cell, int const& value,Sudoku& arg_sudoku) {
    Cell& c=arg_sudoku.cell[cell];
	c.value=value;
	c.found=true;

    int cell_transpose=transpose_cell_index(cell);
    arg_sudoku.transpose->cell[cell_transpose].value=value;
	arg_sudoku.transpose->cell[cell_transpose].found=true;

	//remove other candidates from cell
	for (int i=0;i<9;i++) if (i != value) {
            remove_candidate(cell,i,arg_sudoku);
    }
	//remove candidate from row
	for (int i=0;i<9;i++) {
            int temp_cell=cell_xy(c.row(),i);
            if (temp_cell != cell)
                remove_candidate(temp_cell,value,arg_sudoku);
	}

	//remove candidate from column
    for (int i=0;i<9;i++) {
            int temp_cell=cell_xy(i,c.column());
            if (temp_cell != cell)
                remove_candidate(temp_cell,value,arg_sudoku);
	}

	//remove candidate from box
    for (int i=0;i<9;i++) {
            int temp_cell=cell2_xy(c.box(),i);
            if (temp_cell != cell)
                remove_candidate(temp_cell,value,arg_sudoku);
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
                    assign(count,(int) c - '0' - 1,arg_sudoku);
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
		if (i==26||i==53) {
			for (int k=0;k<30;k++) std::cout << "-";
				std::cout << "+";
			for (int k=0;k<31;k++) std::cout << "-";
				std::cout << "+";
			for (int k=0;k<31;k++) std::cout << "-";
				std::cout << "\n";
		}
	}
}
