/*
 * File    : VerGen.cpp
 * Remark  : Version generator program, utility of this project.
 *
 */

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>

using namespace std;

const char * szDefaultFileName = "AppVersion.h";
const char * szDefaultDefName = "DKVER";

int main(int argc, char* argv[])
{
	char *	szFileName = argv[1];
	char *	szDefName = argv[2];

	if (szFileName == 0) {
		szFileName = const_cast<char *>(szDefaultFileName);
		szDefName = const_cast<char *>(szDefaultDefName);
	} else {
		if (szDefName == 0) szDefName = const_cast<char *>(szDefaultDefName);
	}

	ifstream	fin;
	bool		is_new_file = false;

	fin.open(szFileName);
	if (fin.is_open() == false) {
		if (errno != 2) {
			cout << "Error open file: " << szFileName << "! (err: " << errno << ")." << endl;
			return -1;
		}

		cout << "File " << szFileName << " does not exist, ";
		cout << "try to create a new one... ";

		FILE * f = 0;
		errno_t err = fopen_s(&f, szFileName, "w");
		if (!f) {
			cerr << "Fail! (err: " << err << ")" << endl;
			return -1;
		} else {
			cout << "Success." << endl;
			fclose(f);
			is_new_file = true;
		}
	}

	vector<int>	i_vers;

	if (is_new_file) {
		
		i_vers.push_back(1);
		i_vers.push_back(0);
		i_vers.push_back(1);
		i_vers.push_back(1);

		cout << "New version: ";
		cout << i_vers[0] << "." << i_vers[1] << "." << i_vers[2] << "." << i_vers[3] << endl;

	} else {

		string			ver;
		int i_len = ver.length();
		while (!fin.eof()) {
			char buf[512] = {0};
			fin.getline(buf, 512);
			istringstream istr(buf);
			string word;
			while (istr >> word) {
				if (word == szDefName) {
					istr >> word;
					ver = word;
					break;
				}
			}

			if (ver.length() > 0) break;
		}
		fin.close();

		if (ver.length() == 0) {
			cerr << "Bad file format!" << endl;
			return -1;
		}

		vector<string>	vers;
		vector<string>::iterator	vers_iter;
		vers.push_back("");
		vers.push_back("");
		vers.push_back("");
		vers.push_back("");
		int iver = 0;
		vers_iter = vers.begin();
		for (string::size_type i = 0; i < ver.length(); i++)
		{
			if (vers_iter == vers.end()) break;

			if (ver[i] != ',') {
				(*vers_iter) += ver[i];
			} else {
				vers_iter++;;
			}
		}
		
		vers_iter = vers.begin();
		while (vers_iter != vers.end()) {
			sscanf_s((*vers_iter).c_str(), "%d", &iver);
			i_vers.push_back(iver);
			vers_iter++;
		}

		cout << "Current version: ";
		cout << i_vers[0] << "." << i_vers[1] << "." << i_vers[2] << "." << i_vers[3] << endl;

		i_vers[3]++;

		cout << "Next version: ";
		cout << i_vers[0] << "." << i_vers[1] << "." << i_vers[2] << "." << i_vers[3] << endl;

	}

	ofstream	fout;
	fout.open(szFileName);
	if (fout.is_open() == false)
	{
		cerr << "Error open file: " << szFileName << endl;
		return -1;
	}

	fout.clear(ios_base::goodbit);

	fout << "/*" << endl;
	fout << " * This is generated version file." << endl;
	fout << " * File    : " << szFileName << "." << endl;
	fout << " * Desc.   : Version file for " << szDefName << "." << endl;
	fout << " *" << endl;
	fout << " */" << endl;
	fout << endl;
	fout << endl;

	fout << "#ifndef " << szDefName << "_H" << endl;
	fout << "#define " << szDefName << "_H" << endl;

	fout << endl;

	fout << "#define " << szDefName << "			";
	fout << i_vers[0] << ',';
	fout << i_vers[1] << ',';
	fout << i_vers[2] << ',';
	fout << i_vers[3] << endl;

	fout << "#define " << szDefName << "_STR		_T(\"";
	fout << i_vers[0] << '.';
	fout << i_vers[1] << '.';
	fout << i_vers[2] << '.';
	fout << i_vers[3] << "\")";
	fout << endl;

	fout << endl;

	fout << "#endif		// End of " << szDefName << "_H" << endl;

	fout.close();

	return 0;
}