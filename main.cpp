#include "AccountingMenu.h"


int main()
{
	try
	{
		AccountingMenu menu;
		menu.run();
	}
	catch (std::exception& e) { std::cerr << "From Main: " << e.what() << std::endl; }

	std::cout << "Total memory left:   "  << Ptr<Transaction>::memoryUsage(true) + Ptr<std::string>::memoryUsage(true) + Ptr<AccountManager>::memoryUsage(true) << '\n';
	std::cout << "Automatic Deletions: "  << Ptr<Transaction>::deletions()       + Ptr<std::string>::deletions()       + Ptr<AccountManager>::deletions()       << '\n';
	std::cout << "Program exit: Success!"																														<< std::endl;

	return 0;
}
