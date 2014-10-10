#include <vector>

class Cell {
    friend class Sudoku;

    //index of the Cell[81] array
    unsigned int index;

  public:
    //true if candidate[.] is still viable, false otherwise
    bool candidates[9];
    //true if number in the cell is known, false otherwise
    bool found;
    //if number is known, contains the number
    unsigned int value;
    //returns the number of candidates still viable
    int number_of_candidates();
    //row of the cell
    int row() { return index/9; }
    //column of the cell
    int column() { return index%9; }
    //box of the cell
    int box() { return (index / 27) * 3 + (index % 9) / 3; }
    //returns vector containing the candidates that are still viable
    std::vector<int> candidate_list();
};

class Sudoku {
  public:
    //the 81 cells
    Cell cell[81];
    //return vector containing index of the not found cells in the row
    std::vector<int> empty_cells_row(int const&);
    //return vector containing index of the not found cells in the column
    std::vector<int> empty_cells_column(int const&);
    //return vector containing index of the not found cells in the box
    std::vector<int> empty_cells_box(int const&);
    //pointer to the transposed sudoku
    Sudoku* transpose;
    //used to known which version to display
    bool is_the_transpose;

    Sudoku() {
        for (int j=0;j<81;j++) {
            cell[j].found=false;
            cell[j].index=j;
            for (int i=0;i<9;i++) cell[j].candidates[i]=true;
		}
	}
};





int cell_xy(int const&row,int const&column);
int cell2_xy(int const& box,int const&cell);
int transpose_cell_index(int const& cell);
int transpose_box_index(int const& box);

typedef std::vector<int> intvec;
std::vector<intvec> subsets(int k,intvec const& v,int skip=0);

bool is_subset(std::vector<int> const&a,std::vector<int> const&b);
bool is_member(int const&a,std::vector<int> const&b);

std::vector<int> candidates_in_cells(std::vector<int> const& cells,Sudoku const& arg_sudoku);
std::vector<int> intersection(std::vector<int> const& cells1, std::vector<int> const& cells2);

void import_from_file(Sudoku& arg_sudoku);
void print(Sudoku const& arg_sudoku);
void print_full(Sudoku const& arg_sudoku);
void assign(int const&,int const&,Sudoku& arg_sudoku);
void remove_candidate(int const&,int const&, Sudoku&);
