#pragma once
#include <chrono>


class Transaction
{
public:
	enum TransactionType
	{
		None = 0, Credit, Debit
	};
	const char TransactionTypeName[3][8] = { {"None"}, {"Credit"}, {"Debit"} };
// Constructors
														Transaction						();

														Transaction						(const long long& date,
																						 const float amount,
																						 const TransactionType& type,
																						 const std::string& description,
																						 const std::string& account);

// Copy construcor and assignment operator
														Transaction						(const Transaction& t);

					 Transaction&						operator=						(const Transaction&);

					 bool								operator==						(const Transaction&)									const;

					 bool								equalExceptAccount				(const Transaction&)									const;

// Static comparator functions for sorting using qsort
	static			 int								ComparatorByDate				(const Transaction& one,
																						 const Transaction& other)														{ return one.date() < other.date();     }

	static			 int								ComparatorByAmount				(const Transaction& one,
																						 const Transaction& other)														{ return one.amount() < other.amount(); }


// Getters
	inline			 long long							date							()														const					{ return m_date; }

	inline			 float								amount							()														const					{ return m_amnt; }

	inline			 TransactionType					type							()														const					{ return m_type; }

	inline			 std::string_view					description						()														const					{ return m_desc; }

	inline			 std::string_view					account							()														const					{ return m_acct; }

// Setters
	inline			 void								setType							(const Transaction::TransactionType& type)										{ m_type = type; }

	inline			 void								setAccount						(const std::string& acct)														{ m_acct = acct; }


private:
	long long			  m_date;
	float				  m_amnt;
	TransactionType		  m_type;
	std::string			  m_desc,
						  m_acct;
};

std::ostream& operator<<(std::ostream& os, const Transaction& t);
std::istream& operator>>(std::istream& is, Transaction& t);