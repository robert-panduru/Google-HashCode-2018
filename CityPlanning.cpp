/*
Robert Panduru
2018
This is my solution to the final round of the Google HashCode 2018.
The problem statement can be found here:
https://hashcode.withgoogle.com/2018/tasks/hashcode2018_final_task.pdf
The program contains two classes: Building and Solver. Building is
used to model the specifics of a building plan, such as width, height
but also the location at which the building will be found on the city
plan. Solver will constitute the backbone of the program, dealing with
reading from the file, applying the algorithm and writing the found
solution. The idea behind the algorithm was inspired by greedy programming
and I tried to find a solution as general as possible, although the program
as is will perform above-average results only for building with regular shapes
and long maximum walking distances unfortunately. The computer will parse
the city plan and add the buildings that generate the most amount of points
in the short term. A lot of space is wasted if the buildings have odd shapes
that do not match like puzzle pieces.
*/

#include <bits/stdc++.h>

using namespace std;

// The Building object
class Building {
	public:
		// Constructor
		Building(char t, int h, int w, int v, int id) {
			type = t;
			hp = h;
			wp = w;
			value = v;
			ID = id; 
		}

		// Will be used to obtain the exact plan of the
		// building
		void modify(int i, int j, char c) {
			plan[i][j] = c;
		}

		// Prints the plan for visualisation
		void print(ofstream &out) {
			for (int i = 0; i < hp; i++) {
				for (int j = 0; j < wp; j++)
					out << plan[i][j];
				out << endl;
			}
		}

		// Is used to keep count the utilities an apartment
		// building is already benefiting from
		void addUtility(int id) {
			utilities.push_back(id);
		}

		// Clears the list of utilities
		void removeUtilities() {
			while (utilities.size() != 0)
				utilities.pop_back();
		}

		// Will check if a type of utility is already in use
		bool hasUtility(int id) {
			for (int i = 0; i < utilities.size(); i++)
				if (id == utilities[i])
					return true;
			return false;
		}

		// Getters
		int getValue() {
			return value;
		}

		char getType() {
			return type;
		}

		int getHeight() {
			return hp;
		}

		int getWidth() {
			return wp;
		}

		int getX() {
			return x;
		}

		int getY() {
			return y;
		}

		int getID() {
			return ID;
		}

		char getPlan(int i, int j) {
			return plan[i][j];
		}

		int getArea() {
			return area;
		}

		// Computes the effective area of the building
		void determineArea() {
			area = 0;
			for (int i = 0; i < hp; i++) 
				for (int j = 0; j < wp; j++)
					if (plan[i][j] == '#')
						area++;
		}

		// Sets the coordinates of the upper-left corner
		// of the building on the city plan
		void locate(int y1, int x1) {
			x = x1;
			y = y1;
		}

		// Will check if the location of the building is within
		// bounds and if there aren't any buildings on that spot.
		// If the location is viable the city map will be marked
		// for visualisation
		bool mark(char city[1000][1000], int H, int W, int id) {
			if ((y + hp - 1 >= H) || (x + wp - 1 >= W))
				return false;
			for (int i = 0; i < hp; i++)
				for (int j = 0; j < wp; j++)
					if (plan[i][j] == '#' && city[i + y][j + x] != '.')
						return false;
			for (int i = 0; i < hp; i++)
				for (int j = 0; j < wp; j++)
					if (plan[i][j] == '#')
						city[i + y][j + x] = to_string(id)[0];
			return true;
		}

		// Will erase the building from the plan
		void unmark(char city[1000][1000]) {
			for (int i = 0; i < hp; i++)
				for (int j = 0; j < wp; j++)
					if (plan[i][j] == '#')
						city[i + y][j + x] = '.';
		}

	private:
		int ID; // the index in the plan array
		char type; // type of the building
		int x, y; // coordinates
		int hp; // number of rows of the building
		int wp; // number of columns of the building
		int value; // resident capacity or type of service
		int area; // the number of squares actively used
		char plan[50][50]; // the shape of the building
		vector<int> utilities; // to keep track if the used utilities
};

class Solver {
	public:
		// Constructor
		Solver() {
			read();
			solve();
			print();
		} 

	private:
		// The core of the program
		void solve() {
			int index;
			double value = 0;
			// Determines the most cost-effective housing building
			for (int i = 0; i < B; i++)
				if (plans[i].getType() == 'R') {
					if (plans[i].getValue() * 1.0 / plans[i].getArea() > value) {
						index = i;
						value = plans[i].getValue() * 1.0 / plans[i].getArea();
					}
				}
			
			// And puts it in the upper-left corner of the city plan
			buildings.push_back(plans[index]);
			buildings[buildings.size() - 1].locate(0, 0);
			buildings[buildings.size() - 1].mark(city, H, W, buildings.size() - 1);

			// Will check for the neighbor that generates the most amount of points
			for (int i = 0; i < H; i++)
				for (int j = 0; j < W; j++) {
					double maximum = 0;
					int chosen = -1;
					// If this spot is taken it will be skipped
					if (city[i][j] != '.')
						continue;
					// Iterate though all the plans to find the most profitable one
					for (int k = 0; k < plans.size(); k++) {
						double evaluation = 0;
						buildings.push_back(plans[k]);
						buildings[buildings.size() - 1].locate(i, j);
						// If the building can indeed be placed here the score of the
						// current placements is computed
						if (buildings[buildings.size() - 1].mark(city, H, W, buildings.size() - 1) == true) {
							evaluation = evaluate();
							buildings[buildings.size() - 1].unmark(city);
							// Will remember the candidate
							if (evaluation > maximum) {
								chosen = k;
								maximum = evaluation;
							}
						}

						buildings.pop_back();
					}
					// If a candidate was found it is added to the list of buildings
					if (chosen != -1) {
						buildings.push_back(plans[chosen]);
						buildings[buildings.size() - 1].locate(i, j);
						buildings[buildings.size() - 1].mark(city, H, W, buildings.size() - 1);
					}
				}
		}

		// Evaluation method. Will inspect the current city plan to see what
		// residences benefit of what utilities and will generate a number
		// that takes into account the number of points generated but also of
		// the areas of the buildings placed.
		double evaluate() {
			double score = 0;
			for (int i = 0; i < buildings.size(); i++) {
				if (buildings[i].getType() == 'R') // If it is a residence
					for (int j = 0; j < buildings.size(); j++)
						if (buildings[j].getType() == 'U' && // If it is an utility and the residence
						buildings[i].hasUtility(buildings[j].getID()) == false) { // doesn't have it
							if (distance(buildings[i], buildings[j]) <= D) { // checks the walking 
								score += 1.0 * buildings[i].getValue() / buildings[i].getArea() // distance
								/ buildings[j].getArea();
								buildings[i].addUtility(buildings[j].getID());
							}
						}
				buildings[i].removeUtilities();
			}
			return score;
		}

		// Calculates the distance between two buildings
		// Not the best way to do it
		int distance(Building &a, Building &b) {
			int distance = 1000000;
			int x1 = a.getX();
			int y1 = a.getY();
			int x2 = b.getX();
			int y2 = b.getY();
			// Computes the distance between two occupied points in the
			// building plans taking into account their coordinates on the map
			for (int i = y1; i < y1 + a.getHeight(); i++)
				for (int j = x1; j < x1 + a.getWidth(); j++)
					for (int k = y2; k < y2 + b.getHeight(); k++)
						for (int l = x2; l < x2 + b.getWidth(); l++)
							if (distance > abs(k - i) + abs(l - j) && a.getPlan(i - y1, j - x1) == '#'
								&& b.getPlan(k - y2, l - x2) == '#')
								distance = abs(k - i) + abs(l - j);
			return distance;
		}

		// Handles the file reading
		void read() {
			ifstream in;
			in.open("input.txt");
			in >> H >> W >> D >> B; // Specifics of the city plan
			for (int i = 0; i < B; i++) { // Populates the plan vector
				char t;
				int h, w, v;
				in >> t >> h >> w >> v;
				Building b(t, h, w, v, i); // Creates a building plan with the read characteristics
				// Plots the occupied squares of the building plan
				for (int j = 0; j < h; j ++) 
					for (int k = 0; k < w; k++) {
						char aux;
						in >> aux;
						b.modify(j, k, aux);
					}
				b.determineArea();
				plans.push_back(b);
			}
			in.close();

			// Generates the initial city plan
			for (int i = 0; i < H; i++)
				for (int j = 0; j < W; j++)
					city[i][j] = '.';
		}

		// Prints the final map for visualisation and
		// the locations of the buildings on it
		void print() {
			ofstream out;
			out.open("output.txt");
			// What the city looks like
			for (int i = 0; i < H; i++) {
				for (int j = 0; j < W; j++)
					out << city[i][j];
				out << endl;	
			}
			// Number of buildings
			out << endl << buildings.size() << endl;
			// Type and location
			for (int i = 0; i < buildings.size(); i++)
				out << buildings[i].getID() << " " << buildings[i].getY() << " "
				<< buildings[i].getX() << endl;
			out.close();
		}

		int H; // number of rows
		int W; // number of columns
		int D; // maximum walking distance
		int B; // number of building projects
		char city[1000][1000]; // the map of the future city
		vector<Building> plans; // this will keep all the types of buildings
		vector<Building> buildings; // this will keep a tab on all the buildings placed down
};

int main() {
	Solver s;
	return 0;
}