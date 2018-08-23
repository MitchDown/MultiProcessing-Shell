#include "Shell.h"
#include <iostream>
#include <sys/types.h>
#include <unistd.h>


Shell::Shell()
{
	numTokens = 0;
	numCommands = 1;
}

void Shell::runCommandSequence()
{
	readInput();
	parseLine();
	tokenToCommand();
	int tokensInCommand = 0;
	bool needPipe = numCommands > 1;
	int status;
	int fileDirectory[numCommands - 1][2];
	pid_t pid;
	for(int i = 0; i < numCommands; i++)
	{
		char* args[51];
		char argBuffer[50][100];
		tokensInCommand = 0;
		if(needPipe)
		{
			pipe(fileDirectory[i]);
		}
		pid = fork();
		if(pid == -1)
			perror("Failed to fork. \n");
		else if(pid == 0)
		{
			
			for(int j = 0; j < 30; j++)
			{
				if(commandBuffer[i][j].length() != 0)
				{
					//copies a given processes command arguments into the argBuffer
					strcpy(argBuffer[j],commandBuffer[i][j].c_str());
					tokensInCommand++;
				}
			}
			for(int p = 0; p < tokensInCommand; p++)
			{
				args[p] = NULL;
				args[p] = argBuffer[p];
			}
			args[tokensInCommand] = (char*)NULL;
			if(needPipe)
			{
				if(i > 0)
				{
					//condition for needing a piped input
					dup2(fileDirectory[i - 1][0],0);
					close(fileDirectory[i - 1][1]);
				}
				if(i + 1 < numCommands)
				{
					//condition for needing a piped output
					dup2(fileDirectory[i][1],1);
					close(fileDirectory[i][0]);
				}
				else
				{
					//if you're the last command, close your assigned pipe
					close(fileDirectory[i][1]);
					close(fileDirectory[i][0]);
				}

				if(i > 1)
				{
					//close all previously opened pipes that you are not using
					for(int j = 0; j < i - 1; j++)
					{
						close(fileDirectory[j][1]);
						close(fileDirectory[j][0]);
					}
				}
			}
			execvp(args[0], args);
			perror("bad execution ");
			exit(1);
		}
	}
	for(int i = 0; i < numCommands; i++)
	{
		//close pipes in the parent process
		close(fileDirectory[i][0]);
		close(fileDirectory[i][1]);
	}
	while(pid != -1)
	{
		
		pid = wait(&status);
		if(pid != -1)
			cout << "Process " << pid << " exited with a value of "<< status << endl;
	}
}

void Shell::readInput()
{
	cout << "$ ";
	getline(cin, shellInput);
	cout << "You inputted: " << shellInput << endl;
}

void Shell::parseLine()
{
	char doubleQuote = '"';
	int position = 0;
	int maxLength = shellInput.length();
	while (position < maxLength)
	{
		char currentChar = shellInput[position];
		if(currentChar != ' ')
		{
			if(currentChar == doubleQuote)
			{
				for(int i = position + 1; i < maxLength; i++)
				{
					if(shellInput[i] == doubleQuote)
					{
						tokenBuffer[numTokens] = shellInput.substr(position + 1, i - position - 1);
						position = i;
						numTokens++;
						break;
					}
				}
			}
			else if(shellInput.substr(position, 1) == "'")
			{
				for(int i = position + 1; i < maxLength; i++)
				{
					if(shellInput.substr(i, 1) == "'")
					{
						tokenBuffer[numTokens] = shellInput.substr(position + 1, i - position - 1);
						position = i;
						numTokens++;
						break;
					}
				}
			}
			else
			{
				//Tokens without any quotes require a lot of edge-case protections
				if(shellInput[position] == '|')
				{
					tokenBuffer[numTokens] = shellInput.substr(position, 1);
					position++;
					numTokens++;
				}
				if(shellInput[position + 1] == ' ')
				{
					tokenBuffer[numTokens] = shellInput.substr(position, 1);
					position++;
					numTokens++;
				}
				else if(shellInput[position] != ' ')
				{
					for(int i = position; i < maxLength; i++)
					{
						if(shellInput[i] == '|')
						{
							tokenBuffer[numTokens] = shellInput.substr(position, i - position);
							tokenBuffer[numTokens + 1] = shellInput[i];
							position = i;
							numTokens+= 2;
							break;
						}
						else if(shellInput[i] == ' ')
						{
							tokenBuffer[numTokens] = shellInput.substr(position, i - position);
							position = i;
							numTokens++;
							break;
						}
						else if(i + 1 == maxLength)
						{
							tokenBuffer[numTokens] = shellInput.substr(position, maxLength - position);
							position = i;
							numTokens++;
							break;
						}
					}
				}
			}
		}
		position++;
	}
}


void Shell::tokenToCommand()
{
	int position = 0;
	int tokenQuantity = 0;
	int breakPosition = 0;
	int j = 0;
	while(tokenQuantity < numTokens)
	{
		for(int i = breakPosition; i < numTokens; i++)
		{
			if(tokenBuffer[i] != "|")
			{
				commandBuffer[position][j] = tokenBuffer[i];
				tokenQuantity++;
				j++;
			}
			else
			{
				position++;
				tokenQuantity++;
				numCommands++;
				breakPosition = i + 1;
				j = 0;
			}
		}
	}
}