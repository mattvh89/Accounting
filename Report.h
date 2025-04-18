#pragma once
#include "Transaction.h"
#include "smart_pointer.h"

class Report 
{
public:
																Report								();

																Report								(Ptr<Transaction>& ledger);

							Report&								operator=							(const Report& other);

// Getters
	inline					size_t								getAccountLedgerSize				()											const			{ return m_accountLedger.size(); }

// Setters
							size_t								generateReportByDate				(const long long& days_since_epoch);

							size_t								generateReportByDate				(const long long& dse_start,
																									 const long long& dse_end);

							size_t								generateReportByDate				(const Report&,
																									 const long long& days_since_epoch);

							size_t								generateReportByDate				(const Report&,
																									 const long long& dse_start,
																									 const long long& dse_end);

							size_t								generateReportByDescription			(std::string_view phrase);

							size_t								generateReportByDescription			(const Report&, 
																									 std::string_view phrase);

							size_t								generateReportByAmount				(const double& amount);

							size_t								generateReportByAmount				(const double& amount_lo,
																									 const double& amount_hi);

							size_t								generateReportByAmount				(const Report&,
																									 const double& amount);

							size_t								generateReportByAmount				(const Report&,
																									 const double& amount_lo,
																									 const double& amount_hi);



	inline		const		Ptr<Transaction>&					getWorkingLedger					()											const			{ return m_accountLedger; }

	inline		const		Ptr<Transaction>&					getReport							()											const			{ return m_generatedList; }

	inline					Ptr<Transaction>&					getReport							()															{ return m_generatedList; }

	inline					Ptr<Transaction>					getReportCopy						()											const			{ return m_generatedList; }

							double								calculateTotal						()											const;


private:
	Ptr<Transaction> m_accountLedger;
	Ptr<Transaction> m_generatedList;
};

