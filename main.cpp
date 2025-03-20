#include "AccountingMenu.h"

int main(int argc, char** argv)
{
	try
	{
		AccountingMenu menu;
		menu.run();
	}
	catch (std::exception& e) { std::cerr << e.what() << std::endl; }
	return 0;
}
