#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <random>
#include <cmath>
#include <limits>

using namespace std;

class GridSizeException : public invalid_argument 
{
public:
	explicit GridSizeException (const string& msg) : invalid_argument(msg) {}
};

class CipherLengthException : public invalid_argument
{
public:
	explicit CipherLengthException (const string& msg) : invalid_argument(msg) {}
};

class Util
{
public:
	static string sanitizeInput(const string& input) {
		string result = "";
		for (auto ch: input) { // removes any spaces
		    if (isspace(static_cast<unsigned char>(ch))) {
		        continue; // skips blanks, symbols
		    } else {
		        result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch)))); // converts to uppercase
		    }
		}
		if (result.empty()) {
		        result.push_back('.'); // add full stop if empty string
		}
		if (result.back() != '.') { // adds full stop to end
			result.push_back('.');
		}
	    return result;
	}
	
	static char randomUpper() { // Adds a random uppercase char
        static std::mt19937 rng{ std::random_device{}() }; // starts random engine
        static std::uniform_int_distribution<int> dist(0,25); // randomly chooses int from 0 to 25
		int randomIndex = dist(rng);
		char randomLetter = 'A' + randomIndex;
		return randomLetter;
	}

	static int calcGridSize (const string& text) { // calculates the size of the grid needed for the input string
		int textSize = text.size();
		int m  = 0;

		while (1 + 2*m*(m+1) < textSize) {
			m++;
		}

		return 2*m + 1;
	}

};

class Grid
{
private:
	size_t n; // size of grid
	vector<vector<char>> matrix; // init matrix
	
	string text; // holds sanitized string
	size_t pos; // iterator for text
public:
	Grid(size_t size) : n(size), matrix(size, vector<char>(size, '\0')) {
	    if (n%2 == 0 || n == 0) {
	        throw GridSizeException("Grid size must be a non-zero odd integer");
	    }
	}
	
	void loadPlain(const string& str) {
	    text = Util::sanitizeInput(str);
	    pos = 0;
	}

    void loadMulti(const string& str) {
        text = str;
        pos = 0;
    }

	void loadCipher(const string& cipher) {
		if (cipher.size() != n*n) {
			throw CipherLengthException("Cipher must be of n*n length where n is odd");
		}
		size_t k = 0;
		for (size_t j = 0; j < n; j++) {
			for (size_t i=0; i < n; i++) {
				matrix[i][j] = cipher[k++];
			}
		}
	}

	// gets next char in text, returns '\0' if text is ccomplete
	char getNextChar() {
		if (pos < text.size()) {
			return text[pos++];
		} else {
			return Util::randomUpper();
		}
	}
	
	void fillDiamondLayer(int layer) {
	    size_t center = n / 2; // center row & column of grid
	    size_t col = layer; // Start, (Middle left cell of layer)
	    size_t row = center;
	    
	    matrix[row][col] = getNextChar(); // place initial char

		if (layer == n/2) { // if in innermost layer, stop
			return;
		}

	    // Move up-right till top of layer
	    while(row > layer) {
			row--;
            col++;
	        matrix[row][col] = getNextChar();
	    }
	    
	    // Move down-right till right edge of layer
	    size_t rightEdge = n - 1 - layer;
	    while(col < rightEdge) {
	        row++;
	        col++;
	        matrix[row][col] = getNextChar();
	    }

		// Move down left till bottom of layer
		size_t bottomEdge = n - 1 - layer;
		while(row < bottomEdge) {
			row++;
			col--;
			matrix[row][col] = getNextChar();
		}

		// Move up left
		size_t amountUpLeft = (n - 2*layer - 3) / 2;
		for (int i = 0; i < amountUpLeft; i++) {
			row--;
			col--;
			matrix[row][col] = getNextChar();
		}
	}
	
	// fills all '\0' with rest of message, or a random letter, going column by column
	void fillWithRandom() {
	    for (size_t j = 0; j < n; j++) {
			for (size_t i = 0; i < n; i++) {
				if (matrix[i][j] == '\0') {
					matrix[i][j] = getNextChar();
				}
			}
		}
	}

	// completes diamond spiral text
	void fillAllDiamonds() {
		for (size_t layer = 0; layer <= n/2; layer++) {
			fillDiamondLayer(layer);
		}
	}

	// generates complete encrypted grid
	void generateGrid() {
		fillAllDiamonds();
		fillWithRandom();
	}

	string toEncryptedString() const{
		string encryptedString;
		encryptedString.reserve(n*n);
		for (size_t j = 0; j < n; j++) {
			for (size_t i = 0; i < n; i++) {
				encryptedString.push_back(matrix[i][j]); // concatonate column by column
			}
		}
		return encryptedString;
	}
    
	void printMatrix() {
		for (size_t i = 0; i < n; i++) {
			for (size_t j = 0; j < n; j++) {
				cout << matrix[i][j] << " ";
			}
			cout << endl;
		}
	}

	char getCell (size_t row, size_t col) const {
		return matrix[row][col];
	}
};

class CipherManager
{
public:
	string encryptFirstRound(const string& plain, int gridSize) {
		Grid g(gridSize);
		g.loadPlain(plain);
		g.generateGrid();
		g.printMatrix();
		return g.toEncryptedString();
	}

    string encryptMultiRound(const string& plain) {
        int gridSize = Util::calcGridSize(plain);
        Grid g(gridSize);
        g.loadMulti(plain);
        g.generateGrid();
        g.printMatrix();
        return g.toEncryptedString();
    }
    
    string decryptMultiRound(const string& cipher, int round, int rounds) {
        int gridSize = int(sqrt(cipher.size()));

		Grid g(gridSize);
		g.loadCipher(cipher);
		g.printMatrix();
		cout << endl;

		string msg;
		int center = gridSize / 2; // center row & column of grid
		size_t maxLoops = center;
		bool reachedEnd = false;

		int count = rounds - round + 1;

		decryptLoop(g ,msg, maxLoops, center, gridSize, reachedEnd, count);

        if (round == rounds) {
            return msg;
        }

        return msg;
    }

	void decryptLoop(const Grid& g, string& msg, size_t maxLoops, int center, int gridSize, bool reachedEnd, int count) {
		int fullStopCount = 0;
		for (size_t loop = 0; loop <= maxLoops; loop++) {

	    	size_t row = center;
			size_t col = loop; 

			// Get initial Point
			if (g.getCell(row,col) == '.') {
				fullStopCount++;
				if (fullStopCount == count) {
					reachedEnd = true;
					break;
				}
			}
			msg.push_back(g.getCell(row,col));

			if (loop == maxLoops) {
				break; // Stop if on final loop
			}

			// Go up right
			while (row > loop) {
				row--;
				col++;
				if (g.getCell(row,col) == '.') {
					fullStopCount++;
					if (fullStopCount == count) {
						reachedEnd = true;
						break;
					}
				}

				msg.push_back(g.getCell(row,col));
			}
			if (reachedEnd) {break;}

			// Go down right
			while (col < gridSize - 1 - loop) {
				row++;
				col++;
				if (g.getCell(row,col) == '.') {
					fullStopCount++;
					if (fullStopCount == count) {
						reachedEnd = true;
						break;
					}
				}
				msg.push_back(g.getCell(row,col));
			}
			if (reachedEnd) {break;}
			
			// Go down left
			while (row < gridSize - 1 - loop) {
				row++;
				col--;
				if (g.getCell(row,col) == '.') {
					fullStopCount++;
					if (fullStopCount == count) {
						reachedEnd = true;
						break;
					}
				}
				msg.push_back(g.getCell(row,col));
			}
			if (reachedEnd) {break;}

			// Go up left
			while (col > loop + 1) {
				row--;
				col--;
				if (g.getCell(row,col) == '.') {
					fullStopCount++;
					if (fullStopCount == count) {
						reachedEnd = true;
						break;
					}
				}
				msg.push_back(g.getCell(row,col));
			}
			if (reachedEnd) {break;}

		}
	}


	string encrypt(const string &msg, int rounds, int gridSize) {
		string text = msg;
		for (int r = 1; r <= rounds; r++) {
			if (r == 1) {
				text = Util::sanitizeInput(text);
				cout << "Encryption Round " << r << endl << endl;
				text = encryptFirstRound(text, gridSize);
				cout << endl << "Message after " << r << " round: " << text << endl;
			} else {
				text = Util::sanitizeInput(text);
				cout << endl << "Encryption Round " << r << " text: " << text << endl << endl;
				text = encryptMultiRound(text);
				cout << endl << "Message after " << r << " rounds: " << text << endl;
			}
		}
		cout << endl << "Encrypted Text:" << endl;
		return text;
	}

	string decrypt(const string& cipher, int rounds) {
		string text = cipher;
		for (int r = 1; r <= rounds; r++) {
            if (r == 1 && r == rounds) {
                cout << "Decryption Round " << r << endl;
				text = decryptMultiRound(text, r, rounds);
                cout << endl << "Message after " << r << " round: " << text << endl;
            } else {
                cout << "Decryption Round " << r << endl;
                text = decryptMultiRound(text, r, rounds);
                cout << endl << "Message after " << r << " rounds: " << text << endl;
            }
		}
		string final;
		for (auto ch : text) {
			if (ch == '.') {
				break;
			}
			final.push_back(ch);
		}
		return final;
	}



};

class Menu
{
protected:

	int levelOneAns;
	int levelTwoEncryptAns;
	int levelTwoDecryptAns;
	int oneRoundEncryptAns;
	int multiRoundEncryptAns;

	string msg;
	int gridSize;
	int rounds;

	// helper functions
	bool promptRounds() {
		// Enter round number
		cout << "Enter Amount of Rounds: ";
		string line;
		getline(cin, line);
		if (line.empty()) {
			cout << "Error: round count cannot be empty.\n\n";
			return false;
		}
		try {
			rounds = stoi(line);
		}
		catch (...) {
			cout << "Error: please enter a valid integer.\n\n";
			return false;
		}

		if (rounds < 1 || rounds > 10) {
			cout << "Error: rounds must be within 1 to 10.\n\n";
			return false;
		} 
		cout << endl;
		return true;
	}

	bool promptGridsize() {
		// Enter grid size
		cout << "Enter Gridsize: ";
		string input;
		getline(cin, input);
		if (input.empty()) {
			cout << "Error: gridsize cannot be empty.\n\n";
			return false;
		}
		try {
			gridSize = stoi(input);
		}
		catch (...) {
			cout << "Error: please enter a valid integer.\n\n";
			return false;
		}
		if (gridSize < 1 || gridSize > 99 || gridSize % 2 == 0) {
			cout << "Error: gridsize must be an odd integer between 1 and 99.\n\n";
			return false;
		}

		// check message fits in grid
		if (gridSize < Util::calcGridSize(msg)){
			cout << "Error: gridsize is too small to fit message.\n\n";
			return false;
		} 
		
		cout << endl;
		return true;
	}

	bool promptDecryptMessage() {
		cout << "Enter your message: ";
		getline(cin, msg);
		cout << endl;
		bool rightSize = false;
		for(int n = 1; n < 100; n+=2){ // Check msg is of correct size
			if (msg.size() == n*n) {
				rightSize = true;
				break;
			}
		}
		if (!rightSize) {
			cout << "Error: message must be of length n*n where n is odd.\n\n";
			return false;
		}
		return true;
	}

public:

	// Menu constructor
	Menu() : 
	levelOneAns(0),
	levelTwoEncryptAns(0),
	levelTwoDecryptAns(0),
	oneRoundEncryptAns(0),
	multiRoundEncryptAns(0),
	msg(""), 
	gridSize(0), 
	rounds(0) 
	{}
	

	void levelOne() {
		string choice;
		while(true) {
			cout << "************************" << endl;
			cout << "* Menu - Level 1       *" << endl;
			cout << "* Select an option:    *" << endl;
			cout << "* 1. Encrypt a message *" << endl;
			cout << "* 2. Decrypt a message *" << endl;
			cout << "* 3. Quit              *" << endl;
			cout << "************************" << endl;

			cin >> choice;
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << endl;

			if (choice != "1" && choice != "2" && choice != "3") {
				cout << "Please enter only 1, 2, or 3" << endl;
				continue;
			}

			levelOneAns = stoi(choice);
			break;
		}
	}

	void levelTwoEncrypt() {
		string choice;
		while(true) {
			cout << "******************************************" << endl;
			cout << "* Menu - Level 2: Encryption             *" << endl;
			cout << "* Select an option:                      *" << endl;
			cout << "* 1. Enter a message                     *" << endl;
			cout << "* 2. One-round encryption                *" << endl;
			cout << "* 3. Automatic multi-round encryption    *" << endl;
			cout << "* 4. Back                                *" << endl;
			cout << "******************************************" << endl;

			cin >> choice;
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << endl;

			if (choice != "1" && choice != "2" && choice != "3" && choice != "4") {
				cout << "Please enter only 1, 2, 3 or 4" << endl;
				continue;
			}
			levelTwoEncryptAns = stoi(choice);
			break;
		}
	}

	void levelTwoDecrypt() {
		string choice;
		while(true) {
			cout << "******************************************" << endl;
			cout << "* Menu - Level 2: Decryption             *" << endl;
			cout << "* Select an option:                      *" << endl;
			cout << "* 1. Enter a message                     *" << endl;
			cout << "* 2. Enter the round number              *" << endl;
			cout << "* 3. For each round, print the grid and  *" << endl;
			cout << "*    the corresponding decoded message   *" << endl;
			cout << "* 4. Back                                *" << endl;
			cout << "******************************************" << endl;

			cin >> choice;
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << endl;

			if (choice != "1" && choice != "2" && choice != "3" && choice != "4") {
				cout << "Please enter only 1, 2, 3 or 4" << endl;
				continue;
			}
			levelTwoDecryptAns = stoi(choice);
			break;
		}
	}

	void oneRoundEncrypt() {
		string choice;
		while(true) {
			cout << "*********************************************" << endl;
			cout << "* Menu - Level 3: Encryption                *" << endl;
			cout << "* Select an option:                         *" << endl;
			cout << "* 1. Enter a grid size                      *" << endl;
			cout << "* 2. Automatic grid size                    *" << endl;
			cout << "* 3. Print the grid and the encoded message *" << endl;
			cout << "* 4. Back                                   *" << endl;
			cout << "*********************************************" << endl;

			cin >> choice;
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << endl;

			if (choice != "1" && choice != "2" && choice != "3" && choice != "4") {
				cout << "Please enter only 1, 2, 3 or 4" << endl;
				continue;
			}
			oneRoundEncryptAns = stoi(choice);
			break;
		}
	}

	void multiRoundEncrypt() {
		string choice;
		while(true) {
			cout << "*********************************************" << endl;
			cout << "* Menu - Level 3: Encryption                *" << endl;
			cout << "* Select an option:                         *" << endl;
			cout << "* 1. Enter the round number                 *" << endl;
			cout << "* 2. For each round, print the grid and     *" << endl;
			cout << "*      the corresponding encoded message    *" << endl;
			cout << "* 3. Back                                   *" << endl;
			cout << "*********************************************" << endl;
			
			cin >> choice;
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << endl;

			if (choice != "1" && choice != "2" && choice != "3") {
				cout << "Please enter only 1, 2, or 3" << endl;
				continue;
			}
			multiRoundEncryptAns = stoi(choice);
			break;
		}
	}
};

class RunMenu : public Menu
{
public:
void runLevelOne() {
	while(true) {
		levelOne();
		switch (levelOneAns) {
			case 1:
				runLevelTwoEncrypt();
				break;
			case 2:
				runLevelTwoDecrypt();
				break;
			case 3:
				cout << endl << "Goodbye!" << endl << endl;
				exit(0); // Terminate Program
			default:
				break;
		}
	}
}

void runLevelTwoEncrypt() {
	while(true) {
		levelTwoEncrypt();
		switch (levelTwoEncryptAns) {
			case 1: {
				cout << "Enter your message: ";
				getline(cin, msg); // reads input into msg
				cout << endl;

				// Allow ASCII Only
				bool all_ascii = all_of(msg.begin(), msg.end(), 
				[](unsigned char c){ return c <= 127; });
				if (!all_ascii) {
					cout << "Error: message cannot have non ASCII Characters." << endl << endl;
					continue;
				}

				msg = Util::sanitizeInput(msg); // cleans msg
				gridSize = 0; // user must specify a grid size
				rounds = 0; // user must specify number of rounds 
				break;
			}
			case 2:
				runOneRoundEncrypt();
				break;
			case 3:
				runMultiRoundEncrypt();
				break;
			case 4:
				// Back to run level one
				return;
		}
	}
}

void runLevelTwoDecrypt() {
	while(true) {
		levelTwoDecrypt();
		string result;
		switch (levelTwoDecryptAns) {
			case 1:{
				if (!promptDecryptMessage()) {
					continue;
				}
				break;
			}
			case 2: {
				if (!promptRounds()) {
					continue;
				}
				break;
			}
			case 3: {
				// For each round print grid and message
				if (msg.empty()) {
					cout << "Error: no message entered. Please choose option 1 first.\n\n";
					continue;
				}
				if (rounds < 1) {
					cout << "Error: no round count specified, Please choose option 2 first.\n\n";
					continue;
				}
				CipherManager cm;
				try {
					result = cm.decrypt(msg, rounds);
					cout << endl << result << endl;
					return;
				}
				catch (const invalid_argument& e) {
					cout << "Error: " << e.what() << "\n\n";
				}
				break;
			}
			case 4:
				// Back to runLevelOne
				return;
		}
	}
}

void runOneRoundEncrypt() {
	while(true) {
		oneRoundEncrypt();
		string result;
		switch (oneRoundEncryptAns) {
			case 1: {
				if (!promptGridsize()) {
					continue;
				}
				break;
			}
			case 2:{
				// Auto grid size
				gridSize = Util::calcGridSize(Util::sanitizeInput(msg));
				break;
			}
			case 3: {
				// Print the grid and encoded message
				if (msg.empty()) {
					cout << "Error: no message entered. Please go back and enter a message.\n\n";
					continue;
				}
				if (gridSize < 1) {
					cout << "Error: Please include a gridSize.\n\n";
					continue;
				}
				CipherManager cm;
				try {
					result = cm.encrypt(msg, 1, gridSize);
					cout << result << endl;
					return;
				}
				catch (const invalid_argument& e) {
					cout << "Error: " << e.what() << "\n\n";
				}
				break;
			}
			case 4:
				// Back to encrypt level 2
				return;
		}
	}
}

void runMultiRoundEncrypt() {                                     
	while(true) {
		string result;
		multiRoundEncrypt();
		switch (multiRoundEncryptAns) {
			case 1: {
				if (!promptRounds()) {
					continue;
				}
				break;
			}
			case 2:
				// For each round print grid and message
				if (msg.empty()) {
					cout << "Error: no message entered. Please go back and enter a message.\n\n";
					continue; // re show menu
				}
				if (rounds < 1) {
					cout << "Error: no round count specified, Please choose option 1 first.\n\n";
					continue; // re show menu
				}
				gridSize = Util::calcGridSize(Util::sanitizeInput(msg));

				CipherManager cm;
				try {
					result = cm.encrypt(msg, rounds, gridSize);
					cout << result << endl;
					return;
				}
				catch (const invalid_argument& e) {
					cout << "Error: " << e.what() << "\n\n";
				}
				break;
			case 3:
				// Back to encrypt level 2
				return;
		}
	}
}

};


int main() {
	RunMenu m;
	m.runLevelOne();

	return 0;
}

//clang++ -std=c++17 week11revised.cpp -O2 -o week11r
// ./week11r
