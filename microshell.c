/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rabril-h <rabril-h@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/19 21:02:06 by rabril-h          #+#    #+#             */
/*   Updated: 2023/08/21 21:39:22 by rabril-h         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

/**
 * @brief This function takes care of printing the errors
 * when operations fail as stated in the exam subject
 * 
 * It makes use of a while over a pointer instead of a counter so the string 
 * is parsed without the need for using a var to keep track of the position we are printing such as
 * 
 * int counte;
 * 
 * counter  = 0;
 * 
 * while (str[counter] != '\0')
 * {
 *  write(2,&str[counter], 1);
 *  counter++;
 * }
 * 
 * THE WRITE USES 2 AS FIRST PARAM BECAUSE WE ARE PRITING ERRORS!! * 
 * It has to return 1 as NON success for the operation
 *  
 * @param str 
 * @return int 
 */

int	msh_error(char *str)
{
	while (*str)
		write(2, str++, 1);
	return (1);
}

/**
 * @brief This function takes care of changing (or similating the changing)
 * of a directory
 * 
 * If the number of arguments are not 2 it means on ly the keyword cd has beign passed
 * The subject states to print the following error "error: cd: bad arguments" with a new line
 * 
 * If the operation we try has no success (this is done by trying to chdir(argv[1]) 
 * while argv[1] is the path we are trying to go to) then the subject states to output the following error
 * "error: cd: cannot change directory to [directory]" followed by a new line.
 * 
 * The functions has to return 0 is the operation has a sucess
 * 
 * @param argv 
 * @param i 
 * @return int 
 */

int	msh_cd(char **argv, int i)
{
	if (i != 2)
		return (msh_error("error: cd: bad arguments\n"));
	else if (chdir(argv[1]) == -1)
		return (msh_error("error: cd: cannot change directory to "),
			msh_error(argv[1]), msh_error("\n"));
	return (0);
}

/**
 * @brief 
 * 
 * This function takes care of the execution of comands in this case basically pipes (\) and semicolons (;)
 * 
 * In order to pipe anything we would need the following vars
 * 
 * 1) Two file descriptors so we can have a child process
 * 2) The status we will be storing (this will return either 0 or 1)
 * 3) A reference for the process / pid
 * 4) A custom boolean under the form of int in which we determine if the argument has a pipe
 * 
 * We determine that an argument has a pipe when there is argument and the argument is a pipe with 
 * ? has_pipe = argv[i] && !strcmp(argv[i], "|");
 * 
 * !BUT
 * 
 * If we have a pipe BUT we cannot access de file descriptor we throw an error as stated int he subject in a system call
 * 
 * ? if (has_pipe && pipe(fd) == -1)
 * ? return (msh_error("error: fatal\n"));
 * 
 * If there is no error then we proceed with creating the actual process with 
 * ? pid = fork();
 * 
 * Then we check for error in the process. If so we reverse i to the first argument in the token to 0
 * Then we check if we are dealing with a pipe and we have errors splitting child processes to their respective
 * file descriptors
 * 
 * If there are no errors we actually try to execute the comand BUT as stated in the subject we just deal with regault pipes and semicolons so anyways we fo for execve which takes 3 params (the actual "token" we are dealing with, the argument we are dealing with and the enviroment variables)
 * 
 * Then we return the error of not beign able to execute the command.
 * 
 * If we have a succesfull process then we just wait for the execution of the child process with 
 * ? waitpid(pid, &status, 0);
 * 
 * Then we check again for errors in the execution but this time with the other file descriptor.
 * 
 * Finally we return 0 or 1 based on whether or not the child process has ended its process and the status returned by that process with
 * ? return (WIFEXITED(status) && WEXITSTATUS(status));
 * 
 * @param argv 
 * @param envp 
 * @param i 
 * @return int 
 */

int msh_exec(char **argv, char **envp, int i)
{
	int	fd[2];
	int	status;
	int	pid;
	int	has_pipe;

	has_pipe = argv[i] && !strcmp(argv[i], "|"); // ? This acts as boolean either true or false
	if (has_pipe && pipe(fd) == -1) // ? We check errors if the conversion from our int fd fails
		return (msh_error("error: fatal\n")); 
	pid = fork(); // ? We do the actual fork here
	if (!pid) // ? We check if pid == 0 meaning is the child process. THIS IS NOT AN ERROR
	{
		argv[i] = 0; // ? Close the arg with NULL before sending it to execve so we take just the command we need
		if (has_pipe && (dup2(fd[1], 1) == -1
				|| close(fd[0]) == -1
				|| close(fd[1]) == -1)) // ? Check errors if the conversion cannot be done or if we cannot close any fd
			return (msh_error("error: fatal\n"));
		execve(*argv, argv, envp); 
		return (msh_error("error: cannot execute "),
			msh_error(*argv),
			msh_error("\n")); // ? execve kills the process so if we got here something bad happened
	}
	waitpid(pid, &status, 0); // ? The father process was waiting up until this point
	if (has_pipe && (dup2(fd[0], 0) == -1
			|| close(fd[0]) == -1
			|| close(fd[1]) == -1))
		return (msh_error("error: fatal\n")); // ? Check again for errors
	return (WIFEXITED(status) && WEXITSTATUS(status)); // ? return the status of the child process set by execve
}

/**
 * @brief This is the main function and the entry point of th program
 * 
 * It takes argc as number of arguments
 * It takes argv as the line passed to the program (the arguments)
 * It takes also the enviroment variables as default. We would need them later for the execve
 * 
 * We have a main counter i and a var to hold the exit status for the program both set to 0 at the begining
 * so we assume program will succeed by setting status to 0
 * 
 * If more than just ./microshell has been passed to the program it means we need to to stuff because we have arguments
 * Then while we have an argument and a following argument to that previous one "while (argv[i] && argv[++i])"
 * Then we move the pointer of argv (which is a **char so me move the first pointer (->*)*argv) to the current state of the counter
 * 
 * This would mean that for the first iteration in which we start with i = 0 we would be point art argv[0] and loking for argv[1]
 * 
 * Then inside loop we actully move the pointer as metioned before by equaling argv to += the value of i (in this case 0) so for example if we are looking at the provided subject 
 * 
 * ./microshell /bin/ls "|" /usr/bin/grep microshell ";" /bin/echo i love my microshell 
 * 
 * We would be looking at /bin/ls 
 * 
 * ! the quoted pipes area way of not messing with the actual terminal. In THE EXAM DO NOT USE "|" or ";" JUST | or ;
 * 
 * Once we moved the pointer we reuse the counter i to go through each "token" from the argument at hand. In this case argv[0]
 * 
 * Then we jump for char to char until we find either a pipe or a semicolon by while (argv[i] && strcmp(argv[i], "|") && strcmp(argv[i], ";"))
 * 
 * It would go like this argv[0][i] where argv[0] is the pointer moved at the beginning of the loop with argv += i and argv[0][i] would be the actual char at hand. This is represent just like argv[i] because we are at the right place from argv since the 
 * moment we moved the pointer.
 * 
 * Then as we find either a | or ; we perform our checks
 * 
 * if (!strcmp(*argv, "cd")) --> if we have found a cd instruction at argv[current pointer] which would be *argv 
 * we use the function in charge of changing the directory
 * 
 * Otherwise if our counter is bigger than 0 which means we actually moved beyond the first command from a "token" we try to exectute a command by sending it to the msh_exec function passing all needed info: the argv itself, the enviroment vars and the position we are at.
 * 
 * 
 * @param argc 
 * @param argv 
 * @param envp 
 * @return int 
 */

int	main(int argc, char **argv, char **envp)
{
	int	i;
	int	status;

	i = 0;
	status = 0;
	if (argc > 1)
	{
		while (argv[i] && argv[++i])
		{
			argv += i;
			i = 0;
			while (argv[i] && strcmp(argv[i], "|") && strcmp(argv[i], ";"))
				i++;
			if (!strcmp(*argv, "cd"))
				status = msh_cd(argv, i);
			else if (i)
				status = msh_exec(argv, envp, i);
		}
	}
	return (status);
}
