#include "various.hpp"
#include <iostream>
#include <vector>


// http://www.sadmansoftware.com/sudoku/nakedsingle.htm
// only one candidate in cell => it's the right number
bool naked_single(Sudoku& arg_sudoku) {
	for (int i=0;i<81;i++) {
        // don't do anything if cell is already solved
		if (arg_sudoku.cell[i].found) continue;

        if (arg_sudoku.cell[i].number_of_candidates()==1) {
            assign(i,arg_sudoku.cell[i].candidate_list()[0],arg_sudoku);
			std::cout << "Naked single: " << arg_sudoku.cell[i].candidate_list()[0] << ". Cell: " << i << "\n";
			return true;
		}
	}
	return false;
}

// http://www.sadmansoftware.com/sudoku/hiddensingle.htm
// only one cell in a row / column / box contains a certain candidate => that cell must contain that number
bool hidden_single(std::vector<int> const& cells,Sudoku& arg_sudoku);
bool hidden_single(Sudoku& arg_sudoku) {
    // for each row
    for (int i=0;i<9;++i) if (hidden_single(arg_sudoku.empty_cells_row(i),arg_sudoku)) return true;
    // for each column
    for (int i=0;i<9;++i) if (hidden_single(arg_sudoku.empty_cells_column(i),arg_sudoku)) return true;
    // for each box
    for (int i=0;i<9;++i) if (hidden_single(arg_sudoku.empty_cells_box(i),arg_sudoku)) return true;
    return false;
}
bool hidden_single(std::vector<int> const& cells,Sudoku& arg_sudoku) {
    //for every candidate of any of the cells
    for (auto candidate:candidates_in_cells(cells,arg_sudoku)) {

        // list of cells where candidate is viable
        std::vector<int> viable_cells;

        for (auto cell:cells) {
            if (arg_sudoku.cell[cell].candidates[candidate]) {
                viable_cells.push_back(cell);
            }
        }

        if (viable_cells.size()==1) {
            //possible hidden single found

            //check we didn't already know
            if (!arg_sudoku.cell[viable_cells[0]].found) {
                assign(viable_cells[0],candidate,arg_sudoku);
                std::cout << "Hidden single: " << candidate << ". Cell: " << viable_cells[0] << "\n";
                return true;
            }
        }
    }

	return false;
}

// http://www.sadmansoftware.com/sudoku/blockcolumnrow.htm
// a box only has a certain candidate in a certain row / column => candidate not viable in other cells in that same row / column
bool block_row_interaction(std::vector<int> const& cells,Sudoku& arg_sudoku);
bool block_row_interaction(Sudoku& arg_sudoku) {
    //for each box
    for (int i=0;i<9;++i) {
        if (block_row_interaction(arg_sudoku.empty_cells_box(i),arg_sudoku))
            return true;
    }
    return false;
}
bool block_row_interaction(std::vector<int> const& box, Sudoku& arg_sudoku) {
    bool progress=false;

    // need at least 2 cells (if 1, naked_single or hidden_single will find it)
    if (box.size() < 2) return false;

    //list of candidates still available in any cell
    std::vector<int> candidates=candidates_in_cells(box,arg_sudoku);
    for (auto candidate:candidates) {

        // list of which rows can contain candidate
        std::vector<int> viable_rows;

        for (auto cell:box) {

            Cell& temp_cell=arg_sudoku.cell[cell];
            if (temp_cell.candidates[candidate]) {

                //avoids adding the same row more than once
                if (is_member(temp_cell.row(),viable_rows)) continue;
                viable_rows.push_back(temp_cell.row());
            }
        }

        if (viable_rows.size()==1) {
            //possible block row interaction found

            //for each cell in "the" row
            for (auto cell:arg_sudoku.empty_cells_row(viable_rows[0])) {

                //only if cell wasn't in the original box
                if (!is_member(cell,box)) {

                    //check that we didn't already know that candidate wasn't a viable candidate for that cell
                    if (arg_sudoku.cell[cell].candidates[candidate]) {

                        //change log depending on whether arg_sudoku is the original or transpose
                        if (arg_sudoku.is_the_transpose)
                            std::cout << "Block column interaction in box " << transpose_box_index(arg_sudoku.cell[box[0]].box()) << ". Removed candidate " << candidate << " from cell " << transpose_cell_index(cell) << "\n";
                        else
                            std::cout << "Block row interaction in box " << arg_sudoku.cell[box[0]].box() << ". Removed candidate " << candidate << " from cell " << cell << "\n";

                        remove_candidate(cell,candidate,arg_sudoku);
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

bool block_block_interaction(std::vector<int> const& box1,std::vector<int> const& box2, Sudoku& arg_sudoku);
bool block_block_interaction(Sudoku& arg_sudoku) {
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

        if (block_block_interaction(arg_sudoku.empty_cells_box(other_boxes[0]),arg_sudoku.empty_cells_box(other_boxes[1]),arg_sudoku)) return true;
    }

    return false;
}

bool block_block_interaction(std::vector<int> const& box1,std::vector<int> const& box2,Sudoku& arg_sudoku) {

    // need at least 2 cells in each box, otherwise block block interaction can't happen
    if (box1.size()<2 || box2.size()<2) return false;

    bool progress=false;

    //candidate must still be viable in both boxes
    std::vector<int> candidates=intersection(candidates_in_cells(box1,arg_sudoku),candidates_in_cells(box2,arg_sudoku));

    //typical way to append box2 after box1. cells now contains both cells in box1 and box2
    auto cells=box1;
    cells.insert(cells.end(),box2.begin(),box2.end());

    for (auto candidate:candidates) {

        //list of which rows can contain candidate
        std::vector<int> viable_rows;

        for (auto cell:cells) {
            Cell& temp_cell=arg_sudoku.cell[cell];
            if (temp_cell.candidates[candidate]) {

                //avoids adding the same row more than once
                if (is_member(temp_cell.row(),viable_rows)) continue;
                viable_rows.push_back(temp_cell.row());
            }
        }

        if (viable_rows.size()==2) {
            //block block interaction possible

            auto cells1=arg_sudoku.empty_cells_row(viable_rows[0]);
            auto cells2=arg_sudoku.empty_cells_row(viable_rows[1]);

            //standard way to append cells2 to cells1. cells1 now contains all the empty cells in both rows
            cells1.insert(cells1.end(),cells2.begin(),cells2.end());
            auto all_the_cells=cells1;

            for (auto cell:all_the_cells) {

                //only if cell wasn't in the original box
                if (!is_member(cell,cells)) {

                    //check that we didn't already know that candidate wasn't a viable candidate for that cell
                    if (arg_sudoku.cell[cell].candidates[candidate]) {

                        //change log depending on whether arg_sudoku is the original or transpose
                        if (arg_sudoku.is_the_transpose)
                            std::cout << "Block block column interaction in boxes " << transpose_box_index(arg_sudoku.cell[box1[0]].box()) << " and " << transpose_box_index(arg_sudoku.cell[box2[0]].box()) << ". Removed candidate " << candidate << " from cell " << transpose_cell_index(cell) << "\n";
                        else
                            std::cout << "Block block row interaction in boxes " << arg_sudoku.cell[box1[0]].box() << " and " << arg_sudoku.cell[box2[0]].box() << ". Removed candidate " << candidate << " from cell " << cell << "\n";

                        remove_candidate(cell,candidate,arg_sudoku);
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
bool naked_subsets(std::vector<int> const& cells,Sudoku& arg_sudoku);
bool naked_subsets(Sudoku& arg_sudoku) {
    //for each row
    for (int i=0;i<9;++i) if (naked_subsets(arg_sudoku.empty_cells_row(i),arg_sudoku)) return true;
    //for each column
    for (int i=0;i<9;++i) if (naked_subsets(arg_sudoku.empty_cells_column(i),arg_sudoku)) return true;
    //for each box
    for (int i=0;i<9;++i) if (naked_subsets(arg_sudoku.empty_cells_box(i),arg_sudoku)) return true;
    return false;
}

bool naked_subsets(std::vector<int> const& cells,Sudoku& arg_sudoku) {

    //need 3 for naked pairs, 5  for triplets, 7  for quads
    unsigned int upto;
    if (cells.size()>=7) upto=4;
    else if (cells.size()>=5) upto=3;
    else if (cells.size()>=3) upto=2;
    else return false;

    bool progress=false;

    //list of candidates still available in any cell
    std::vector<int> candidates=candidates_in_cells(cells,arg_sudoku);

    //first we consider subsets of size 2, then 3 (if upto>=3), then 4 (if upto>=4)
    for (unsigned int i=2;i<=upto;i++) {

        //for each subset of size i of candidates
        for (std::vector<int> naked_candidates:subsets(i,candidates)) {

            //vector containing list of cells that are viable for the naked subset, i.e. so that    candidates_list(cell) is contained (is_subset()) in naked_candidates
            std::vector<int> viable_naked;

            for (auto cell:cells) {
                if (is_subset(arg_sudoku.cell[cell].candidate_list(),naked_candidates)) {
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
                        if (arg_sudoku.cell[cell].candidates[candidate]) {
                            std::cout << "Naked subset: ";
                            for (auto g:naked_candidates) std::cout << g << " ";
                            std::cout << ". Removed candidate " << candidate << " from cell " << cell << "\n";
                            remove_candidate(cell,candidate,arg_sudoku);
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
bool hidden_subsets(std::vector<int> const& cells,Sudoku& arg_sudoku);
bool hidden_subsets(Sudoku& arg_sudoku) {
    //for each row
    for (int i=0;i<9;++i) if (hidden_subsets(arg_sudoku.empty_cells_row(i),arg_sudoku)) return true;
    //for each column
    for (int i=0;i<9;++i) if (hidden_subsets(arg_sudoku.empty_cells_column(i),arg_sudoku)) return true;
    //for each box
    for (int i=0;i<9;++i) if (hidden_subsets(arg_sudoku.empty_cells_box(i),arg_sudoku)) return true;
    return false;
}

bool hidden_subsets(std::vector<int> const& cells,Sudoku& arg_sudoku) {

    //need 3 cells for hidden pairs, 5 for triplets, 7 for quads
    unsigned int upto;
    if (cells.size()>=7) upto=4;
    else if (cells.size()>=5) upto=3;
    else if (cells.size()>=3) upto=2;
    else return false;

    bool progress=false;

    //list of candidates still available in any cell
    std::vector<int> candidates=candidates_in_cells(cells,arg_sudoku);

    //first we consider subsets of size 2, then 3 (if upto>=3), then 4 (if upto>=4)
    for (unsigned int i=2;i<=upto;i++) {

        //for each subset of size i of candidates
        for (std::vector<int> hidden_candidates:subsets(i,candidates)) {

            //list of cells where any of hidden_candidates are still viable
            std::vector<int> viable_hidden;

            for (auto candidate:hidden_candidates) {
                for (auto cell:cells) {
                    if (arg_sudoku.cell[cell].candidates[candidate]) {

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
                                if (arg_sudoku.cell[cell].candidates[candidate]) {
                                    std::cout << "Hidden subset: ";
                                    for (auto g:hidden_candidates) std::cout << g << " ";
                                    std::cout << ". Removed candidate " << candidate << " from cell " << cell << "\n";
                                    remove_candidate(cell,candidate,arg_sudoku);
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

		if (block_row_interaction(sudoku)) continue;
        //use transpose to avoid writing different function for columns
		if (block_row_interaction(*sudoku.transpose)) continue;

		if (block_block_interaction(sudoku)) continue;
        //use transpose to avoid writing different function for columns
        if (block_block_interaction(*sudoku.transpose)) continue;

		if (naked_subsets(sudoku)) continue;
        if (hidden_subsets(sudoku)) continue;
		break;
	}
}
int main(int argc,char** argv) {

    //sudoku.transpose points to sudoku_transpose, and vice versa
    Sudoku sudoku,sudoku_transpose;
    sudoku.transpose=&sudoku_transpose;
    sudoku_transpose.transpose=&sudoku;
    sudoku.is_the_transpose=false;
    sudoku_transpose.is_the_transpose=true;

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

