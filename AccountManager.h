#pragma once
#include <map>
#include <string>
#include "Account.h"
#include "smart_pointer.h"

#define SIGNED_IN  true
#define SIGNED_OUT false

#define NUM_DEFAULT_ACCOUNTS 9
const std::string DEFAULT_ACCOUNTS[] = { "Main", "Checking", "Saving", "Expense", "Receivable", "Payable", "WorkDone", "Capital", "Taxes"};

class AccountManager
{
public:
// Constructor
																	AccountManager								(const char* fileName, 
																												 const char* password);

// Setters
							void									updateMainFile								(const char* name);

				static		void									CreateNewMainFile							(const char* name,
																												 const char* pw);

							bool									addAccount									(const char* accountName);

							bool									doubleEntry									(const std::string&   debitAccount,
																												 const std::string&   creditAccount,
																												 const Transaction& transaction);

							bool									removeTransaction							(std::string_view acctname,
																												 const size_t& index);

							void									sortTransactionsByDate						(std::string_view acctname);

							void									sortTransactionsByAmount					(std::string_view acctname);

							void									sortMainAccountByDate						();

// Getters
	inline					bool									isGood										()													const		{ return m_signinSuccess; }

							bool									accountExists								(const std::string& acct)							const;

							Account&								getAccount									(std::string_view acctName);

							void									printAccountNames							()													const;

							Ptr<std::string>						getAccountNames								()													const;

							Ptr<Transaction>						getByDate									(std::string_view accountName,
																												 const long long& days_since_epoch_date)			const;

							int										indexOfAccount								(const std::string& name)							const;
private:
	std::map<std::string, Account>			 m_accounts;
	std::string							     m_name,
											 m_pw;
	bool									 m_signinSuccess;
};

