#include "AccountingMenu.h"


int main(int argc, char** argv)
{
	try
	{
		AccountingMenu menu;
		menu.run();
	}
	catch (std::exception& e) { std::cerr << "From Main: " << e.what() << std::endl; }

	std::cout << "Total memory left:   "  << Ptr<Transaction>::memoryUsage(true) + Ptr<std::string>::memoryUsage(true) + Ptr<AccountManager>::memoryUsage(true) << std::endl;
	std::cout << "Automatic Deletions: "  << Ptr<Transaction>::deletions()       + Ptr<std::string>::deletions()       + Ptr<AccountManager>::deletions()       << std::endl;
	std::cout << "Program exit: Success!"																														<< std::endl;

	return 0;
}
