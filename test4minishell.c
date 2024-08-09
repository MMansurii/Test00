
/////// minishellproject
/////// 11111
#include "minishell.h"

static int	export_valid(char **vec, int i, char **kv)
{
	int	j;

	j = 0;
	if (!kv[0] || !*kv[0] || ft_isdigit(kv[0][0]) || vec[i][0] == '=')
		return (1);
	while (kv[0][j])
		if (ft_strchr("|<>[],.:/{}/+^%#@!~=-?&*", kv[0][j++]))
			return (1);
	return (0);
}

void	ft_export(char **vec, char ***env)
{
	int		i;
	char	**kv;

	i = 1;
	while (vec[i])
	{
		kv = ft_split(vec[i], '=');
		if (!vec[i] || !*vec[i] || !*kv)
		{
			terminate("export", ft_strjoin(vec[i],
					": is not a valid identifier adsfads"));
			i++;
			continue ;
		}
		if (export_valid(vec, i, kv))
			terminate("export", ft_strjoin(vec[i],
					": is not a valid identifier"));
		if (ft_veclen(kv) == 2)
			set_env(env, kv[0], kv[1]);
		ft_vecfree(kv);
		i++;
	}
}

void	ft_echo(char **vec)
{
	int	i;
	int	flag;

	i = 1;
	flag = 0;
	if (vec[1])
	{
		while (vec[i] && (ft_strncmp(vec[i], "-n", 2) == 0
				&& ft_strlen(vec[i]) == 2))
		{
			flag = 1;
			i++;
		}
	}
	while (vec[i])
	{
		ft_putstr_fd(vec[i++], 1);
		if (vec[i])
			ft_putchar_fd(' ', 1);
	}
	if (!flag)
		ft_putstr_fd("\n", 1);
}

void	ft_exit(char **v)
{
	int	code;

	ft_putendl_fd("exit", 1);
	if (!v[0])
		code = 0;
	else if (ft_strisnum(v[0] + 4))
		code = ft_atoi(v[0] + 4);
	else
		code = 255;
	exit(code);
}


//////////////////2
char	*get_env(char **env, char *key)
{
	int	i;

	if (!key || !*key)
		return (ft_strdup(""));
	i = 0;
	while (env[i])
	{
		if (!ft_strncmp(env[i], key, ft_strlen(key)))
			return (env[i] + ft_strlen(key) + 1);
		i++;
	}
	return (ft_strdup(""));
}

void	set_env(char ***env, char *key, char *value)
{
	int	i;

	if (!key || !*key)
		return ;
	i = 0;
	while ((*env)[i])
	{
		if (!ft_strncmp((*env)[i], key, ft_strlen(key)))
		{
			free((*env)[i]);
			(*env)[i] = ft_strjoin(ft_strjoin(key, "="), value);
			return ;
		}
		i++;
	}
	ft_vecadd(env, ft_strjoin(ft_strjoin(key, "="), value));
}

static char	*join_path(char *s1, char *s2)
{
	char	*parent;
	char	*path;

	parent = ft_strjoin(s1, "/");
	path = ft_strjoin(parent, s2);
	free(parent);
	return (path);
}

char	*get_path(char *program)
{
	extern char	**environ;
	char		**v;
	char		*path;
	int			i;

	i = 0;
	v = ft_split(get_env(environ, "PATH"), ':');
	while (v[i])
	{
		path = join_path(v[i], program);
		if (!access(path, X_OK))
			return (path);
		free(path);
		path = NULL;
		i++;
	}
	ft_vecfree(v);
	return (NULL);
}

void	ft_cd(char **vec, char ***env)
{
	int	j;

	if (!vec[1])
		j = chdir(get_env(*env, "HOME"));
	else if (!ft_strncmp(vec[1], "-", 1))
		j = chdir(get_env(*env, "OLD_PWD"));
	else
		j = chdir(vec[1]);
	if (j != 0)
		return (terminate("cd", "No such file or directory"));
	set_env(env, "OLD_PWD", get_env(*env, "PWD"));
	set_env(env, "PWD", getcwd(NULL, 0));
}

////////////////3
int	h_up(int count, int key)
{
	HIST_ENTRY	*entry;

	(void)count;
	(void)key;
	entry = previous_history();
	if (entry)
	{
		rl_replace_line(entry->line, 0);
		rl_point = rl_end;
		rl_redisplay();
	}
	return (0);
}

int	h_down(int count, int key)
{
	HIST_ENTRY	*entry;

	(void)count;
	(void)key;
	entry = next_history();
	if (entry)
		rl_replace_line(entry->line, 0);
	else
		rl_replace_line("", 0);
	rl_point = rl_end;
	rl_redisplay();
	return (0);
}

void	disable_ctrl_sign(void)
{
	struct termios	term;

	tcgetattr(0, &term);
	term.c_lflag &= ~(ECHOCTL);
	tcsetattr(0, TCSANOW, &term);
}

void	handle_ctrl_c(int sig)
{
	(void)sig;
	ft_putstr_fd("\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
}

int	main(void)
{
	char		*line;
	char		**tokens;
	extern char	**environ;
	char		**vv;

	vv = ft_vecdup(environ);
	using_history();
	rl_bind_keyseq("\\e[A", &h_up);
	rl_bind_keyseq("\\e[B", &h_down);
	disable_ctrl_sign();
	signal(SIGINT, handle_ctrl_c);
	while (1)
	{
		line = readline("minishell$ ");
		if (!line)
			break ;
		if (!*line)
			continue ;
		tokens = ft_split(line, '|');
		process(line, tokens, &vv);
	}
	return (0);
}

///////////////4
extern int		g_fd[2];

static int	parse_redirect(char **v, int i, int (*then)(char **, int))
{
	if (ft_strlen(v[i]) == 1 && (v[i][0] == '<' || v[i][0] == '>'))
	{
		if (v[i][0] == '<')
		{
			g_fd[0] = open(v[i + 1], O_RDONLY);
			if (g_fd < 0)
				return (terminate(v[i + 1], NULL), 1);
			v[i] = ft_strrepl(v[i], "<", "");
			v[i + 1] = ft_strrepl(v[i + 1], v[i + 1], "");
		}
		else if (v[i][0] == '>')
		{
			g_fd[1] = open(v[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
			v[i] = ft_strrepl(v[i], ">", "");
		}
		v[i + 1] = ft_strrepl(v[i + 1], v[i + 1], "");
	}
	else if (ft_strlen(v[i]) == 2 && !ft_strncmp(v[i], ">>", 2))
	{
		g_fd[1] = open(v[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
		v[i] = ft_strrepl(v[i], ">>", "");
		v[i + 1] = ft_strrepl(v[i + 1], v[i + 1], "");
	}
	then(v, i);
	return (0);
}

void	parse_env_vars(char **v, char **env)
{
	int		i;
	char	*tmp;
	char	*loc;
	char	*end;
	char	*key;

	i = 0;
	while (v[i])
	{
		loc = v[i];
		while (ft_strchr(loc, '$'))
		{
			loc = ft_strchr(loc, '$');
			end = loc + 1;
			while (ft_isalnum(*end) || *end == '_')
				end++;
			key = ft_substr(loc, 1, end - loc - 1);
			tmp = get_env(env, key);
			v[i] = ft_strrepl(v[i], ft_strjoin("$", key), tmp);
			free(key);
			loc = end;
		}
		i++;
	}
}

t_quote_parsed	parse_quotes(const char *s)
{
	t_quote_parsed	p;
	int				ij[2];

	ij[0] = -1;
	ij[1] = 0;
	p.str = ft_calloc(strlen(s) + 1, sizeof(char));
	p.end = 0;
	while (s[++ij[0]])
	{
		if (s[ij[0]] == '\'' || s[ij[0]] == '"')
		{
			if (p.end == 0)
				p.end = s[ij[0]];
			else if (p.end == s[ij[0]])
				p.end = 0;
			else
				p.str[ij[1]++] = s[ij[0]];
		}
		else
			p.str[ij[1]++] = s[ij[0]];
	}
	return (p);
}

static int	parse_heredoc(char **v, int i)
{
	char	*hd_str;
	char	*end;

	hd_str = NULL;
	end = NULL;
	if (parse_quotes(v[i]).end)
		end = (char [2]){parse_quotes(v[i]).end, 0};
	else if ((ft_strlen(v[i]) == 2 && !ft_strncmp(v[i], "<<", 2)))
		end = v[i + 1];
	else if (ft_strnstr(v[i], "<<", ft_strlen(v[i])))
		end = ft_strchr(v[i], '<') + 2;
	if (end)
		hd_str = here_doc(end);
	if (hd_str)
	{
		if (!ft_strcmp(hd_str, "ERR"))
			return (2);
		v[i] = ft_strrepl(v[i], "<", "");
		v[i] = ft_strrepl(v[i], end, hd_str);
		free(hd_str);
	}
	return (0);
}

char	*parse(char *s, char **env)
{
	int		in[2];
	char	*tmp;
	char	*tmp2;
	char	*tmp3;
	char	**v;

	in[0] = 0;
	v = ft_split(s, ' ');
	parse_env_vars(v, env);
	in[1] = ft_veclen(v);
	while (v[in[0]++])
		parse_redirect(v, in[0] - 1, parse_heredoc);
	if (g_fd[0] == -1 || g_fd[1] == -1)
	{
		g_fd[1] = 1;
		g_fd[0] = 0;
		return (terminate(NULL, NULL), ft_strdup(""));
	}
	tmp = ft_vecnjoin(v, " ", in[1]);
	tmp2 = ft_strtrim(tmp, " ");
	tmp3 = ft_strrmchr(tmp2, "\\;\"'");
	free(tmp);
	free(tmp2);
	ft_vecfree(v);
	return (tmp3);
}

////////////////////5
int			g_fd[2] = {0, 1};

static void	run_builtin(char **v, char ***env)
{
	int	i;

	i = 0;
	if (!ft_strcmp(v[0], "echo"))
		ft_echo(v);
	else if (!ft_strcmp(v[0], "cd"))
		ft_cd(v, env);
	else if (!ft_strcmp(v[0], "pwd"))
		ft_putendl_fd(get_env(*env, "PWD"), 1);
	else if (!ft_strcmp(v[0], "export"))
		ft_export(v, env);
	else if (!ft_strcmp(v[0], "unset"))
		set_env(env, v[1], NULL);
	else if (!ft_strcmp(v[0], "env"))
		while ((*env)[i])
			ft_putendl_fd((*env)[i++], 1);
	free(v);
}

static void	run(char *s, char ***env)
{
	char	*file;
	char	**v;
	char	**builtins;

	builtins = ft_split("echo cd pwd export unset env exit", ' ');
	v = ft_split(s, ' ');
	if (!v || !*v)
		return ;
	if (ft_vecget(builtins, v[0]))
		return (run_builtin(v, env));
	if (v[0][0] == '/')
		file = ft_strdup(v[0]);
	else
		file = get_path(v[0]);
	if (!file)
		return (terminate(v[0], "command not found"));
	if (execve(file, v, NULL) < 0)
		terminate(file, NULL);
	free(file);
	ft_vecfree(v);
}

void	pipe_child(int d[2], int pipefd[2], char *cmd, char ***env)
{
	int	is_not_last;
	int	in_fd;

	is_not_last = d[0];
	in_fd = d[1];
	if (in_fd != 0)
	{
		dup2(in_fd, 0);
		close(in_fd);
	}
	if (is_not_last)
	{
		dup2(pipefd[1], 1);
		close(pipefd[1]);
	}
	else if (g_fd[1] != 1)
		dup2(g_fd[1], 1);
	run(cmd, env);
}

void	pipe_parent(int *d[3], int pipefd[2])
{
	int	i;
	int	n;
	int	*in_fd;

	i = *d[0];
	n = *d[1];
	in_fd = d[2];
	if (*in_fd != g_fd[0])
		close(*in_fd);
	if (i < n - 1)
	{
		close(pipefd[1]);
		*in_fd = pipefd[0];
	}
	if (i == n - 1 && g_fd[1] != 1)
		close(g_fd[1]);
}

void	run_pipes(char **commands, int n, char ***env)
{
	int	pipefd[2];
	int	i;
	int	in_fd;
	int	status;

	i = -1;
	in_fd = g_fd[0];
	while (++i < n)
	{
		if (i < n - 1)
			pipe(pipefd);
		if (fork() == 0)
			pipe_child((int [2]){i < n - 1, in_fd}, pipefd, commands[i], env);
		else
			pipe_parent((int *[3]){&i, &n, &in_fd}, pipefd);
	}
	while (i--)
		wait(&status);
	set_env(env, "?", ft_itoa(WEXITSTATUS(status)));
}

///////////////////////6
void	terminate(char *cmd, char *reason)
{
	ft_putstr_fd("minishell: ", 2);
	if (cmd)
	{
		ft_putstr_fd(cmd, 2);
		ft_putstr_fd(": ", 2);
	}
	if (reason)
		ft_putstr_fd(reason, 2);
	else
		ft_putstr_fd(strerror(errno), 2);
	ft_putstr_fd("\n", 2);
}

char	*here_doc(char *end)
{
	char	*line;
	char	*str;
	char	*tmp;

	if (!end || !*end)
		return (terminate(NULL, ERR_HD), ft_strdup("ERR"));
	str = ft_strdup("");
	while (1)
	{
		line = readline("> ");
		if (!line || (ft_strlen(line) == ft_strlen(end) && !ft_strncmp(line,
					end, ft_strlen(end))))
			return (free(line), str);
		tmp = str;
		str = ft_strjoin(str, ft_strjoin("\n", line));
		if (((*end == '"' || *end == '\'') && parse_quotes(line).end == *end))
			return (ft_substr(str, 0, ft_strchr(str, *end) - str));
		free(tmp);
		free(line);
	}
	return (str);
}

void	process(char *line, char **tokens, char ***env)
{
	int	i;

	i = -1;
	add_history(line);
	while (tokens[++i])
		tokens[i] = parse(tokens[i], *env);
	if (!ft_strncmp(line, "exit ", 5))
		ft_exit(tokens);
	run_pipes(tokens, ft_veclen(tokens), env);
	free(line);
	ft_vecfree(tokens);
	write_history("~/.minishell_history");
}

//////////////// header 
