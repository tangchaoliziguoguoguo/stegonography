#pragma once
#include <QString>
#include "Hardinter/CRecord.cpp"
#include "Hardinter/CReport.cpp"
using namespace std;

class CFormCheck {
public:
	CFormCheck(const QString & filepath);
	~CFormCheck();
	const class CRecord & ImgFormCheck(const QString& serialnum);
	const QString & geterror();
private:
	unsigned long long myfilelength;
	QString My_msg;
	CReport matchReport;
	QString filepath;
	int DataMatch(int length, long offset,const QString& remarks);
	int DataMatch(int length, long offset, unsigned char * capital, const QString& remarks);
	int DataMatch(const QString& remarks);
	int BreakBMPImg(const QString& filepath, unsigned char *IMGBUF, long offset);
	int BreakPNGImg(const QString& filepath, unsigned char *IMGBUF, long offset);
	int BreakJPGImg(const QString& filepath, unsigned char *IMGBUF, long offset);
	int ConfirmFormat(const QString& filepath, unsigned char* bImageFormat);
	int CheckEND(unsigned long pointer, unsigned long long END, const QString& remark);
	int errorcheck(int error);
};

template <class T>
T numrote(T a)//将长短整形值翻转，弥补memcpy从数组拷贝到整形值时发生的翻转
{
	int s = sizeof(a);
	T temp = 0;

	if (s == 2) {//短整形
		T temp1 = a >> 8 & 0x00ff;
		T temp2 = a & 0x00ff;
		temp = (temp2 << 8 & 0xff00) | (temp1 & 0xff00);
	}
	if (s == 4) {//长整型
		T temp1 = a >> 24 & 0x000000ff;
		T temp2 = a >> 16 & 0x000000ff;
		T temp3 = a >> 8 & 0x000000ff;
		T temp4 = a & 0x000000ff;
		temp = (temp4 << 24 & 0xff000000) |
			(temp3 << 16 & 0x00ff0000) |
			(temp2 << 8 & 0x0000ff00) |
			(temp1 & 0x000000ff);
	}
	a = temp;
	return a;
}
