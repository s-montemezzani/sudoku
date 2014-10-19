#include <vector>

//a Sudoko class contains an array of Cell with 81 elements
//the index of the array can be used to get the row/column/box to which the Cell belongs:
//  0  1  2  3  4  5  6  7  8
//  9  10 11 12 13 14 15 16 17
//           ...
//           ...
//  72 73 74 75 76 77 78 79 80
//
//We define the corresponding member functions row(), column() and box().
//To avoid writing two versions of some algorithms, once for row and once for columns, we pass a bool to indicate when we want to consider the transpose. In the transpose, the cell indexes look like
// 0  9  18 27 36 45 54 63 72
// 1  10 19 28 37 46 55 64 73
//            ...
//            ...
// 8  17 26 35 44 53 62 71 80


class Sudoku;
class Cell {
    public:
        //index in the Cell[81] array
        int index;
        //pointer to the sudoku the cell belongs to
        Sudoku* sudoku;
        //true if candidate[.] is still viable, false otherwise
        bool candidates[9];
        //true if number in the cell is known, false otherwise
        bool found;
        //if number is known, contains the number
        int value;
        //returns the number of candidates still viable
        int number_of_candidates() const;
        //returns vector containing the candidates that are still viable
        std::vector<int> candidate_list() const;

        //returns row of the cell.
        //if transpose==true, it means we consider the transposed sudoku.
        //For example, row(true) is the same as column(). box(true) returns the box of the cell with index transpose_cell_index(this->index)
        int row(bool const& transpose=false) const;
        int column(bool const& transpose=false) const;
        int box(bool const& transpose=false) const;
};

class Sudoku {
  public:
    //the 81 cells
    Cell cell[81];

    //return vector containing index of the not found cells in the row
    //again, if transpose==true, we consider the transpored sudoku.
    std::vector<Cell*> empty_cells_row(int const&,bool const& transpose=false);

    std::vector<Cell*> empty_cells_column(int const&,bool const& transpose=false);

    std::vector<Cell*> empty_cells_box(int const&, bool const& transpose=false);

    //constructor: initializes the field to be "empty"
    //Note: if using Cell() constructor we couldn't assign index or sudoku
    Sudoku() {
        for (int i=0;i<81;i++) {
            cell[i].found=false;
            cell[i].index=i;
            cell[i].sudoku=this;
            for (int j=0;j<9;j++) cell[i].candidates[j]=true;
		}
	}
};

//used for T=int and T=Cell*
template <typename T>
bool is_member(T const& a,std::vector<T> const& b) {
    for (auto element_b:b)
        if (a==element_b) return true;
    return false;
}




//see various.cpp for function comments

int cell_xy(int const&row,int const&column);
int cell2_xy(int const& box,int const&cell);
int transpose_cell_index(int const& cell);

std::vector<std::vector<int>> subsets(int k,std::vector<int> const& v,int skip=0);

bool is_subset(std::vector<int> const&a,std::vector<int> const&b);
std::vector<int> candidates_in_cells(std::vector<Cell*> const& cells);
std::vector<int> intersection(std::vector<int> const& cells1, std::vector<int> const& cells2);

void import_from_file(Sudoku& arg_sudoku);
void print(Sudoku const& arg_sudoku);
void print_full(Sudoku const& arg_sudoku);
void assign(Cell* const&,int const&);
void remove_candidate(Cell* const&,int const&);
