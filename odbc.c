#include <stdio.h>
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>

SQLHENV  henv=NULL;
SQLHDBC  hdbc=NULL;

int ODBCConnectDB(SQLCHAR* ds, SQLCHAR* user, SQLCHAR* pw)
{
    SQLRETURN  rc;

    hdbc=NULL;
    henv=NULL;

    // Allocate environment handle
    rc=SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (rc==SQL_SUCCESS || rc==SQL_SUCCESS_WITH_INFO)
    {
        //  Set the ODBC version environment attribute
        rc=SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

        if (rc==SQL_SUCCESS || rc==SQL_SUCCESS_WITH_INFO)
        {
            // Allocate connection handle
            rc=SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

            if (rc==SQL_SUCCESS || rc==SQL_SUCCESS_WITH_INFO)
            {
                // Set login timeout to 5 seconds.
                SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

                // Connect to data source
                rc=SQLConnect(hdbc, ds, SQL_NTS, user, SQL_NTS, pw, SQL_NTS);

                if (rc==SQL_SUCCESS || rc==SQL_SUCCESS_WITH_INFO)
                {
                    return 1;
                }
                SQLDisconnect(hdbc);
            }
            SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        }
    }
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    hdbc=NULL;
    henv=NULL;

    return 0;
}

int ODBCDisconnectDB()
{
    if(hdbc)
    {
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    }
    if(henv) SQLFreeHandle(SQL_HANDLE_ENV, henv);
    hdbc=NULL;
    henv=NULL;
    return 1;
}

int db_exec_stmt(char* stmt_str)
{
    SQLHSTMT hstmt;
    SQLRETURN  rc;

    if(henv==NULL || hdbc==NULL) return 0;

    /* Allocate statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if(!(rc==SQL_SUCCESS || rc==SQL_SUCCESS_WITH_INFO) ) return 0;

    rc = SQLExecDirect(hstmt, stmt_str, SQL_NTS);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    if(rc==SQL_SUCCESS || rc==SQL_SUCCESS_WITH_INFO) return 1;
    if(rc==SQL_NO_DATA) return 1;

    printf("sqlerr: %d\n", rc);
    return 0;
}

int db_fetch()
{
    SQLRETURN r;
    SQLHSTMT hstmt;
    SQLCHAR sql[256];
    SQLLEN n;
    SQLINTEGER id;
    SQLCHAR name[20];
    SQLREAL age;
    SQL_DATE_STRUCT birthday;
    SQL_TIMESTAMP_STRUCT create_timestamp;

    if(hdbc==NULL || henv==NULL) return 0;

    r=SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if(!(r==SQL_SUCCESS||r==SQL_SUCCESS_WITH_INFO)) return 0;

    strcpy(sql,"select id,name,age,birthday,create_timestamp from employee");
    r=SQLExecDirect(hstmt, sql, SQL_NTS);
    if(r!=SQL_SUCCESS)
    {
        printf("cannot access [%s]\n", sql);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return 0;
    }

    while(1)
    {
        r=SQLFetch(hstmt);
        if(r==SQL_SUCCESS||r==SQL_SUCCESS_WITH_INFO)
        {
            r=SQLGetData(hstmt, 1, SQL_C_ULONG, &id, 0, &n);
            r=SQLGetData(hstmt, 2, SQL_C_CHAR, name, 20, &n);
            r=SQLGetData(hstmt, 3, SQL_C_FLOAT, &age, 0, &n);
            r=SQLGetData(hstmt, 4, SQL_C_TYPE_DATE, &birthday, 0, &n);
            r=SQLGetData(hstmt, 5, SQL_C_TYPE_TIMESTAMP,&create_timestamp,0,&n);
            printf("%d, %s\n", id, name);
        }
        else if(SQL_NO_DATA==r) break;
        else
        {
            printf("%s\n", "fail to fetch data");
            break;
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
}

void main()
{
    ODBCConnectDB("localhost", "user", "123456");

    db_exec_stmt("insert into employee (name,age,birthday,create_timestamp) \
    values (\'joy\',20.0,\'2001-1-2\',now())");

    db_fetch();

    ODBCDisconnectDB();
}
