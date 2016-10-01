#include <stdlib.h>
#include <iostream>
#include <unistd.h>                  /*  for sleep()  */
#include <curses.h>
#include <algorithm>

#include "InputWatcher.h"

struct link{
	int posy;
	int posx;
	int numInstances;
	float throughput;
	char text[20];
	int left;
	int right;
	int up;
	int down;
};

int main(void) {

	WINDOW * mainwin;

	if ( (mainwin = initscr()) == NULL ) exit(EXIT_FAILURE);

	keypad(stdscr, TRUE);
	noecho();
	curs_set(0);

	attrset(A_BOLD);
	mvaddstr(0, 0, "Hydralisk blocks interconnects");
	attrset(A_DIM);
	mvaddstr(1, 1, "Use arrow keys to navigate");
	mvaddstr(2, 1, "Use +/- to add load to the selected link");
	attrset(A_NORMAL);

	mvaddstr(7 ,  6, "N0---------------------N1");
	mvaddstr(8 ,  6, "|\\_                   _/|");
	mvaddstr(9 ,  6, "|  \\                _/  |");
	mvaddstr(10,  6, "|     \\_          _/    |");
	mvaddstr(11,  6, "|       \\_      _/      |");
	mvaddstr(12,  6, "^         \\_  _/        ^");
	mvaddstr(13,  6, "|           \\/          |");
	mvaddstr(14,  6, "|          _/\\_         |");
	mvaddstr(15,  6, "v         /    \\_       v");
	mvaddstr(16,  6, "|      _/        \\_     |");
	mvaddstr(17,  6, "|   _/             \\_   |");
	mvaddstr(18,  6, "| _/                 \\_ |");
	mvaddstr(19,  6, "|/                     \\|");
	mvaddstr(20,  6, "N2---------------------N3");

	refresh();

	struct link links[12]={
		{ 6, 13, 0, 0.0f, "%2dx%1.3fG >", 5, 9, 0, 1}, // 0: N0 -> N1 
		{ 8, 12, 0, 0.0f, "<%2dx%1.3fG",  5, 9, 0, 5},  // 1: N1 -> N0

		{14,  2, 0, 0.0f, "%2dx%1.3fG", 2, 8, 3, 7},   // 2: N0 -> N2
		{13,  2, 0, 0.0f, "%2dx%1.3fG", 3, 9, 4, 2},   // 3: N2 -> N0

		{11,  8, 0, 0.0f, "%2dx%1.3fG >", 3, 9, 5, 3}, // 4: N0 -> N3
		{10,  9, 0, 0.0f, "<%2dx%1.3fG", 3, 9, 1, 4},  // 5: N3 -> N0

		{17,  8, 0, 0.0f, "%2dx%1.3fG >", 2, 8, 7, 10}, // 6: N1 -> N2
		{16,  9, 0, 0.0f, "<%2dx%1.3fG", 2, 8, 2, 6},  // 7: N2 -> N1

		{14, 26, 0, 0.0f, "%2dx%1.3fG", 2, 8, 9, 10},   // 8: N1 -> N3
		{13, 26, 0, 0.0f, "%2dx%1.3fG", 3, 9, 1, 8},   // 9: N3 -> N1

		{19, 13, 0, 0.0f, "%2dx%1.3fG >", 6, 8, 6, 11}, //10: N2 -> N3
		{21, 12, 0, 0.0f, "<%2dx%1.3fG", 6, 8, 10, 11},  //11: N3 -> N2
	};

	int currentLink = 0;

	std::vector<ChildProcess> blockers;
	InputWatcher watcher;

	watcher.setStdinCallback([&]{
		switch ( getch() )
		{
			case KEY_LEFT:
				currentLink = links[currentLink].left;
				break;
			case KEY_RIGHT:    	
				currentLink = links[currentLink].right;
				break;
			case KEY_UP:
				currentLink = links[currentLink].up;
				break;
			case KEY_DOWN:
				currentLink = links[currentLink].down;
				break;
			case '+':
				if (links[currentLink].numInstances < 20) {
					links[currentLink].numInstances++;
					blockers.emplace_back(currentLink);
					watcher.add(blockers.back());
				}
				break;
			case '-':
				if (links[currentLink].numInstances > 0) 
				{
					links[currentLink].numInstances--;
					if(links[currentLink].numInstances == 0) links[currentLink].throughput = 0;
					const auto toKill = std::find_if(blockers.cbegin(), blockers.cend(), [&](const auto & blocker){
						return blocker.interconnect_id() == currentLink;
					});
					if (toKill != blockers.cend()) {
						watcher.removeAndKill(*toKill);
						blockers.erase(toKill);
					}
				}
				break;
			case 'q':
				endwin();
				exit(EXIT_SUCCESS);
				break;
			default: break;
		}

	});

	while(1)
	{
		for(int i = 0; i < 12; i++)
		{
			if( i == currentLink ){ attrset(A_STANDOUT);}
			else {attrset(A_BOLD);}
			mvprintw(links[i].posy, links[i].posx, links[i].text, links[i].numInstances, links[i].throughput);
			attrset(A_NORMAL);
		}

		refresh();
		for (const auto & child: watcher.select()) {
			links[child.interconnect_id()].throughput = child.readBandwidth();
		}
		refresh();
	}

	delwin(mainwin);
	endwin();
	refresh();

	return EXIT_SUCCESS;
}
