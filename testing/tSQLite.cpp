<<<<<<< HEAD
#include "Poco/Data/Session.h"
#include "Poco/Data/SQLite/Connector.h"
#include <vector>
#include <iostream>

=======
#include "Poco/Data/Data.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/Extraction.h"
#include "Poco/Data/Statement.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/Data.h"
#include <vector>
#include <iostream>

using namespace Poco::Data;

>>>>>>> 877f4143259cbeccc23bed2b9d44c00ddcadd83d
struct Person {
    std::string name;
    std::string address;
    int age;
};

int main() {
<<<<<<< HEAD
    using namespace Poco::Data;
=======
>>>>>>> 877f4143259cbeccc23bed2b9d44c00ddcadd83d
    // register SQLite connector
    Poco::Data::SQLite::Connector::registerConnector();

    // create a session
    Poco::Data::Session session("SQLite", "sample.db");

    // drop sample table, if it exists
<<<<<<< HEAD
    session << "DROP TABLE IF EXISTS Person", Poco::Data::now;

    // (re)create table
    session << "CREATE TABLE Person (Name VARCHAR(30), Address VARCHAR, Age "
               "INTEGER(3))",
        Poco::Data::now;

    // insert some rows
    Person person = {"Bart Simpson", "Springfield", 12};

    Poco::Data::Statement insert(session);
    insert << "INSERT INTO Person VALUES(?, ?, ?)", use(person.name),
        use(person.address), use(person.age);

    insert.execute();

    person.name = "Lisa Simpson";
    person.address = "Springfield";
    person.age = 10;

    insert.execute();

    // a simple query
    Poco::Data::Statement select(session);
    select << "SELECT Name, Address, Age FROM Person", into(person.name),
        into(person.address), into(person.age),
        range(0, 1); //  iterate over result set one row at a time

    while (!select.done()) {
        select.execute();
        std::cout << person.name << " " << person.address << " " << person.age
                  << std::endl;
    }
=======
    // session << "DROP TABLE IF EXISTS Person", Poco::Data::now;

    // // (re)create table
    // session << "CREATE TABLE Person (Name VARCHAR(30), Address VARCHAR, Age "
    //            "INTEGER(3))",
    //     Poco::Data::now;

    // // insert some rows
    // Person person = {"Bart Simpson", "Springfield", 12};

    // Statement insert(session);
    // insert << "INSERT INTO Person VALUES(?, ?, ?)", use(person.name),
    //     use(person.address), use(person.age);

    // insert.execute();

    // person.name = "Lisa Simpson";
    // person.address = "Springfield";
    // person.age = 10;

    // insert.execute();

    // // a simple query
    // Statement select(session);
    // select << "SELECT Name, Address, Age FROM Person", into(person.name),
    //     into(person.address), into(person.age),
    //     range(0, 1); //  iterate over result set one row at a time

    // while (!select.done()) {
    //     select.execute();
    //     std::cout << person.name << " " << person.address << " " << person.age
    //               << std::endl;
    // }
>>>>>>> 877f4143259cbeccc23bed2b9d44c00ddcadd83d

    return 0;
}
