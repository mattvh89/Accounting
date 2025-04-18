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
	//Ptr<int> ptrInt(ARRAY, 10);
	//for (size_t i = 0; i < 20; ++i)
	//	ptrInt.insertAt(i, i);

	//ptrInt.insertAt(100, 10);
	//ptrInt.insertAt(200, 16);
	//ptrInt.insertAt(300, ptrInt.size());
	//ptrInt.insertAt(400, ptrInt.size() + 1);

	//for (size_t i = 0; i < ptrInt.size(); ++i)
	//	std::cout << ptrInt[i] << " ";
	return 0;
}
