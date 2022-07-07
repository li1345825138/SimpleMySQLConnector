#pragma once
#include <mysql.h>
#include <exception>
#include <memory.h>
#include <cstring>
#include <map>

using std::exception;
using std::memcpy;
using std::map;
using std::pair;
using std::strlen;
using std::strcmp;
using std::strncmp;
using std::size_t;

/**
 * @brief help to easy connect to MySQL Server
 * @author li1345825138
 * @create date 06-17-2022
 * @version 1.2
 */
class MySQLConnector {
public:
	/**
	 * @brief SQL Connect Failure Exception
	 * inherit from standard exception
	 */
	class ConnectFailureException : public exception {
		virtual const char* what() const override {
			return "SQL Connect Failure";
		}
	};

	/**
	 * @brief SQL Query Exception
	 * inherit from standard exception
	 */
	class QueryFailureException : public exception {
		virtual const char* what() const override {
			return "SQL Query Failure";
		}
	};

	/**
	 * @brief SQL No Element On Result Exception
	 * inherit from standard exception
	 */
	class NoElementException : public exception {
		virtual const char* what() const override {
			return "No Element Exception";
		}
	};

	/**
	 * @brief SQL Database is not exists Exception
	 * inherit from standard exception
	 * Must use with changeDatabase function
	 */
	class DatabaseNotExistsException : public exception {
		virtual const char* what() const override {
			return "Database Is Not Exists";
		}
	};

	/**
	 * @brief The access index is out of bounce
	 * throw exception if access index is not in number of
	 * rows and columns
	 */
	class IndexOutOfBounceException : public exception {
		virtual const char* what() const override {
			return "Index is not in Range";
		}
	};

	/**
	 * @brief The command pointer is null or empty
	 * throw exception if command is nullptr or empty
	 */
	class NullCommandPointerException : public exception {
		virtual const char* what() const override {
			return "Null Command Pointer Exception: SQL Command is Empty";
		}
	};

private:
	MYSQL* connect;
	MYSQL_RES* res;
	uint64_t row_count;
	unsigned int field_count;
	char* hostName;
	char* userName;
	char* password;
	char* dbName;
	int port;
	map<int, map<int, char*>>* dbResult = nullptr;
	

	/**
	 * @brief init and dynamic allocate memory size
	 * @param hostNameLenth - The Length of host name
	 * @param usernameLength - The Length of username
	 * @param passwordLength - The Length of password
	 * @param dbNameLength - The Length of database Name
	 */
	void initMemory(const size_t& hostNameLenth, const size_t& usernameLength, const size_t& passwordLength, const size_t& dbNameLength) {
		this->hostName = new char[hostNameLenth + 1] { 0 };
		this->userName = new char[usernameLength + 1] {0};
		this->password = new char[passwordLength + 1] {0};
		this->dbName = new char[dbNameLength + 1] {0};
	}

	/**
	 * @brief delete all dynamic allocate memory
	 */
	void emptyMemory() {
		if (this->hostName) {
			delete[] this->hostName;
			this->hostName = nullptr;
		}
		if (this->userName) {
			delete[] this->userName;
			this->userName = nullptr;
		}
		if (this->password) {
			delete[] this->password;
			this->password = nullptr;
		}
		if (this->dbName) {
			delete[] this->dbName;
			this->dbName = nullptr;
		}
		cleanResultMemory();
	}

	/**
	 * @brief clear Result Vector Memory
	 */
	void cleanResultMemory() {
		if (this->dbResult) {
			delete this->dbResult;
			this->dbResult = nullptr;
			this->row_count = 0;
			this->field_count = 0;
		}
	}

	/**
	 * @brief Delete default constructor
	 */
	MySQLConnector() = delete;

	/**
	 * @brief Delete default copy constructor
	 */
	MySQLConnector(const MySQLConnector&) = delete;

public:
	/**
	 * @brief Constructor
	 * @param hostName - SQL login host name
	 * @param userName - SQL login username
	 * @param password - SQL login password
	 * @param dbName - SQL which database will be use after login
	 * @param port - SQL service port to connect
	 */
	MySQLConnector(const char* hostName, const char* userName, const char* password, const char* dbName, const int& port) {
		this->connect = nullptr;
		this->res = nullptr;
		size_t hostNameLength = strlen(hostName);
		size_t userNameLength = strlen(userName);
		size_t passwordLength = strlen(password);
		size_t dbNameLength = strlen(dbName);
		initMemory(hostNameLength, userNameLength, passwordLength, dbNameLength);
		memcpy(this->hostName, hostName, sizeof(char) * hostNameLength);
		memcpy(this->userName, userName, sizeof(char) * userNameLength);
		memcpy(this->password, password, sizeof(char) * passwordLength);
		memcpy(this->dbName, dbName, sizeof(char) * dbNameLength);
		this->port = port;
		this->row_count = 0;
		this->field_count = 0;
	}

	/**
	 * @brief start sql connection
	 * @exception throw ConnectFailureException if connect failure
	 */
	void startConnection() {
		this->connect = mysql_init(nullptr);
		if (!mysql_real_connect(this->connect, this->hostName, this->userName, this->password, this->dbName, this->port, nullptr, NULL)) {
			emptyMemory();
			throw ConnectFailureException();
		}
	}

	/**
	 * @brief send SQL command to query Database
	 * @exception throw NullCommandPointerException if command argument is nullptr or empty
	 * @exception throw QueryFailureException if query fail
	 * @param command - SQL command to send
	 */
	void queryDatabase(const char* command) {
		if (!command || strcmp(command, "") == 0) {
			throw NullCommandPointerException();
		}

		if (mysql_query(this->connect, command)) {
			throw QueryFailureException();
		}

		// Only command start with SELECT will enter this condition to store result into map
		if (strncmp(command, "SELECT", 6) == 0 || strncmp(command, "select", 6) == 0) {
			this->res = mysql_store_result(this->connect);
			this->row_count = mysql_num_rows(this->res);
			this->field_count = mysql_num_fields(this->res);

			// store the data into map
			cleanResultMemory();

			this->dbResult = new map<int, map<int, char*>>();
			for (uint64_t i = 0; i < this->row_count; i++) {
				MYSQL_ROW row = mysql_fetch_row(this->res);
				map<int, char*> rowMap;
				for (unsigned int j = 0; j < this->field_count; j++) {
					if (!row[j]) {
						rowMap.insert(pair<int, char*>(j, "NULL"));
						continue;
					}
					rowMap.insert(pair<int, char*>(j, row[j]));
				}
				this->dbResult->insert(pair<int, map<int, char*>>(i, rowMap));
			}
		}
	}

	/**
	 * @brief Return total row count after get query Result
	 * @return How many row in query result
	 */
	const uint64_t getRowCount() const {
		return this->row_count;
	}

	/**
	 * @brief Return total field count after query Result
	 * @return How many field in query result
	 */
	const unsigned int getFieldCount() const {
		return this->field_count;
	}

	/**
	 * @brief switch to another database
	 * @exception throw exception if new database name is not exists
	 */
	void changeDatabase(const char* newDBName) const {
		if (!newDBName)
			return;
		if (mysql_select_db(this->connect, newDBName)) {
			throw DatabaseNotExistsException();
		}
	}

	/**
	 * @brief get a single row of result
	 * @return the single row result from MySQL
	 */
	[[deprecated("This Method is deprecated, use getSingleFieldResult instead")]]
	const MYSQL_ROW getSingleRowResult() const {
		return mysql_fetch_row(this->res);
	}

	/**
	 * @brief get data from specific index
	 * @param indexRow - data row index
	 * @param indexColumn - data column index
	 * @exception throw IndexOutOfBounceException if index is not in range
	 * @return the data from specific index
	 */
	const char* getSingleFieldResult(const uint64_t& indexRow, const unsigned int& indexColumn) const {
		if (indexRow < 0 || indexRow >= this->row_count || indexColumn < 0 || indexColumn >= this->field_count) {
			throw IndexOutOfBounceException();
		}
		if (!this->dbResult) {
			return "NULL";
		}
		return this->dbResult->at(indexRow).at(indexColumn);
	}

	/**
	 * @brief Destructor
	 */
	~MySQLConnector() {
		if (this->res)
			mysql_free_result(this->res);
		if (this->connect)
			mysql_close(this->connect);
		emptyMemory();
	}
};