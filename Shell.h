#include <string>

using namespace std;

class Shell
{
	private:
		string tokenBuffer[100]; //buffer containing the raw input from the user, separated into tokens.
		string commandBuffer[50][30]; //buffer containing all the user's commands. The outer array separates the commands, the inner array holds the arguments.
		string shellInput;
		int numTokens;
		int numCommands;
	public:
		Shell();

		//Takes input from the user and stores it in shellInput for use in other commands
		void readInput();

		//Parses through shellInput to determine what the argument tokens are, and stores them in tokenBuffer
		void parseLine();

		//Separates the tokens into commands based on '|' characters
		void tokenToCommand();

		//Runs the previous functions in a sequence, forks children, and sets up pipes as necessary.
		void runCommandSequence();
};