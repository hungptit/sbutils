/*
	Problem:  Given a Cartesian graph which is a 2D planar representation of a geographic terrain.  The blue numbers in each unit square represents the elevation of the terrain of that square. 
		For instances, a value of 1 means 1 meter, a value of 2 means 2 meter, and so on.
 
		Robot “Ro” is at a location (0,4) that has an elevation of 1 meter. And it has to reach destination “De” at a location (8,4), which also has an elevation of 1 meter.
		Fuel costs of moving from one square to another square:
		- Cost of moving the robot from one square to another square of the same elevation is 1 unit.
		- Cost of moving the robot from one square of elevation n to another square of elevation m = abs(n-m) + 1 units.
		
		The objective of this problem is to find the most optimal path from “Ro” (0,4) to “Dest” (9,4), i.e., the path that has the least fuel costs.
*/

/*
	Assumption: Robot can only move Up, Down, Left, Right one step at a time.  x-coordinate are in {0, 1, ..., N - 1}, y-coordinates are in {0, 1, ..., M - 1}.

	Approach:  This can be converted into a finding shortest path from a single source problem on a weighted undirected simple graph, whose vertices are the locations (squares) on the map. 
		Two vertices are adjacient to each other with weight equal to (their absolute height difference + 1) if their squares share an edge.  
		To store the data, I use struct Matrix, which is a 2D array with its number of rows (nrow) and and its number of columns.
		To solve the problem, I use the Djikstra algorithm with binary heap (called Djikstra_Heap) to keep track of the smallest unvisited vertex.
		Djikstra_Heap was built on Matrix since I need a dictionary to keep track of the current heap location of an unvisited vertex for updating their key (current shortest distance from Source (Robot)).
		Djikstra_Heap has 3 main operations:
		- insert: add a new node to heap (complexity O(log_2 (size of heap)))
		- check: update the position in the heap of a node (complexity O(log_2 (size of heap)))
		- pop_min: extract the node with the smallest key (root of the heap) (complexity O(log_2 (size of heap)))
		So if the map is given by an (M rows x N columns) arrays, the complexity of the worst case would be O( (M*N) * log_2 (M*N) ), since the number of edges = (M - 1)*N + M*(N - 1) = O(M*N).


	Input:	N M Ro.x Ro.y De.x De.y
		Height matrix starting from coordinate (0, M - 1) ... (N - 1, M - 1)
													.				.
													.				.
												(0, 0)	  ...  (N - 1, 0)

	Output: The least fuel costs
		Path from Robot to Destination (UDLR).
*/

#include <iostream>
#include <iomanip>
#include <vector>
#include <array>
using namespace std;

enum Status {
             UNVISITED = 0,
             ABOVE = 1,
             LEFT = 2,
             RIGHT = 3,
             BELOW = 4,
             MARKED = 5,
             MARKED_ABOVE = 6,
             MARKED_LEFT = 7,
             MARKED_RIGHT = 8,
             MARKED_BELOW = 9,
};

const int direction[4][2] = { {1, 0}, {0, -1}, {0, 1}, {-1, 0} };
const std::array<char, 4> dir = {'U', 'L', 'R', 'D'};


struct Coordinate {
  int row, col;
  Coordinate() : row(0), col(0) {}
  Coordinate(const int r, int c) : row(r), col(c) {}
};

struct CoordinateKey : Coordinate {
  int key;
  
  CoordinateKey() : Coordinate() {
    key = 0;
  }
  
  CoordinateKey(int r, int c, int k) : Coordinate(r, c) {
    key = k;
  }
  
	CoordinateKey(Coordinate coord, int k) {
		row = coord.row;
		col = coord.col;
		key = k;
	}
};

struct Matrix {
	int nrow = 0;
	int ncol = 0;

	vector< vector<int> > h;

	Matrix(int nr, int nc){
		nrow = nr;
		ncol = nc;
		h.resize(nr);
		for (int i = 0; i < nr; i++)
			h[i].resize(nc);
	}

	Matrix& operator=(Matrix m) {
		swap(nrow, m.nrow);
		swap(ncol, m.ncol);
		swap(h, m.h);
		return *this;
	}
};

/*
	Binary heap structure used in Djikstra algorithm
*/

struct Djikstra_Heap : Matrix {
	int size = 0;
	vector<CoordinateKey>	list;

	Djikstra_Heap(int nr, int nc) : Matrix(nr, nc){
		list.resize(nr*nc);

		for (int i = 0; i < nr; i++)
			for (int j = 0; j < nc; j++)
				h[i][j] = -1;
	}

	int parent(int pos) {
		return ((pos - 1) / 2);
	}

	int left(int pos) {
		return ((pos * 2) + 1);
	}

	int right(int pos) {
		return ((pos * 2) + 2);
	}

	void check(int pos) {
		int p = pos;
		int par = parent(p);
		while (list[p].key < list[par].key) {
			h[list[p].row][list[p].col] = par; 
			h[list[par].row][list[par].col] = p;
			swap(list[p], list[par]);
			p = par;
			par = parent(p);
		}
	}

	void insert(CoordinateKey c) {
		list[size] = c;
		h[c.row][c.col] = size;
		size++;
		check(size - 1);
	}

	CoordinateKey pop_min() {
		CoordinateKey min = list[0];
		h[list[0].row][list[0].col] = -1;
		h[list[size - 1].row][list[size - 1].col] = 0;
		swap(list[0], list[size - 1]);
		size--;

		bool flag = true;
		int p = 0;
		while (flag) {
			int l = left(p);
			int r = right(p);
			int m = p;

			if ((l < size) && (list[l].key < list[m].key))
				m = l;
			
			if ((r < size) && (list[r].key < list[m].key))
				m = r;

			if (m == p)
				flag = false;
			else {
				h[list[p].row][list[p].col] = m;
				h[list[m].row][list[m].col] = p;
				swap(list[p], list[m]);
				p = m;
			}

		}

		return min;
	}
};

bool in_range(int nrow, int ncol, Coordinate c) {
	return ((0 <= c.row) && (c.row < nrow) && (0 <= c.col) && (c.col < ncol));
}

/*
	Find the shortest path from Robot to Destination (Djikstra algorithm with binary heap)
*/

int find_path(Matrix map, Coordinate Ro, Coordinate De, Matrix& pr) {
	int nrow = map.nrow;
	int ncol = map.ncol;
	int shortest_distance = -1;

	if (!in_range(nrow, ncol, Ro))
		return -1;
	else if  (!in_range(nrow, ncol, De))
		return -2;

	Matrix previous(nrow, ncol);
	for (int i = 0; i < nrow; i++) {
		for (int j = 0; j < ncol; j++)
			previous.h[i][j] = UNVISITED;
	}
	CoordinateKey source(Ro, 0);

	Djikstra_Heap dheap(nrow, ncol);
	dheap.insert(source);

	while (dheap.size > 0) {
		CoordinateKey current_min = dheap.pop_min();
		previous.h[current_min.row][current_min.col] += MARKED;

		if (previous.h[De.row][De.col] >= MARKED) {
			shortest_distance = current_min.key;
			break;
		}

		CoordinateKey neighbor;

		for (int i = 1; i <= 4; i++) {
			neighbor.row = current_min.row + direction[i - 1][0];
			neighbor.col = current_min.col + direction[i - 1][1];

			if (in_range(nrow, ncol, neighbor)) {
				neighbor.key = current_min.key + abs(map.h[current_min.row][current_min.col] - map.h[neighbor.row][neighbor.col]) + 1;

				if (previous.h[neighbor.row][neighbor.col] < MARKED)
					if (previous.h[neighbor.row][neighbor.col] == UNVISITED) {
						dheap.insert(neighbor);
						previous.h[neighbor.row][neighbor.col] = 5 - i;
					}
					else if (dheap.list[dheap.h[neighbor.row][neighbor.col]].key > neighbor.key) {
						dheap.list[dheap.h[neighbor.row][neighbor.col]].key = neighbor.key;
						previous.h[neighbor.row][neighbor.col] = 5 - i;
						dheap.check(dheap.h[neighbor.row][neighbor.col]);
					}
			}
		}
	}

	pr = previous;
	return shortest_distance;
}

/*
	Write down the path from Robot to Destination
*/

void mark_path(Coordinate p, Matrix* pr) {
	if (pr->h[p.row][p.col] > MARKED) {
		Coordinate n;
		n.row = p.row + direction[pr->h[p.row][p.col] - MARKED - 1][0];
		n.col = p.col + direction[pr->h[p.row][p.col] - MARKED - 1][1];
		
		mark_path(n, pr);
		pr->h[p.row][p.col] = 2*MARKED;
	}
}

void print(Matrix map, Matrix pr) {
	for (int i = map.nrow - 1; i >= 0; i--) {
		for (int j = 0; j < map.ncol; j++)
			if (pr.h[i][j] == MARKED)
				cout << "R ";
			else if (pr.h[i][j] == 2 * MARKED)
				cout << "* ";
			else
				cout << map.h[i][j] << ' ';
		cout << endl;
	}
}

/*
	Main program
*/


int main(int argc, char** argv) {
	int nrow = 0, ncol = 0;
	Coordinate Ro, De;
	cin >> ncol >> nrow >> Ro.col >> Ro.row >> De.col >> De.row;
	Matrix map(nrow, ncol);

	for (int i = nrow - 1; i >= 0; i--)
		for (int j = 0; j < ncol; j++) {
			cin >> map.h[i][j];
	}

	Matrix pr(nrow, ncol);
	int sd = find_path(map, Ro, De, pr);

	if (sd == -1)
		cout << "Robot is out of range!" << endl;
	else if (sd == -2)
		cout << "Destination is out of range" << endl;
	else {
		cout << sd << endl;
		mark_path(De, &pr);
		print(map, pr);
	}
}
