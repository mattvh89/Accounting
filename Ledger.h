#pragma once
#include "smart_pointer.h"
#include "Transaction.h"

class Ledger
{
public:
																		Ledger								();


							bool											loadFromFile							(const char* fname, 
																										 const char* pw);

							bool											saveToFile							(const char* fname, 
																										 const char* pw);

							bool											appendToFile							(const char* fname, 
																										 const char* pw,
																										 const Transaction& t);

	inline					       size_t											size								()														const			{ return m_transactions.size(); }

	inline					       Ptr<Transaction>&								        getTransactions							()																		{ return m_transactions; }

	inline		const		               Ptr<Transaction>&									getTransactions							()														const			{ return m_transactions; }

							size_t											addTransaction							(const Transaction&);

							size_t											removeTransaction						(const Transaction&);

							size_t											removeTransaction						(const size_t& index);

							size_t											removeLastTransaction						()																		{ return m_transactions.removeBack(); }
private:
	Ptr<Transaction> m_transactions;
};
