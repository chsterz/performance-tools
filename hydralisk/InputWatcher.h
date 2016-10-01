#include <vector>
#include <functional>
#include <map>
#include <sys/select.h>
#include <sys/wait.h>
#include <curses.h>
#include <unistd.h>
#include <cstdlib>
#include <signal.h>
#include <algorithm>

#include <iostream>

////////////////////////////////////////////////////////////////////////////////

struct interconnect{
	int from;
	int to;
};

static struct interconnect interconnects[12]={
	{0, 1},
	{1, 0},
	{0, 2},
	{2, 0},
	{0, 3},
	{3, 0},
	{1, 2},
	{2, 1},
	{1, 3},
	{3, 1},
	{2, 3},
	{3, 2}
	
};

class ChildProcess
{
	enum { INVALID = -1 };
public:


////////////////////////////////////////////////////////////////////////////////

	ChildProcess(int interconnect_id = INVALID)
		: m_interconnect_id(interconnect_id)
	{
		if (interconnect_id == INVALID)
			return;

		int pipe_fds[2];
		pipe(pipe_fds);

		m_pid = fork();
		duppipe(pipe_fds[0], pipe_fds[1]);
		execute();
	}
	
////////////////////////////////////////////////////////////////////////////////

	ChildProcess(const ChildProcess & other):
		m_fd(other.m_fd),
		m_pid(other.m_pid),
		m_interconnect_id(other.m_interconnect_id)
	{
		;;
	}

////////////////////////////////////////////////////////////////////////////////

	ssize_t read(size_t nbytes, void *buf) const
	{
		return ::read(m_fd, buf, nbytes);
	}

////////////////////////////////////////////////////////////////////////////////

	float readBandwidth() const
	{
		float res = 0;
		read(sizeof(res), &res);
		return res;
	}

////////////////////////////////////////////////////////////////////////////////

	int kill() const
	{
		close(m_fd);
		int res = ::kill(m_pid, SIGKILL);
		if (!res)
			waitpid(m_pid, nullptr, 0);
		return res;
	}

////////////////////////////////////////////////////////////////////////////////

	int fd() const
	{
		return m_fd;
	}

////////////////////////////////////////////////////////////////////////////////


	int interconnect_id() const
	{
		return m_interconnect_id;
	}

////////////////////////////////////////////////////////////////////////////////

	pid_t pid() const
	{
		return m_pid;
	}

////////////////////////////////////////////////////////////////////////////////

private:
	int m_fd;
	pid_t m_pid;
	int m_interconnect_id;

	bool inParent() const
	{
		return m_pid != 0;
	}

////////////////////////////////////////////////////////////////////////////////

	void duppipe(int dst, int src)
	{
		if (inParent()) {
			m_fd = dst;
			close(src);
		} else {
			close(dst);
			dup2(src, STDOUT_FILENO);
		}
	}

////////////////////////////////////////////////////////////////////////////////

	void execute()
	{
		if (!inParent()) {
			char *prog="numactl";
			
			char node[4];
			snprintf(node, 4, "-N%d", interconnects[m_interconnect_id].from);

			char from[2];
			snprintf(from, 2, "%d", interconnects[m_interconnect_id].from);

			char to[2]; 
			snprintf(to, 2, "%d", interconnects[m_interconnect_id].to);
			
			char *passed_argv[] ={"./gland", from, to, nullptr};
			
			execvp("./gland", passed_argv);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////


class InputWatcher
{
public:
	InputWatcher() :
		m_maxfd(STDIN_FILENO)
	{
		FD_ZERO(&m_set);
		FD_SET(STDIN_FILENO, &m_set);
	}

////////////////////////////////////////////////////////////////////////////////

	void add(const ChildProcess & child)
	{
		FD_SET(child.fd(), &m_set);
		m_children[child.fd()] = child;

		m_maxfd = std::max(m_maxfd, child.fd());
	}

////////////////////////////////////////////////////////////////////////////////

	void removeAndKill(const ChildProcess & child)
	{
		auto proc = std::find_if(m_children.begin(), m_children.end(), [&](auto pair){
			return pair.second.pid() == child.pid();
		});
		m_children.erase(proc);
		FD_CLR(child.fd(), &m_set);
		
		child.kill();
	}

////////////////////////////////////////////////////////////////////////////////

	void setStdinCallback(const std::function<void()> & callback)
	{
		m_stdinCallback = callback;
	}

////////////////////////////////////////////////////////////////////////////////

	std::vector<ChildProcess> select() const
	{
		fd_set testset = m_set;
		std::vector<ChildProcess> res;

		int nready = ::select(m_maxfd + 1, &testset, nullptr, nullptr, nullptr);
		if (nready == -1) 
		{
		  //Fixme crash on resize
		   	std::cerr << "Hydralisk crashed, [Fixme] Resize bug" << std::endl;
			endwin();
			exit(EXIT_FAILURE);
		}
		
		res.reserve(nready);
		if (FD_ISSET(STDIN_FILENO, &testset)) {
			nready--;
			FD_CLR(STDIN_FILENO, &testset);
			m_stdinCallback();
		}

		for (int i = 0; i <= m_maxfd && nready > 0; i++) {
			if (FD_ISSET(i, &testset)) {
				const auto child = m_children.find(i);
				if (child != m_children.cend())
					res.push_back(child->second);
				nready--;
			}
		}
		return res;
	}

////////////////////////////////////////////////////////////////////////////////

private:
	int m_maxfd;
	fd_set m_set;

	std::map<int, ChildProcess> m_children;
	std::function<void()> m_stdinCallback;

};

////////////////////////////////////////////////////////////////////////////////
