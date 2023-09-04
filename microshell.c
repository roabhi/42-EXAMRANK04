/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rabril-h <rabril-h@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/19 21:02:06 by rabril-h          #+#    #+#             */
/*   Updated: 2023/09/04 22:02:13 by rabril-h         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * @brief simple function to count char. We are not using
 * the *str++ approach because we want to preserve the string
 * integrity
 * 
 * @param str 
 * @return int 
 */

int	msh_strlen(char *str)
{
	int	i;

	i = 0;
	while (str[i])
		i++;
	return (i);
}

/**
 * @brief simple function to output a string on the
 * standard output as error. Write as much chars as
 * the length of the string passed as argument
 * 
 * @param str 
 * @return int 
 */

int	msh_error(char *str)
{
	write(2, str, msh_strlen(str));
	return (1);
}

/**
 * @brief function that takes care of the execution (execve)
 * of commands
 * 
 * This funcion takes a string which can be the whole string or until
 * it reaches a ; (not included in the string)
 * 
 * The function tries to to execute the command with the name passed as argv[0]
 * which is the first portion of the passes string followed by the rest of the chain string
 * 
 * If the execution is successful the control is sent back to the program and no error is printed (everything after execve line)
 * 
 * 
 * 
 * @param argv 
 * @param i 
 * @param tmp_fd 
 * @param env 
 * @return int 
 */

int	msh_exec(char *argv[], int i, int tmp_fd, char *env[])
{
	argv[i] = NULL; // ? This closes the string chain so we do not overflow 
	dup2(tmp_fd, 0); // ? This makes our tmp_fd the same as the standard input like if we are writting text
	close(tmp_fd); // ? We close our tmp_fd
	execve(argv[0], argv, env);
	msh_error("error: cannot execute ");
	msh_error(argv[0]);
	msh_error("\n");
	return (1);
}

int	main(int argc, char *argv[], char *env[])
{
	int	i; // ? we need a counter
	int	fd[2]; // ? we need a couple of file descriptors for pipes
	int	tmp_fd; // ? we need a temporary fd
	int	pid; // ? we need our reference for the process

	(void)argc; // ? is needed in exam, because the exam tester compiles with -Wall -Wextra -Werror
	i = 0; // ? Set counter
	tmp_fd = dup(0); // ? set our tmp_fd as the standard input since we will be sending from other place. We store the position.
	// ? Loop through as long as we a char and anything after that
	while (argv[i] && argv[i + 1])
	{
		argv = &argv[i + 1]; // ? We move the pointer as we go through the string chain. This can only happen if there is actually to loop through thanks to our previous while
		i = 0; // ? Reset the counter once the pointer has been moved so no need for another counter such as j or whatever
		while (argv[i] && strcmp(argv[i], ";") && strcmp(argv[i], "|"))
			i++; // ? increment the counter as long as there is a char and that char is NOT ; AND is NOT | (strcmp returns 1 when there is no match)
		if (!strcmp(argv[0], "cd")) // ? if the first argument (command) for the stirng chain is cd
		{
			if (i != 2) // ? more or less than 2 arguments for cd is invalid, hence print error
				msh_error("error: cd: bad arguments\n");
			else if (chdir(argv[1]) == -1) // ? If we try to change the directory and there is an error, print error (the dir does not actually change in microshell. Do not freak out)
			{
				msh_error("error: cd: cannot change directory to ");
				msh_error(argv[1]);
				msh_error("\n");
			}
		}
		else if (i != 0 && (argv[i] == NULL || strcmp(argv[i], ";") == 0)) //exec in stdout
		{
			pid = fork();
			if (pid == -1)
				return 1;
			if (pid == 0)
			{
				if (msh_exec(argv, i, tmp_fd, env))
					return (1);
			}
			else
			{
				close(tmp_fd);
				waitpid(pid, NULL, WUNTRACED);
				tmp_fd = dup(0);
			}
		}
		else if(i != 0 && strcmp(argv[i], "|") == 0) //pipe
		{
			pipe(fd);
			pid = fork();
			if (pid == 0)
			{
				dup2(fd[1], 1);
				close(fd[0]);
				if (msh_exec(argv, i, tmp_fd, env))
					return (1);
			}
			else
			{
				close(fd[1]);
				close(tmp_fd);
				tmp_fd = fd[0];
			}
		}
	} // ? End of main loop
	close(tmp_fd); // ? we close our tmp_fd
	return (0); // ? default return as sucess
}
