#include <iostream>
#include <iomanip>
#include "MySQLConnector.hpp"

using std::cout;
using std::endl;
using std::ios;
using std::setw;


// this is example file for using MySQLConnector
int main(int argc, char* argv[])
{
	MySQLConnector* sqlconn = new MySQLConnector("hostname", "username", "sql password", "database to use", 3306); //3306 is default sql port, change other port number if your sql port is other than 3306
	cout.setf(ios::left);
	try {
		sqlconn->startConnection();
		cout << "SQL Database connect success" << endl;
		sqlconn->queryDatabase("sql command go here");

		// get how many row on result
		for (uint64_t row = 0; row < sqlconn->getRowCount(); row++) {

			// get how many field on a single row
			for (unsigned int column = 0; column < sqlconn->getFieldCount(); column++) {

				// print result by using row and column index like 2 dimension array
				cout << setw(15) << sqlconn->getSingleFieldResult(row, column);
			}
			cout << endl;
		}

		// free the source
		delete sqlconn;
		sqlconn = nullptr;
	}
	catch (exception& ex) {
		delete sqlconn;
		sqlconn = nullptr;
		cout << ex.what() << endl;
	}
	return 0;
}