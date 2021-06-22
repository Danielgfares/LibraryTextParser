// LibraryTextParser.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <string>
#include <iostream>
#include <fstream>
#include <List>
#include <vector>
#include <sstream>
#include <thread>
#include <mutex>
#include <algorithm>

using namespace std;

mutex mtx;

list<struct Book> books;
list<struct Book> ReadBooks(string input);
list<struct Book> FindBooks(string searchString);

typedef struct Book
{
	list<string> authors;
	string title;
	string publisher;
	int publicationYear = 0;
} Book;


void thRun(vector<string>& _queries, list<struct Book>& res, struct Book& book)
{
	bool authors = false;
	bool match = false;
	unsigned int i = 0;

	while (!match && i < _queries.size()) {

		list<string>::iterator it = book.authors.begin();
		while (!authors && it != book.authors.end()) {
			if ((*it).find(_queries[i]) != string::npos) {
				authors = true;
			}
			it++;
		}

		if (book.title.find(_queries[i]) != string::npos ||
			book.publisher.find(_queries[i]) != string::npos ||
			to_string(book.publicationYear).find(_queries[i]) != string::npos ||
			authors) {
			match = true;
		}
		i++;
	}

	if (match) {
		mtx.lock();
		res.push_back(book);
		mtx.unlock();
	}
}



int main()
{
	books = ReadBooks("Text.txt");
	
	for (list<struct Book>::iterator it = books.begin(); it != books.end(); it++)
	{
		cout << (*it).title << endl;
		cout << (*it).publisher << endl;
		cout << (*it).publicationYear << endl;
		cout << (*it).authors.front() << endl;
	}

	list<struct Book> bb;
	bb = FindBooks("*Peter*");

	return 0;
}

list<struct Book> ReadBooks(string input)
{
	string data;
	string header;
	string text;
	ifstream infile;
	list<struct Book> res;
	struct Book actual_book;
	bool reading_book = false;

	infile.open(input, ios::in);
	if (!infile) {
		cout << "No such file" << endl;
	}
	
	if (infile.is_open()) {
		while (infile.good()) {
			getline(infile, data);
			if (data.size() == 0) {
				if (reading_book) {
					res.push_back(actual_book);
					struct Book newBook;
					actual_book = newBook;
				}
				reading_book = false;
			}
			else {
				stringstream ss(data);
				getline(ss, header, ' ');
				if (reading_book) {
					if (header == "Book:") {
						res.push_back(actual_book);
						struct Book newBook;
						actual_book = newBook;
					}
					else if (header == "Author:") {
						getline(ss, text);
						actual_book.authors.push_back(text);
					}
					else if (header == "Title:") {
						getline(ss, text);
						actual_book.title = text;
					}
					else if (header == "Publisher:") {
						getline(ss, text);
						actual_book.publisher = text;
					}
					else if (header == "Published:") {
						int published = 0;
						ss >> published;
						actual_book.publicationYear = published;
					}
				}
				else {
					if (header == "Book:") {
						reading_book = true;
					}
				}
			}
		}
		// close the opened file.
		infile.close();
	}
	else {
		cout << "Could'nt open file" << endl;
	}
	return res;
}

list<struct Book> FindBooks(string searchString)
{
	vector<thread> threads;
	list<struct Book> res;
	stringstream ss(searchString);
	vector<string> queries;
	string s;
	while (getline(ss, s, '&')) {
		replace(s.begin(), s.end(), '*', ' ');
		s.erase(remove_if(s.begin(), s.end(), isspace), s.end());
		queries.push_back(s);
	}
	
	for (list<struct Book>::iterator it = books.begin(); it != books.end(); it++)
	{
		
		thread thread(&thRun, ref(queries), ref(res), ref(*it));
		threads.push_back(move(thread));
	}
	
	for (unsigned int i = 0; i < threads.size(); ++i)
	{
		threads.at(i).join();
	}

	return res;
}



