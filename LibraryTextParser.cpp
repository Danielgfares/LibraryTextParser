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
// mutex to control access for the threads when searching for books thats satisfy a criteria
mutex mtx;

// definition of function and global variables 
list<struct Book> books;
list<struct Book> ReadBooks(string input);
list<struct Book> FindBooks(string searchString);

// the given data structure with a little changes (from class to struct) 
typedef struct Book
{
	list<string> authors;
	string title;
	string publisher;
	int publicationYear = 0;
} Book;

/**
* this function is the base function for the threads used to find books that satisfy the criteria
*/
void thRun(vector<string>& _queries, list<struct Book>& res, struct Book& book)
{
	bool authors = false;
	bool match = false;
	unsigned int i = 0;
	// for every word in the criteria search in all atributes of the book
	while (!match && i < _queries.size()) {
		// first compare all names of the authors
		list<string>::iterator it = book.authors.begin();
		while (!authors && it != book.authors.end()) {
			if ((*it).find(_queries[i]) != string::npos) {
				authors = true;
			}
			it++;
		}
		// then compare other atributes
		if (book.title.find(_queries[i]) != string::npos ||
			book.publisher.find(_queries[i]) != string::npos ||
			to_string(book.publicationYear).find(_queries[i]) != string::npos ||
			authors) {
			match = true;
		}
		i++;
	}
	// if found a match with any substring or any value. Then add the book to the list of found books
	if (match) {
		mtx.lock();
		res.push_back(book);
		mtx.unlock();
	}
}



int main()
{
	books = ReadBooks("Text.txt");
	
	list<struct Book> bb;
	bb = FindBooks("*20* & *Peter*");
	
	for (list<struct Book>::iterator it = bb.begin(); it != bb.end(); it++)
	{
		cout << "Book: " << endl;
		cout << "Title: " << (*it).title << endl;
		cout << "Publisher: " << (*it).publisher << endl;
		cout << "Publication Year: " << (*it).publicationYear << endl;
		cout << "Author: " << (*it).authors.front() << endl;
		cout << endl;
	}
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

	// open file with a input stream
	infile.open(input, ios::in);
	if (!infile) {
		cout << "No such file" << endl;
	}
	
	if (infile.is_open()) {
		while (infile.good()) {
			// read a line of data 
			getline(infile, data);
			// if empty line then finished with reading a book a now must start reading a new book reading
			if (data.size() == 0) {
				if (reading_book) {
					res.push_back(actual_book);
					struct Book newBook;
					actual_book = newBook;
				}
				reading_book = false;
			}
			else {
				// read first word in a line 
				stringstream ss(data);
				getline(ss, header, ' ');
				if (reading_book) {
					// found to what atribute this word correspond and assign the rest of the line to this atribute 
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
	// split the query string by the symbol & 
	while (getline(ss, s, '&')) {
		// eliminate the empty spaces
		replace(s.begin(), s.end(), '*', ' ');
		s.erase(remove_if(s.begin(), s.end(), isspace), s.end());
		queries.push_back(s);
	}
	// for every book in the program  
	for (list<struct Book>::iterator it = books.begin(); it != books.end(); it++)
	{
		// start a thread  
		thread thread(&thRun, ref(queries), ref(res), ref(*it));
		threads.push_back(move(thread));
	}
	
	for (unsigned int i = 0; i < threads.size(); ++i)
	{
		threads.at(i).join();
	}

	return res;
}



