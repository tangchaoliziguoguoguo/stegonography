#pragma once
#include <QtCore/QCoreApplication>
#include <iostream>
#include "formatextract\formatextract.cpp"
using namespace std;
int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QString mypath("C:\\Users\\Stephen\\Desktop\\111.jpg");
	CFormCheck AA = CFormCheck(mypath);
	AA.ImgFormCheck("0");
	AA.geterror();
	return a.exec();
}
