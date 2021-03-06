#pragma once
#include "formatextract.h"
CFormCheck::CFormCheck(const QString & path){
	myfilelength = 0;
	My_msg = "";
	filepath.append(path);
}
CFormCheck::~CFormCheck(){

}
const class CRecord & CFormCheck::ImgFormCheck( const QString & serialnum)
{	
	QFile IMG(filepath);//确认文件是否存在
	if (!IMG.open(QIODevice::ReadOnly)){
		My_msg.append("error:no such file\n");
		return matchReport.GetRecord();
	}
	int retmsg = 0;
	long offset = 0;
	if (serialnum == NULL)
		retmsg = matchReport.NewReportFile(filepath, 0);//确认建立report的结果
	else//恢复报告
		offset = (matchReport.OpenReportFile(serialnum)).getLastOffset();
	if (retmsg == FUNC_SUCCEED){
		unsigned char bImageFormat[4] = { 0, 0, 0, 0 };
		myfilelength = unsigned long long(IMG.size());//初始文件长度
		if (myfilelength <= 4 ){//文件太短或读取错误
			My_msg.append("error:file too short\n");
			IMG.close();
			return matchReport.GetRecord();
		}
		
		unsigned char * cImgBuff = new(std::nothrow) unsigned char[myfilelength];//图片缓冲
		if (cImgBuff == NULL) {//内存错误异常处理
			My_msg.append("error:memory run out!");
			return matchReport.GetRecord();
		}
		for (unsigned long long i = 0; i < myfilelength;i++)
			IMG.getChar((char *)&cImgBuff[i]);//读入图片
		for (int i = 0; i < 4; i++)
			bImageFormat[i] = cImgBuff[i];
		switch (ConfirmFormat(filepath, bImageFormat)){//选择对应的拆解方法
		case BMP_FORMAT:
			retmsg = BreakBMPImg(filepath, cImgBuff, offset);
			break;
		case PNG_FORMAT:
			retmsg = BreakPNGImg(filepath, cImgBuff, offset);
			break;
		case JPG_FORMAT:
			retmsg = BreakJPGImg(filepath, cImgBuff, offset);
			break;
		case NONE_FORMAT:
			if (My_msg != NULL)
				break;
			else{
				My_msg.append("error:wrong type\n");
				break;
			}
		default:
			if (My_msg != NULL)
				break;
			else{
				My_msg.append("error: wrong with ConfirmFormat\n");
				break;
			}

		}
		if (retmsg == FUNC_SUCCEED || retmsg == OBJ_ERR){//成功直接返回否则进入错误处理
			matchReport.Close();
			return matchReport.GetRecord();
		}
		else if (retmsg == ERROR_INPUT){
			My_msg.append("offset is over perhaps the file is ok!");
			matchReport.Close();
			return matchReport.GetRecord();
		}
	}
	errorcheck(retmsg);
	return matchReport.GetRecord();
}
int CFormCheck::DataMatch(int length,long  offset, const QString& remarks)
{
	return matchReport.AddToReport(offset, remarks, length);
}
int CFormCheck::DataMatch(int length, long offset, unsigned char * capital, const QString& remarks){
	int searchresult = 0;
	int retmsg = 0;
	searchresult = matchReport.SearchInReport(0, capital, length);
	if (searchresult == FUNC_SUCCEED){
		retmsg = matchReport.AddToReport(offset, "WARNING FOUND REMARKS.", length);
		return retmsg;
	}
	else retmsg = matchReport.AddToReport(offset, remarks, length);
	return retmsg;
}
int CFormCheck::DataMatch(const QString& remarks){
	return matchReport.AddToReport( remarks);
}
/*检查BMP文件格式包括内容：
	1.是否完整
	2.保留区是否正常
	3.位图信息指标是否正常
	4.颜色使用及调色板区是否正常
	5.是否具有冗余数据段
*/
int CFormCheck::BreakBMPImg(const QString & filepath, unsigned char *IMGBUF, long offset)
{
	unsigned long   nSet = 0;//位置指针
	unsigned long  lDataLength = 0;//数据区长度
	unsigned long  lDataOffset = 0;//数据区首地址偏移量
	unsigned long  lfilelength = 0;
	unsigned short  nTypeBmp = 0;//存储位数
	unsigned int lTemp = 0;//位图信息头
	unsigned int Biclinform[2] = {0,0};//已用颜色数,重要颜色数
	unsigned char SUSPECTBLOCK[4];//可疑区大小
	int rtmsg = 0;//调用函数返回值
	memset(SUSPECTBLOCK, 0x00, 4);
	memcpy(&lDataLength, &IMGBUF[34], 4);//读入长度（实际像素所占空间）
	numrote(lDataLength);//整形翻转（保证数据高低位正确）
	if (lDataLength == 0)//若为0读入文件大小
	{
		memcpy(&lDataLength, &IMGBUF[2], 4);//读取位图文件大小
		numrote(lDataLength);
	}
	memcpy(&lDataOffset, &IMGBUF[10], 4);//数据偏移量
	numrote(lDataOffset);
	lfilelength = lDataOffset + lDataLength;//文件指示长度
	memcpy(&nTypeBmp, &IMGBUF[28], 2);//位深，每像素几位
	rtmsg = CheckEND((lfilelength-1), myfilelength, "warning:picture is incomplete.");
	if (rtmsg != FUNC_SUCCEED) return rtmsg;
	nSet += 6;//跳过bfTYPE及bfSIZE
	rtmsg=CheckEND(nSet, myfilelength, "picture END before Reserved.");
	if (rtmsg != FUNC_SUCCEED) return rtmsg;
	memcpy(&SUSPECTBLOCK, &IMGBUF[nSet], 4);//读取保留区检查其是否藏有内容
	if (SUSPECTBLOCK[0] != 0x00 || SUSPECTBLOCK[1] != 0x00 || SUSPECTBLOCK[2] != 0x00 || SUSPECTBLOCK[3] != 0x00){
		if (offset < nSet){
			rtmsg = DataMatch(4, nSet, SUSPECTBLOCK, "illegal reserve block");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
		}
	}
	nSet += 4;
	nSet += 4;//跳过boffset（已记录）
	rtmsg = CheckEND(nSet, myfilelength, "picture END before bitmap information.");
	if (rtmsg != FUNC_SUCCEED) return rtmsg;
	memcpy(&lTemp, &IMGBUF[nSet], 4);//读取位图信息段长度
	numrote(lTemp);
	if (lTemp != 40)
		if (offset < nSet){
			rtmsg = DataMatch(4, nSet, "special version of bmp");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
		}
	nSet += 28;//跳至色彩数据
	rtmsg = CheckEND(nSet, myfilelength, "picture END before Biclrused.");
	if (rtmsg != FUNC_SUCCEED) return rtmsg;
	memcpy(&Biclinform[0], &IMGBUF[nSet], 4);//读取使用颜色数
	numrote(Biclinform[0]);
	nSet += 4;
	memcpy(&Biclinform[1], &IMGBUF[nSet], 4);//读取重要颜色数
	numrote(Biclinform[1]);
	if (Biclinform[0] <= Biclinform[1]){
		if (offset < nSet-4){
			rtmsg = DataMatch(8, nSet - 4, "color information has been changed");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
		}
	}
	if (nTypeBmp == 8 || nTypeBmp == 4)//处理调色板
	{
		bool alpha = 0;
		unsigned int nCountColour = 0;
		if (nTypeBmp == 4) nCountColour = 16;
		if (nTypeBmp == 8) nCountColour = 256;//抵达第1个保留位	
		for (int i = 0; i < nCountColour; i++)
			if (IMGBUF[54 + i * 4 + 3] != 0x00) alpha = 1;//检查透明通道值
		QString ctmsg = "color table";//调色板保存
		if ((nCountColour>Biclinform[0] && Biclinform[0] != 0) || nCountColour>Biclinform[1] && Biclinform[1] != 0)
			ctmsg.append(",waning:color table has redundance");//可能冗余调色板
		if (nCountColour < Biclinform[0] || nCountColour < Biclinform[1]){
			ctmsg.append(",waning:color table has been changed");//调色板不足
			if (offset < nSet - 4){
				rtmsg = DataMatch(8, nSet - 4, (unsigned char *)Biclinform, "suspected colornumber!");
				if (rtmsg != FUNC_SUCCEED) return rtmsg;
			}
		}
		if (alpha == 1)
			ctmsg.append(" color table with alpha");
		rtmsg = DataMatch(4 * max(Biclinform[0], nCountColour), lTemp + 14, ctmsg);
		if (rtmsg != FUNC_SUCCEED) return rtmsg;
	}
	if (myfilelength > lfilelength)//存在冗余数据段
	{
		int i = 0;
		i = myfilelength - lfilelength;
		if (offset < lfilelength){//记录打入标记
			rtmsg = DataMatch(i, lfilelength, "extradata");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
		}
		else return ERROR_INPUT;
	}
	return FUNC_SUCCEED;
}

int CFormCheck::BreakPNGImg(const QString & filepath, unsigned char *IMGBUF, long offset)
{
	return FUNC_SUCCEED;
}

int CFormCheck::BreakJPGImg(const QString & filepath, unsigned char *IMGBUF, long offset)
{
	unsigned long nSet=0;//位置指针
	bool Hierarchicalflag = false;//分层结构标记
	unsigned long perception = 0;//精度
	unsigned int rtmsg = 0;//返回指示
	unsigned int framenum = 0;//帧计数
	bool frameused = false;//帧使用标记
	unsigned long mcu = 0;//最小编码单元
	bool RSTavailable = false;//重开始标记
	bool RSTused = false;//RST已使用标记
	unsigned int scannum = 0;//扫描计数
	unsigned int Interval = 0;//间隔长度
	unsigned int  Pictureflag = 0;//图像数据标记
	unsigned short  nSegment_head = 0;//段标志2字节
	unsigned short bSegmentLength = 0;//段长度
	unsigned int segment_space = 0;//段间冗余
	unsigned long FrameType = 0;//段编码类型
	unsigned long unknowndatalength = 0;//未知数据块
	unsigned char * tabledress;//样本因子数组
	tabledress = new unsigned char[256];
	memset(tabledress,0x00, 256);//初始化样本因子数组
	unsigned short precision = 0;//数据精度
	unsigned short Nf = 0;//图块数量
	unsigned int hvtablebefore = 0;//前一个扩展因子表标记
	bool arithmeticused = 0; //使用提供算法标记
	while (nSet < myfilelength){//若还有字节或未到EOI段则继续
		if (IMGBUF[nSet] != 0xff){//段头数据有误
			if (unknowndatalength == 0){//若此为第一个数据添加标记出错
				rtmsg = DataMatch("incorrect segment.");
				if (rtmsg != FUNC_SUCCEED) return rtmsg;
			}
			if (nSet >= myfilelength){//若到达文件尾则存储
				if (offset < nSet){//若offset正常则储存
					rtmsg = DataMatch(unknowndatalength, nSet - unknowndatalength, " END with special chars.");
					return rtmsg;
				}
				else return ERROR_INPUT;//错误返回入参错误
			}
			unknowndatalength++;
			nSet++;//跳过此字节
			continue;
		}
		else if (unknowndatalength != 0){//若存在未知段落
			if (nSet + 1 < myfilelength){//下一个字节存在
				nSet++;
				if (IMGBUF[nSet] == 0x00 || IMGBUF[nSet] == 0xFF){//非段标记
					unknowndatalength += 2;
					nSet++;
					if (nSet >= myfilelength){//若长度溢出则储存奇异字符串
						if (offset < nSet - unknowndatalength){//记录正常
							rtmsg = DataMatch(unknowndatalength, nSet - unknowndatalength, " END with special chars.");
							return rtmsg;
						}
						else return ERROR_INPUT;//offset溢出
					}
					else continue;//未越界则存储该字符
				}
				else if (offset < nSet - unknowndatalength - 1){//若确实进入正常段储存特殊字段
					rtmsg = DataMatch(unknowndatalength, nSet - unknowndatalength - 1, " special blocks.");
					if (rtmsg != FUNC_SUCCEED) return rtmsg;
					nSet--;
					unknowndatalength = 0;
					continue;
				}
				else{//此可疑字段已处理
					nSet--;
					unknowndatalength = 0;
					continue;
				}
			}
			else{
				rtmsg = DataMatch(unknowndatalength, nSet - unknowndatalength, " special blocks.");
			}
		}
		memcpy(&nSegment_head, IMGBUF+nSet, 2);
		nSet += 2;//跳过标记
		memcpy((char *)&bSegmentLength + 1, IMGBUF + nSet, 1);
		memcpy(&bSegmentLength , IMGBUF + nSet + 1, 1);
		if (nSegment_head == 0xffff){
			nSet += 1;
			segment_space++;
			continue;
		}
		else if (segment_space != 0){
			QString tmpmsg;
			tmpmsg = QString("redundant 0xff between'%1'and'%2'").arg(nSet - 4).arg(nSet);
			rtmsg = DataMatch(tmpmsg);
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
		}
		/*段内容检测*/
		unsigned char mark = ((char *)&nSegment_head)[1];//获得标识符
		if (mark == 0xd8){//SOI段
			RSTavailable = false;
			if (nSet>=2){
				rtmsg = DataMatch("error SOI in the  image .");
				if (rtmsg != FUNC_SUCCEED) return rtmsg;
			}
			continue;
		}
		else if (mark >= 0xc0 && mark <= 0xcf && mark != 0xc4 && mark != 0xc8 && mark != 0xcc){//SOFn段
			rtmsg = CheckEND(nSet + bSegmentLength, myfilelength, "picture end at a SOF segment.");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
			framenum++;
			if (Hierarchicalflag){
				nSet += 2;
				memcpy(((char *)&precision) + 1, IMGBUF + nSet, 1);//输入精度
				nSet += 4;
				char temp=0x00; 
				short nowNf=0;//次帧的组成部分
				int tableerror = 0;//与扩展因子表有出入
				if (framenum == 1 || frameused==false) tableerror = 1;//前一个表没有用或为DHP表
				memcpy(((char *)&nowNf) + 1, IMGBUF + nSet, 1);//输入数量
				if (nowNf != Nf&&tableerror){
					rtmsg = DataMatch(Nf, hvtablebefore, (unsigned char*)(IMGBUF + hvtablebefore), "useless haffman table.");
					if (rtmsg != FUNC_SUCCEED) return rtmsg;
					tableerror = 0;
				}
				nSet += 2;//到达扩展因子表
				for (int i = 0; i < nowNf; i++){
					memcpy(&temp, IMGBUF + nSet, 1);//保存因子
					if (temp != *(tabledress + i) && *(tabledress + i)&&tableerror&&offset<nSet){//如果存在和未用表的差异值，存储前表的差异值
						rtmsg = DataMatch(1, hvtablebefore+3*i-2, "changed Huffmantable item in a SOF segment.");
						tableerror++;
						if (rtmsg != FUNC_SUCCEED) return rtmsg;
					}
					*(tabledress + i) = temp;//更新表
					nSet += 3;//到达下个值
				}
				if (tableerror - 1){
					rtmsg = DataMatch(QString("There is '%i' changed in huffmantable.").arg(tableerror - 1));
					if (rtmsg != FUNC_SUCCEED) return rtmsg;
				}
				Nf = nowNf;
				hvtablebefore = nSet - 3 * Nf;//更新记录的扩展因子表
				continue;
			}
			else{
				nSet += 2;
				memcpy(&precision, IMGBUF + nSet, 1);//输入精度
				nSet += 5;
				memcpy(&Nf, IMGBUF + nSet, 1);//输入数量
				for (int i = 0; i < Nf; i++){
					nSet += 2;
					memcpy(tabledress + i, IMGBUF + nSet, 1);
					nSet++;
				}
				continue;
			}
		}
		else if (mark >= 0xE0 && mark <= 0xEF){//ARPn段
			nSet += 2;
			rtmsg = CheckEND(nSet + bSegmentLength-2, myfilelength, "picture END at ARPsegment.");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
			if (bSegmentLength>2&&offset < nSet){//有值
				rtmsg = DataMatch(bSegmentLength - 2, nSet, (unsigned char *)IMGBUF + nSet, "application log.");
				if (rtmsg != FUNC_SUCCEED) return rtmsg;
			}
			nSet += bSegmentLength-2;
			continue;
		}
		else if (mark == 0xFE){//COM段
			nSet += 2;
			rtmsg = CheckEND(nSet + bSegmentLength-2, myfilelength, "picture END at comment segment.");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
			if (bSegmentLength>2&&offset < nSet){//有值
				rtmsg = DataMatch(bSegmentLength-2, nSet,(unsigned char *)IMGBUF+nSet, "comment log.");
				if (rtmsg != FUNC_SUCCEED) return rtmsg;
			}
			nSet += bSegmentLength;
			continue;
		}
		else if (mark == 0xDB){//DQT段 定义量化表
			rtmsg = CheckEND(nSet + bSegmentLength , myfilelength, "picture end at a DQT segment.");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
			nSet += bSegmentLength;
			continue;
		}
		else if (mark == 0xC4){//DHT段 定义哈夫曼表
			rtmsg = CheckEND(nSet + bSegmentLength , myfilelength, "picture end at a DHT segment.");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
			nSet += bSegmentLength;
			continue;
		}
		else if (mark == 0xDA){//sos段 扫描开始段
			rtmsg = CheckEND(nSet + bSegmentLength, myfilelength, "picture end at a sos header.");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
			scannum++;
			nSet += 2;
			unsigned short NS = 0;
			memcpy(&NS, IMGBUF + nSet, 1);//存储NS
			nSet++;//到达表达区
			if (RSTavailable&NS>1){
				unsigned short Ctemp = 0;//临时变量记录块
				unsigned char HV = 0;//HV因子
				mcu = 0;//每个scan内mcu重置
				for (int i = 0; i < NS; i++){
					memcpy(&Ctemp, IMGBUF + nSet, 1);//存储Ctemp
					HV = *((unsigned char*)tabledress + Ctemp);
					mcu += (HV & 0x0f)*(HV & 0xf0);
					nSet += 2;
				}
				mcu = mcu * 64 * (precision / 8);//计算mcu
				//nSet += 3+mcu*Interval;//跳过数据压缩区
				nSet += bSegmentLength - 2;
				for (; ((IMGBUF[nSet] != 0xff) || ((IMGBUF[nSet] == 0xff )&& ((IMGBUF[nSet + 1] == 0x00 )|| (IMGBUF[nSet + 1] == 0xff)))) && (nSet < myfilelength); nSet++);

			}
			else{//跳过数据压缩区
				nSet += bSegmentLength - 2;
				for (; ((IMGBUF[nSet] != 0xff) || ((IMGBUF[nSet] == 0xff) && ((IMGBUF[nSet + 1] == 0x00) || (IMGBUF[nSet + 1] == 0xff)))) && (nSet < myfilelength); nSet++);
			}
			continue;
		}
		else if (mark == 0xc8||mark >= 0xF0 && mark <= 0xFD){//JPGn保留段
			nSet += 2;
			rtmsg = CheckEND(nSet + bSegmentLength - 2, myfilelength, "picture END at ARPsegment.");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
			if (bSegmentLength>2&&offset < nSet + 2){//有值
				rtmsg = DataMatch(bSegmentLength - 2, nSet, (unsigned char *)IMGBUF + nSet, "JPG extension log");
				if (rtmsg != FUNC_SUCCEED) return rtmsg;
			}
			nSet += bSegmentLength;
			continue;
		}
		else if (mark == 0xcc){//DAC段 定义算法段
			rtmsg = CheckEND(nSet + bSegmentLength , myfilelength, "picture end at a dac segment.");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
			nSet += bSegmentLength;
			continue;
		}
		else if (mark == 0xDC){//DNL段
			rtmsg = CheckEND(nSet + bSegmentLength , myfilelength, "picture end at a dnl segment.");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
			nSet += bSegmentLength;
			continue;
		}
		else if (mark == 0xDD){//DRI段
			rtmsg = CheckEND(nSet + bSegmentLength , myfilelength, "picture end at a DRI segment.");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
			nSet += 2;
			if (Interval != 0 && RSTused == false){//若存在未用过的DRI
				rtmsg = DataMatch(QString("the DRI before is useless value:'%1'").arg(Interval));
				if (rtmsg != FUNC_SUCCEED) return rtmsg;
			}
			memcpy((char *)&Interval+1, IMGBUF + nSet ,1);
			memcpy(&Interval, IMGBUF + nSet + 1, 1);
			if (Interval == 0) RSTavailable = false;
			else RSTavailable = true;
			RSTused = false;
			nSet += 2;
			continue;
		}
		else if (mark == 0xDE){//DHP段
			rtmsg = CheckEND(nSet + bSegmentLength , myfilelength, "picture end at a DHP segment.");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
			Hierarchicalflag = true;
			nSet += 2;
			memcpy(((char *)&precision) + 1, IMGBUF + nSet, 1);//输入精度
			nSet += 4;
			memcpy(((char *)&Nf) + 1, IMGBUF + nSet, 1);//输入数量
			for (int i = 0; i < Nf; i++){
				nSet += 2;
				memcpy(tabledress+i, IMGBUF + nSet, 1);
				nSet++;
				if (IMGBUF[nSet] != 0x00){
					if (offset > nSet){
						nSet++;
						continue;
					}
					rtmsg = DataMatch(1, nSet , "illgal Tqi in a DHP segment.");
					if (rtmsg != FUNC_SUCCEED) return rtmsg;
				}
			}
			continue;
		}
		else if (mark == 0xDF){//EXP段
			rtmsg = CheckEND(nSet + bSegmentLength , myfilelength, "picture end at a EXP segment.");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
			if (!Hierarchicalflag&&offset < nSet + 2){
					rtmsg = DataMatch(1, nSet + 2, "illgal a EXP segment.");
					if (rtmsg != FUNC_SUCCEED) return rtmsg;
			}
			nSet += bSegmentLength;
			continue;
		}
		else if (mark == 0x01){//TEM段
			rtmsg = CheckEND(nSet + bSegmentLength , myfilelength, "picture end at a res segment.");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
			if (!arithmeticused){
				rtmsg = DataMatch(QString("unknown tmp segment in '%i'.").arg(nSet - 2));
				if (rtmsg != FUNC_SUCCEED) return rtmsg;
				continue;
			}
			else {
				if (offset > nSet) continue;
				else{
					rtmsg = DataMatch(bSegmentLength,nSet-2,"tmp segment.");
					if (rtmsg != FUNC_SUCCEED) return rtmsg;
				}
			}
		}
		else if (mark >= 0xD0 && mark <= 0xD7){//RSTm段
			rtmsg = CheckEND(nSet, myfilelength, "picture end at a RST segment.");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
			if (!RSTavailable){
				rtmsg = DataMatch(QString("WRONG RST IN '%i'").arg(nSet-2));
				if (rtmsg != FUNC_SUCCEED) return rtmsg;
				nSet += 2;
				continue;
			}
			RSTused = true;
			nSet +=2;
			for (; ((IMGBUF[nSet] != 0xff) || ((IMGBUF[nSet] == 0xff) && ((IMGBUF[nSet + 1] == 0x00) || (IMGBUF[nSet + 1] == 0xff)))) && (nSet < myfilelength); nSet++);
		}
		else if (mark >= 0x02 && mark <= 0xBF){//RES段
			rtmsg = CheckEND(nSet + bSegmentLength - 2, myfilelength, "picture end at a res.");
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
			rtmsg = DataMatch(QString("unknown res segment in '%i'").arg(nSet - 2));
			if (rtmsg != FUNC_SUCCEED) return rtmsg;
			continue;
		}
		else if (mark == 0xD9){//EOI段
			break;
		}
		}
		if (nSet < myfilelength){
			int i = myfilelength - nSet;
			if (offset < nSet){//记录打入标记
				rtmsg = DataMatch(i, nSet, "extradata");
				if (rtmsg != FUNC_SUCCEED) return rtmsg;
			}
			else return ERROR_INPUT;
		}
		return FUNC_SUCCEED;
	}
int CFormCheck::ConfirmFormat(const QString& filepath, unsigned char *bImageFormat)
{


	if (bImageFormat[0] == 0x42 && bImageFormat[1] == 0x4d)
		return BMP_FORMAT;			//BMP格式
	if (bImageFormat[0] == 0xff && bImageFormat[1] == 0xd8)
		return JPG_FORMAT;			//JPEG格式
	if ((bImageFormat[0] == 0x89 && bImageFormat[1] == 0x50 && bImageFormat[2] == 0x4e && bImageFormat[3] == 0x47))
		return PNG_FORMAT;			//PNG格式
	return NONE_FORMAT;
}

const QString & CFormCheck::geterror()
{
	return My_msg;
}
int CFormCheck::errorcheck(int error){
	if (error == HD_RUN_OUT){//硬盘空间不足
		My_msg.append("error:Harware run out!");
		return FUNC_SUCCEED;
	}
	else if (error == LIST_OVER_BOUND){//数组越界
		My_msg.append("error: list is over bound.\n");
		return FUNC_SUCCEED;
	}
	else if (error == MEM_RUN_OUT){//内存不足
		My_msg.append("error:memory run out!");
		return FUNC_SUCCEED;
	}
	//未知错误
	else My_msg.append("error:unknown error");
	return -1;
}
int CFormCheck::CheckEND(unsigned long pointer,unsigned long long END, const QString& remark){//检查文件是否到达尾部
	if (pointer >= END){
		int rtmsg=DataMatch(remark);
		if (rtmsg==FUNC_SUCCEED)
			return  OBJ_ERR;
		else return rtmsg;
	}
	return FUNC_SUCCEED;
}