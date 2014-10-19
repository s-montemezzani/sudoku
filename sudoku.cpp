#include "various.hpp"
#include <iostream>
#include <vector>


// http://www.sadmansoftware.com/sudoku/nakedsingle.htm
// only one candidate in cell => it's the right number
bool naked_single(Sudoku& arg_sudoku) {
	for (int i=0;i<81;i++) {
        Cell* cell=&arg_sudoku.cell[i];
        // don't do anything if cell is already solved
		if (cell->found) continue;

        if (cell->number_of_candidates()==1) {
            assign(cell,cell->candidate_list()[0]);
			std::cout << "Naked single: " << cell->candidate_list()[0] << ". Cell: " << i << "\n";
			return true;
		}
	}
	return false;
}

// http://www.sadmansoftware.com/sudoku/hiddensingle.htm
// only one cell in a row / column / box contains a certain candidate => that cell must contain that number
bool hidden_single(std::vector<Cell*> const& cells);
bool hidden_single(Sudoku& arg_sudoku) {
    // for each row
    for (int i=0;i<9;++i) if (hidden_single(arg_sudoku.empty_cells_row(i))) return true;
    // for each column
    for (int i=0;i<9;++i) if (hidden_single(arg_sudoku.empty_cells_column(i))) return true;
    // for each box
    for (int i=0;i<9;++i) if (hidden_single(arg_sudoku.empty_cells_box(i))) return true;
    return false;
}
bool hidden_single(std::vector<Cell*> const& cells) {
    //for every candidate of any of the cells
    for (auto candidate:candidates_in_cells(cells)) {

        // list of cells where candidate is viable
        std::vector<Cell*> viable_cells;

        for (auto cell:cells) {
            if (cell->candidates[candidate]) {
                viable_cells.push_back(cell);
            }
        }

        if (viable_cells.size()==1) {
            //possible hidden single found

            //check we didn't already know
            if (!viable_cells[0]->found) {
                assign(viable_cells[0],candidate);
                std::cout << "Hidden single: " << candidate << ". Cell: " << viable_cells[0]->index << "\n";
                return true;
            }
        }
    }

	return false;
}

// http://www.sadmansoftware.com/sudoku/blockcolumnrow.htm
// a box only has a certain candidate in a certain row / column => candidate not viable in other cells in that same row / column

//if transpose==true, we consider the transposed sudoku. This allows us to do both "block row" and "block column" algorithms using the same function
bool block_row_interaction(std::vector<Cell*> const& cells,bool transpose=false);
bool block_row_interaction(Sudoku& arg_sudoku,bool transpose=false) {
    //for each box
    for (int i=0;i<9;++i) {
        if (block_row_interaction(arg_sudoku.empty_cells_box(i),transpose))
            return true;
    }
    return false;
}
bool block_row_interaction(std::vector<Cell*> const& box,bool transpose) {
    // need at least 2 cells (if 1, naked_single or hidden_single will find it)
    if (box.size() < 2) return false;

    bool progress=false;

    //list of candidates still available in any cell
    std::vector<int> candidates=candidates_in_cells(box);
    for (auto candidate:candidates) {

        // list of which rows can contain candidate
        std::vector<int> viable_rows;

        for (auto cell:box) {

            if (cell->candidates[candidate]) {

                //avoids adding the same row more than once
                if (is_member(cell->row(transpose),viable_rows)) continue;
                viable_rows.push_back(cell->row(transpose));
            }
        }

        if (viable_rows.size()==1) {
            //possible block row interaction found
            Sudoku* sudoku=box[0]->sudoku;

            //for each cell in "the" row
            for (auto cell:sudoku->empty_cells_row(viable_rows[0],transpose)) {

                //only if cell wasn't in the original box
                if (!is_member(cell,box)) {

                    //check that we didn't already know that candidate wasn't a viable candidate for that cell
                    if (cell->candidates[candidate]) {

                        //change log depending on whether we were considering the transpose
                        if (transpose)
                            std::cout << "Block column";
                        else
                            std::cout << "Block row";
                        std::cout << " interaction in box " << box[0]->box() << ". Removed candidate " << candidate << " from cell " << cell->index << "\n";

                        remove_candidate(cell,candidate);
                        progress=true;
                    }
                }
            }
        }
    }

    if (progress) return true;
    return false;
}

//http://www.sadmansoftware.com/sudoku/blockblock.htm
//check if candidate can only go in two rows in each of two boxes on the same row
//e.g. boxes 0 and 2 (same row), if candidate 4 can only go in rows 1 and 2 in both boxes, then 4 can't go in rows 1 and 2 in box 1

//if transpose==true, we consider the transposed sudoku. This allows us to do both "block block row" and "block block column" algorithms using the same function
bool block_block_interaction(std::vector<Cell*> const& box1,std::vector<Cell*> const& box2,bool transpose=false);
bool block_block_interaction(Sudoku& arg_sudoku,bool transpose=false) {
    //based on box_id, find box_id's of the other two boxes in same box-row: e.g. 0 -> 1,2  ; 1 -> 0,2 ; 2 -> 0,1
    for (int box=0;box<9;++box) {

        //the two different boxes in the same box-row as "box"
        int other_boxes[2];

        int count=0;
        // box / 3 * 3 is the box on the left side in the same box-row. +1 for the middle one, +2 for the right one
        for (int i=0;i<3;++i) {
            if (box / 3 * 3 + i != box) {
                other_boxes[count]=box / 3 * 3 + i;
                ++count;
            }
        }

        //if transpose==true we want the algorithm to work on the transposed sudoku => use the box ID's of the transposed sudoku
        if (block_block_interaction(arg_sudoku.empty_cells_box(other_boxes[0],transpose),arg_sudoku.empty_cells_box(other_boxes[1],transpose),transpose)) return true;

    }

    return false;
}

bool block_block_interaction(std::vector<Cell*> const& box1,std::vector<Cell*> const& box2,bool transpose) {

    // need at least 2 cells in each box, otherwise block block interaction can't happen
    if (box1.size()<2 || box2.size()<2) return false;

    bool progress=false;

    //candidate must still be viable in both boxes
    std::vector<int> candidates=intersection(candidates_in_cells(box1),candidates_in_cells(box2));

    //typical way to append box2 after box1. cells now contains both cells in box1 and box2
    auto cells=box1;
    cells.insert(cells.end(),box2.begin(),box2.end());

    for (auto candidate:candidates) {

        //list of which rows can contain candidate
        std::vector<int> viable_rows;

        for (auto cell:cells) {
            if (cell->candidates[candidate]) {

                //avoids adding the same row more than once
                if (is_member(cell->row(transpose),viable_rows)) continue;
                viable_rows.push_back(cell->row(transpose));
            }
        }

        if (viable_rows.size()==2) {
            //block block interaction possible
            Sudoku* sudoku=box1[0]->sudoku;
            auto cells1=sudoku->empty_cells_row(viable_rows[0],transpose);
            auto cells2=sudoku->empty_cells_row(viable_rows[1],transpose);

            //standard way to append cells2 to cells1. cells1 now contains all the empty cells in both rows
            cells1.insert(cells1.end(),cells2.begin(),cells2.end());
            auto all_the_cells=cells1;

            for (auto cell:all_the_cells) {

                //only if cell wasn't in the original box
                if (!is_member(cell,cells)) {

                    //check that we didn't already know that candidate wasn't a viable candidate for that cell
                    if (cell->candidates[candidate]) {

                        //change log depending on whether sudoku is the original or transpose
                        if (transpose)
                            std::cout << "Block block column";
                        else
                            std::cout << "Block block row";
                        std::cout << " interaction in boxes " << box1[0]->box() << " and " << box2[0]->box() << ". Removed candidate " << candidate << " from cell " << cell->index << "\n";

                        remove_candidate(cell,candidate);
                        progress=true;
                    }
                }
            }
        }
    }

    if (progress) return true;
    else return false;
}
// http://www.sadmansoftware.com/sudoku/nakedsubset.htm
// two (three, four) cells in the same row /column / box with exactly the same two (three, four) candidates => those candidates can't go in other cells in the same row / column / box
bool naked_subsets(std::vector<Cell*> const& cells);
bool naked_subsets(Sudoku& arg_sudoku) {
    //for each row
    for (int i=0;i<9;++i) if (naked_subsets(arg_sudoku.empty_cells_row(i))) return true;
    //for each column
    for (int i=0;i<9;++i) if (naked_subsets(arg_sudoku.empty_cells_column(i))) return true;
    //for each box
    for (int i=0;i<9;++i) if (naked_subsets(arg_sudoku.empty_cells_box(i))) return true;
    return false;
}

bool naked_subsets(std::vector<Cell*> const& cells) {

    //need 3 for naked pairs, 5  for triplets, 7  for quads
    unsigned int upto;
    if (cells.size()>=7) upto=4;
    else if (cells.size()>=5) upto=3;
    else if (cells.size()>=3) upto=2;
    else return false;

    bool progress=false;

    //list of candidates still available in any cell
    std::vector<int> candidates=candidates_in_cells(cells);

    //first we consider subsets of size 2, then 3 (if upto>=3), then 4 (if upto>=4)
    for (unsigned int i=2;i<=upto;i++) {

        //for each subset of size i of candidates
        for (std::vector<int> naked_candidates:subsets(i,candidates)) {

            //vector containing list of cells that are viable for the naked subset, i.e. so that    candidates_list(cell) is contained (is_subset()) in naked_candidates
            std::vector<Cell*> viable_naked;

            for (auto cell:cells) {
                if (is_subset(cell->candidate_list(),naked_candidates)) {
                    viable_naked.push_back(cell);
                }
            }

            if (viable_naked.size()==i) {
                //possible naked subset found

                //but does it allow anything to be removed?
                for (auto cell:cells) {

                    //only remove it from the other cells
                    if (is_member(cell,viable_naked)) continue;

                    for (auto candidate:naked_candidates) {

                        //check if we didn't already know
                        if (cell->candidates[candidate]) {
                            std::cout << "Naked subset:";
                            for (auto g:naked_candidates) std::cout << " " << g;
                            std::cout << ". Removed candidate " << candidate << " from cell " << cell->index << "\n";
                            remove_candidate(cell,candidate);
                            progress=true;
                        }
                    }
                }

            }
        }
    }
     if (progress) return true;
     return false;
}

// http://www.sadmansoftware.com/sudoku/hiddensubset.htm
// two (three, four) cells in a row / column / box are the only ones that can contain two (three, four) fixed candidates => can remove other candidates from those two (three, four) cells
bool hidden_subsets(std::vector<Cell*> const& cells);
bool hidden_subsets(Sudoku& arg_sudoku) {
    //for each row
    for (int i=0;i<9;++i) if (hidden_subsets(arg_sudoku.empty_cells_row(i))) return true;
    //for each column
    for (int i=0;i<9;++i) if (hidden_subsets(arg_sudoku.empty_cells_column(i))) return true;
    //for each box
    for (int i=0;i<9;++i) if (hidden_subsets(arg_sudoku.empty_cells_box(i))) return true;
    return false;
}

bool hidden_subsets(std::vector<Cell*> const& cells) {

    //need 3 cells for hidden pairs, 5 for triplets, 7 for quads
    unsigned int upto;
    if (cells.size()>=7) upto=4;
    else if (cells.size()>=5) upto=3;
    else if (cells.size()>=3) upto=2;
    else return false;

    bool progress=false;

    //list of candidates still available in any cell
    std::vector<int> candidates=candidates_in_cells(cells);

    //first we consider subsets of size 2, then 3 (if upto>=3), then 4 (if upto>=4)
    for (unsigned int i=2;i<=upto;i++) {

        //for each subset of size i of candidates
        for (std::vector<int> hidden_candidates:subsets(i,candidates)) {

            //list of cells where any of hidden_candidates are still viable
            std::vector<Cell*> viable_hidden;

            for (auto candidate:hidden_candidates) {
                for (auto cell:cells) {
                    if (cell->candidates[candidate]) {

                        //avoid adding it more than once
                        if (!is_member(cell,viable_hidden)) {
                            viable_hidden.push_back(cell);
                        }

                    }
                }
            }

            if (viable_hidden.size()==i) {
                //possible hidden subset found

                //but does it allow anything to be removed?

                for (auto cell:viable_hidden) {
                    for (auto candidate:candidates) {
                            //remove the OTHER candidates from the cells of viable_hidden
                            if (!is_member(candidate,hidden_candidates)) {
                                //check we didn't already know
                                if (cell->candidates[candidate]) {
                                    std::cout << "Hidden subset:";
                                    for (auto g:hidden_candidates) std::cout << " " << g;
                                    std::cout << ". Removed candidate " << candidate << " from cell " << cell->index << "\n";
                                    remove_candidate(cell,candidate);
                                    progress=true;
                                }
                            }
                    }
                }

            }
        }
    }
     if (progress) return true;
     return false;
}
void solve(Sudoku& sudoku) {
	while (true) {
		if (naked_single(sudoku)) continue;
		if (hidden_single(sudoku)) continue;

		if (block_row_interaction(sudoku,false)) continue;
		if (block_row_interaction(sudoku,true)) continue;

		if (block_block_interaction(sudoku,false)) continue;
		if (block_block_interaction(sudoku,true)) continue;

		if (naked_subsets(sudoku)) continue;
        if (hidden_subsets(sudoku)) continue;
		break;
	}
}

int main(int argc,char** argv) {
    Sudoku sudoku;

	import_from_file(sudoku);

	std::cout << "Imported sudoku:\n\n";
	print(sudoku);
	std::cout << "\n";
	print_full(sudoku);
	std::cout << "\n";

    std::cout << "Solving now: \n\n";
	solve(sudoku);

    std::cout << "\n";
    std::cout << "Solved sudoku:\n\n";
    print_full(sudoku);
    std::cout << "\n";
	print(sudoku);

	return 0;
}

