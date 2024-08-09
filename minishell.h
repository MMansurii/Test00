#ifndef MINISHELL_H
# define MINISHELL_H

# include "./libft/libft.h"
# include <errno.h>
# include <fcntl.h>
# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/wait.h>
# include <termios.h>
# include <unistd.h>
# include <readline/history.h>
# include <readline/readline.h>

# define ERR_HD "syntax error near unexpected token `newline'"

void			process(char *line, char **tokens, char ***env);

void			run_pipes(char **commands, int n, char ***env);
void			terminate(char *cmd, char *reason);

char			*get_env(char **env, char *key);
void			set_env(char ***env, char *key, char *value);
char			*get_path(char *program);

char			*parse(char *s, char **env);
void			print_vec(char **vec);
char			*here_doc(char *end);

typedef struct s_quote_parsed
{
	char		*str;
	char		end;
}				t_quote_parsed;

t_quote_parsed	parse_quotes(const char *s);
/* Builtins */
void			ft_echo(char **v);
void			ft_cd(char **v, char ***env);
void			ft_export(char **v, char ***env);
void			ft_exit(char **v);

#endif