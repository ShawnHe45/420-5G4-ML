#include <iostream>

using namespace std;

.def("get_book", [](Library& lib, const string& isbn)
    optional<Book>{
        Book* book = lib.findBookByISBN(isbn);
        if(!book) return nullopt;
        return *book;
    },
    py::arg("isbn"))

.def("get_user", [](Library& lib, const string& userId)
            optional<User> {
                User* user = lib.findUserById(userId);
                if(!user) return nullopt;
                return *user
            },
            py::arg("userId"))