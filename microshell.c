/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rabril-h <rabril-h@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/19 21:02:06 by rabril-h          #+#    #+#             */
/*   Updated: 2023/08/19 21:32:52 by rabril-h         ###   ########.fr       */
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
 * @brief This is the main function and the entry point of th program
 * 
 * It takes argc as number of arguments
 * It takes argv as the line passed to the program (the arguments)
 * It takes also the enviroment variables as default. We would need them later for the execve
 * 
 * We have a main counter i and a var to hold the exit status for the program both set to 0 at the begining
 * so we assume program will succeed by setting status to 0
 * 
 * If more than jsut ./microshell has been passed to the program it means we need to to stuff
 * Then while we have an argument and a following argument to that previous one "while (argv[i] && argv[++i])"
 * Then we move the pointer of argv (which is a **char so me move the first pointer) to the current state of the counter
 * 
 * This would mean that for the first iteration in which we start with i = 0
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
	(void)envp;
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
		}
	}
	return (status);
}