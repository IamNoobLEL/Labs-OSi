#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
int main()
{
	int fd1[2], fd2[2];
	if ((pipe(fd1) == -1) || (pipe(fd2) == -1))
	{
		std::cout << "Failed to open pipes between parent and child processes" << std::endl;
		exit(1);
	}
	pid_t child;
	if ((child = fork()) == -1)
	{
		std::cout << "Failed to create child process" << std::endl;
		exit(1);
	}
	else if (child > 0)
	{
		std::cout << "You're in the parent process with id [" << getpid() << "]" << std::endl;
		std::string file_path, string;
		std::cout << "Input path to the file" << std::endl;
		getline(std::cin, file_path);
		int length = file_path.length() + 1;
		write(fd1[1], &length, sizeof(int));
		write(fd1[1], file_path.c_str(), length * sizeof(char));
		char symbol, *in = (char*) malloc (2 * sizeof(char));
		int counter = 0, size_of_in = 2;
		std::cout << "Now input some strings. If you want to end input, press Ctrl+D" << std::endl;
		while ((symbol = getchar()) != EOF)
		{
			in[counter++] = symbol;
			if (counter == size_of_in)
			{
				size_of_in *= 2;
				in = (char*) realloc (in, size_of_in * sizeof(char));
			}
		}
		in = (char*) realloc (in, (counter + 1) * sizeof(char));
		in[counter] = '\0';
		write(fd1[1], &(++counter), sizeof(int));
		write(fd1[1], in, counter * sizeof(char));
		close(fd1[0]);
		close(fd1[1]);
		int out_length, wstatus;
		waitpid(child, &wstatus, 0);
		if (wstatus)
		{
			close(fd2[0]);
			close(fd2[1]);
			free(in);
			exit(1);
		}
		std::cout << "You're back in parent process with id [" << getpid() << "]" << std::endl;
		read(fd2[0], &out_length, sizeof(int));
		char* out = (char*) malloc (out_length);
		read(fd2[0], out, out_length * sizeof(char));
		std::ifstream fin(file_path.c_str());
		if (!fin.is_open())
		{
			close(fd2[0]);
			close(fd2[1]);
			free(in);
			std::cout << "Failed to open file to read strings" << std::endl;
			exit(1);
		}
		std::cout << "------------------------------------------------" << std::endl << "These strings end in character '.' or ';' :" << std::endl;
		if (fin.peek() != EOF)
		{
			while (!fin.eof())
			{
				getline(fin, string);
				std::cout << string << std::endl;
			}
		}
		fin.close();
		std::cout << "------------------------------------------------" << std::endl << "These strings don't end in character '.' or ';' :" << std::endl;
		for (int i = 0; i < out_length - 1; i++)
		{
			std::cout << out[i];
		}
		close(fd2[0]);
		close(fd2[1]);
		free(in);
		free(out);
	}
	else
	{
		int length;
		read(fd1[0], &length, sizeof(int));
		char* c_file_path = (char*) malloc (length * sizeof(char));
		read(fd1[0], c_file_path, length * sizeof(char)); 
		int counter;
		read(fd1[0], &counter, sizeof(int));
		char* inc = (char*) malloc (counter * sizeof(char));
		read(fd1[0], inc, counter * sizeof(char));
		close(fd1[0]);
		close(fd1[1]);
		std::cout << "Now you are in child process with id [" << getpid() << "]" << std::endl;
		std::string string = std::string(), out_string = std::string(), file_string = std::string();
		for (int i = 0; i < counter; i++)
		{
			if (inc[i] != '\0')
			{
				string += inc[i];
			}
			if ((inc[i] == '\n') || (inc[i] == '\0'))
			{
				if ((i > 0) && ((inc[i - 1] == '.') || (inc[i - 1] == ';')))
				{
					file_string += string;
				}
				else
				{
					out_string += string;
				}
				string = std::string();
			}
		}
		if ((file_string.length()) && (file_string[file_string.length() - 1] == '\n'))
		{
			file_string.pop_back();
		}
		std::ofstream fout(c_file_path);
		if (!fout.is_open())
		{
			std::cout << "Failed to create or open file to write strings" << std::endl;
			free(inc);
			free(c_file_path);
			close(fd2[0]);
			close(fd2[1]);
			return 1;
		}
		int out_length = out_string.length() + 1;
		write(fd2[1], &out_length, sizeof(int));
		write(fd2[1], out_string.c_str(), out_length * sizeof(char));
		close(fd2[0]);
		close(fd2[1]);
		if (!file_string.empty())
		{
			fout << file_string;
		}
		fout.close();
		free(inc);
		free(c_file_path);
	}
	return 0;
}