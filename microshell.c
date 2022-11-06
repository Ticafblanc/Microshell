 #include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

enum ctrl
{
	END = 0,
	PIPE = 1,
	SEMI = 2,
};

typedef struct s_cm
{
	int		ctrl;
	char 	**cmd;
	char 	*path;
	int		fd[2];
}t_cmd;

int str_len(char *str)
{
	int i;

	i = 0;
	while(str[i] != '\0')
		i++;
	return (i);
}

int pars(char ***argv, t_cmd *cmd)
{
	int	i;

	i = 0;
	if (**argv && (strcmp(";", **argv) == 0))
	{
		(*argv)++;
		return(0);
	}
	while(**argv && strcmp(";", **argv) != 0 && strcmp("|", **argv) != 0)
	{
		if (i == 0)
			cmd->path = **argv;
		cmd->cmd[i] = **argv;
		i++;
		(*argv)++;
	}
	if (**argv && strcmp(";", **argv) == 0)
	{
		cmd->ctrl = SEMI;
		(*argv)++;
	}
	else if (**argv && strcmp("|", **argv) == 0)
	{
		cmd->ctrl = PIPE;
		(*argv)++;
	}
	else
		cmd->ctrl = END;
	//printf("coucou\n");
	return(1);
}

void	exec(t_cmd *cmd, int *status, char **envp)
{
	pid_t	pid;

	if (strcmp("cd", cmd->cmd[0]) == 0)
	{
		if (cmd->cmd[1])
		{
			if(chdir(cmd->cmd[1]))
			{
				write(2, "error: cd: cannot change directory to ", 40);
				write(2, cmd->cmd[1], str_len(cmd->cmd[1]));
				write(2, "\n", 1);
				*status = EXIT_FAILURE;
			}
			else
                *status = EXIT_SUCCESS;
		}
		else
		{
			write(2, "error: cd: bad arguments\n", 26);
			*status = EXIT_FAILURE;
		}
	}
	else
	{
		if (cmd->ctrl == PIPE)
			if (pipe(cmd->fd))
			{
				write(2, "error: fatal\n", 13);
				exit(EXIT_FAILURE);

			}
		pid = fork();
		if (pid < 0)
		{
			write(2, "error: fatal\n", 13);
			exit(EXIT_FAILURE);
		}
		if(!pid)
		{
			if (cmd->ctrl == PIPE)
			{
				close(cmd->fd[0]);
				if(dup2(cmd->fd[1], 1) < 0)
				{
					write(2, "error: fatal\n", 13);
					exit(EXIT_FAILURE);
				}
				close(cmd->fd[1]);
			}
			execve(cmd->path, cmd->cmd, envp);
			write(2, "error: cannot execute ", 22);
			write(2, cmd->cmd[0], str_len(cmd->cmd[0]));
			write(2, "\n", 1);
			exit(EXIT_FAILURE);
		}
		//printf("%s\n", cmd->cmd[0]);
		if (cmd->ctrl == PIPE)
		{
			close(cmd->fd[1]);
			if(dup2(cmd->fd[0], 0) < 0)
			{
				write(2, "error: fatal\n", 13);
				exit(EXIT_FAILURE);
			}
			close(cmd->fd[0]);
		}
		waitpid(pid, status, 0);
	}
}

int	main(int argc, char **argv, char **envp)
{
	t_cmd  	cmd;
	int		status;
	int		i;

	cmd.cmd = malloc(sizeof(char *) * 20);
	status = EXIT_SUCCESS;
	argv++;
	if (argc > 1)
	{
		while(*argv)
		{
			i = 20;
			if (pars(&argv, &cmd))
				exec(&cmd, &status, envp);
			while(i-- > 0)
				cmd.cmd[i] = NULL;
		}
	}
	free(cmd.cmd);
	return(status);
}


	
